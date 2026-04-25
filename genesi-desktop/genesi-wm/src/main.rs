use tracing::{info, Level};
use tracing_subscriber::FmtSubscriber;
use wayland_server::{Display, Client};
use calloop::{EventLoop, Interest, Mode, PostAction, generic::Generic};

use smithay::{
    delegate_compositor, delegate_shm, delegate_xdg_shell, delegate_seat, delegate_data_device, delegate_output,
    backend::{
        input::{InputEvent, KeyboardKeyEvent},
        renderer::{
            Color32F, Frame, Renderer,
            element::{
                Kind,
                solid::SolidColorRenderElement,
                surface::{WaylandSurfaceRenderElement, render_elements_from_surface_tree},
            },
            gles::GlesRenderer,
            utils::{draw_render_elements, on_commit_buffer_handler},
        },
        winit::{self as smithay_winit, WinitEvent},
    },
    wayland::{
        buffer::BufferHandler,
        compositor::{CompositorHandler, CompositorState, CompositorClientState, SurfaceAttributes, TraversalAction, with_surface_tree_downward},
        shm::{ShmHandler, ShmState},
        shell::xdg::{XdgShellHandler, XdgShellState, ToplevelSurface, PopupSurface, PositionerState},
        shell::xdg::decoration::{XdgDecorationHandler, XdgDecorationState},
        selection::{
            SelectionHandler,
            data_device::{DataDeviceHandler, DataDeviceState, WaylandDndGrabHandler},
        },
        output::{OutputHandler, OutputManagerState},
    },
    output::{Output, PhysicalProperties, Subpixel, Mode as OutputMode},
    input::{Seat, SeatHandler, SeatState, keyboard::FilterResult},
    reexports::wayland_server::protocol::{wl_surface::WlSurface, wl_buffer::WlBuffer, wl_seat::WlSeat},
    utils::{Rectangle, Serial, Transform, Point, Logical},
};

use smithay::backend::renderer::{ImportAll, ImportMem};
smithay::backend::renderer::element::render_elements! {
    pub CustomRenderElements<R> where R: ImportAll + ImportMem;
    Surface=WaylandSurfaceRenderElement<R>,
    SolidColor=SolidColorRenderElement,
}

use wayland_protocols::xdg::shell::server::xdg_toplevel;
use ::winit::platform::pump_events::PumpStatus;

// O estado do cliente conectado
#[derive(Default)]
pub struct ClientState {
    pub compositor_state: CompositorClientState,
}
impl wayland_server::backend::ClientData for ClientState {
    fn initialized(&self, _client_id: wayland_server::backend::ClientId) {}
    fn disconnected(&self, _client_id: wayland_server::backend::ClientId, _reason: wayland_server::backend::DisconnectReason) {}
}

// Estrutura principal do nosso Sistema Operacional
pub struct GenesiState {
    pub compositor_state: CompositorState,
    pub shm_state: ShmState,
    pub xdg_shell_state: XdgShellState,
    pub xdg_decoration_state: XdgDecorationState,
    pub seat_state: SeatState<Self>,
    pub data_device_state: DataDeviceState,
    pub output_manager_state: OutputManagerState,
    pub seat: Seat<Self>,
    pub output: Output,
    pub pointer_location: Point<f64, Logical>,
    pub window_positions: std::collections::HashMap<WlSurface, Point<i32, Logical>>,
    pub window_order: Vec<WlSurface>,
    pub moving_window: Option<(WlSurface, Point<i32, Logical>)>,
    // Estado para resize interativo via protocolo xdg_toplevel.resize()
    pub resizing_window: Option<ResizingState>,
    // Janelas minimizadas (não renderizadas, mas mantidas no estado)
    pub minimized_windows: Vec<WlSurface>,
    // Posição e tamanho salvos antes de maximizar
    pub unmaximized_state: std::collections::HashMap<WlSurface, (Point<i32, Logical>, smithay::utils::Size<i32, Logical>)>,
    // Borda sendo "hovered" no momento
    pub hovered_edge: Option<(WlSurface, xdg_toplevel::ResizeEdge)>,
    // Cursor customizado fornecido por um cliente Wayland
    pub cursor_status: smithay::input::pointer::CursorImageStatus,
}

#[derive(Clone)]
pub struct ResizingState {
    pub surface: WlSurface,
    pub edges: xdg_toplevel::ResizeEdge,
    pub start_pointer: Point<f64, Logical>,
    pub start_rect: Rectangle<i32, Logical>, // posição + tamanho iniciais
}

// =================================================================================
// IMPLEMENTAÇÃO DOS PROTOCOLOS DO WAYLAND VIA SMITHAY
// =================================================================================

// 1. Compositor
impl CompositorHandler for GenesiState {
    fn compositor_state(&mut self) -> &mut CompositorState {
        &mut self.compositor_state
    }
    fn client_compositor_state<'c>(&self, client: &'c Client) -> &'c CompositorClientState {
        &client.get_data::<ClientState>().unwrap().compositor_state
    }
    fn commit(&mut self, surface: &WlSurface) {
        on_commit_buffer_handler::<Self>(surface);

        // Bugfix WebKitGTK (Tauri):
        // O WebkitGTK entra em deadlock e a tela do Tauri nunca é desenhada (fica preta).
        if let Some(state) = self.xdg_shell_state.toplevel_surfaces().iter().find(|s| s.wl_surface() == surface) {
            // Verifica se precisamos disparar um configure forçado
            let is_activated = state.with_pending_state(|s| s.states.contains(xdg_toplevel::State::Activated));
            if !is_activated {
                state.send_configure();
            }
        }
    }
}

// 2. SHM (Shared Memory)
impl ShmHandler for GenesiState {
    fn shm_state(&self) -> &ShmState {
        &self.shm_state
    }
}

// 3. Buffer
impl BufferHandler for GenesiState {
    fn buffer_destroyed(&mut self, _buffer: &WlBuffer) {}
}

// 4. XDG Shell
impl XdgShellHandler for GenesiState {
    fn xdg_shell_state(&mut self) -> &mut XdgShellState {
        &mut self.xdg_shell_state
    }

    fn new_toplevel(&mut self, surface: ToplevelSurface) {
        info!("🪟 Nova janela (Toplevel) solicitada pelo app!");
        surface.with_pending_state(|state| {
            state.states.set(xdg_toplevel::State::Activated);
            state.size = Some((1024, 768).into()); // Tamanho inicial garantido
        });
        
        // Cadastra a janela no sistema de posicionamento (Janelas novas abrem no meio)
        let wl_surface = surface.wl_surface().clone();
        if !self.window_positions.contains_key(&wl_surface) {
            self.window_positions.insert(wl_surface.clone(), (200, 200).into());
            self.window_order.push(wl_surface);
        }

        self.output.enter(surface.wl_surface());
        surface.send_configure();
    }

    fn maximize_request(&mut self, surface: ToplevelSurface) {
        // Ao maximizar, desminimiza a janela se estiver minimizada
        let wl = surface.wl_surface().clone();
        self.minimized_windows.retain(|s| s != &wl);
        surface.with_pending_state(|state| {
            state.states.set(xdg_toplevel::State::Maximized);
        });
        surface.send_configure();
    }

    fn unmaximize_request(&mut self, surface: ToplevelSurface) {
        surface.with_pending_state(|state| {
            state.states.unset(xdg_toplevel::State::Maximized);
        });
        surface.send_configure();
    }

    fn fullscreen_request(&mut self, surface: ToplevelSurface, _output: Option<smithay::reexports::wayland_server::protocol::wl_output::WlOutput>) {
        surface.with_pending_state(|state| {
            state.states.set(xdg_toplevel::State::Fullscreen);
        });
        surface.send_configure();
    }

    fn unfullscreen_request(&mut self, surface: ToplevelSurface) {
        surface.with_pending_state(|state| {
            state.states.unset(xdg_toplevel::State::Fullscreen);
        });
        surface.send_configure();
    }

    fn new_popup(&mut self, surface: PopupSurface, positioner: PositionerState) {
        info!("🔽 Novo menu/popup solicitado!");
        surface.with_pending_state(|state| {
            state.geometry = positioner.get_geometry();
        });
        self.output.enter(surface.wl_surface());
        let _ = surface.send_configure();
    }
    fn reposition_request(&mut self, surface: PopupSurface, _positioner: PositionerState, token: u32) {
        surface.send_repositioned(token);
    }
    fn grab(&mut self, _surface: PopupSurface, _seat: WlSeat, _serial: Serial) {}

    // === Handler para appWindow.startDragging() do Tauri ===
    fn move_request(&mut self, surface: ToplevelSurface, _seat: WlSeat, _serial: Serial) {
        let wl_surface = surface.wl_surface().clone();
        // Desminimiza ao arrastar
        self.minimized_windows.retain(|s| s != &wl_surface);
        let pos = self.window_positions.get(&wl_surface).cloned().unwrap_or((0, 0).into());
        let offset_x = self.pointer_location.x as i32 - pos.x;
        let offset_y = self.pointer_location.y as i32 - pos.y;
        self.moving_window = Some((wl_surface, (offset_x, offset_y).into()));
        info!("⭐ Move request recebido - iniciando arraste da janela");
    }

    // === Handler para appWindow.startResizeDragging() do Tauri ===
    fn resize_request(&mut self, surface: ToplevelSurface, _seat: WlSeat, _serial: Serial, edges: xdg_toplevel::ResizeEdge) {
        use smithay::wayland::compositor::with_states;
        use smithay::wayland::shell::xdg::SurfaceCachedState;

        let wl_surface = surface.wl_surface().clone();
        let pos = self.window_positions.get(&wl_surface).cloned().unwrap_or((0, 0).into());
        
        let geometry = with_states(&wl_surface, |states| {
            states.cached_state.get::<SurfaceCachedState>()
                .current()
                .geometry
                .unwrap_or(Rectangle::new((0, 0).into(), (800, 600).into()))
        });

        self.resizing_window = Some(ResizingState {
            surface: wl_surface,
            edges,
            start_pointer: self.pointer_location,
            start_rect: Rectangle::new(pos, geometry.size),
        });
        info!("↔️ Resize request recebido - edges: {:?}", edges);
    }

    // === Handler para appWindow.minimize() do Tauri ===
    fn minimize_request(&mut self, surface: ToplevelSurface) {
        let wl_surface = surface.wl_surface().clone();
        if !self.minimized_windows.contains(&wl_surface) {
            self.minimized_windows.push(wl_surface);
        }
        info!("⬇️ Minimize request recebido");
    }

    // === Limpeza quando uma janela é destruída (appWindow.close()) ===
    fn toplevel_destroyed(&mut self, surface: ToplevelSurface) {
        let wl_surface = surface.wl_surface().clone();
        self.window_positions.remove(&wl_surface);
        self.window_order.retain(|s| s != &wl_surface);
        self.minimized_windows.retain(|s| s != &wl_surface);
        if let Some((ref s, _)) = self.moving_window {
            if s == &wl_surface { self.moving_window = None; }
        }
        if let Some(ref rs) = self.resizing_window {
            if rs.surface == wl_surface { self.resizing_window = None; }
        }
        info!("❌ Janela destruída e removida do window manager");
    }
}

// 4.1 XDG Decoration
impl XdgDecorationHandler for GenesiState {
    fn new_decoration(&mut self, toplevel: ToplevelSurface) {
        use smithay::reexports::wayland_protocols::xdg::decoration::zv1::server::zxdg_toplevel_decoration_v1::Mode;
        toplevel.with_pending_state(|state| {
            state.decoration_mode = Some(Mode::ServerSide);
        });
        toplevel.send_configure();
    }

    fn request_mode(
        &mut self,
        toplevel: ToplevelSurface,
        _mode: smithay::reexports::wayland_protocols::xdg::decoration::zv1::server::zxdg_toplevel_decoration_v1::Mode,
    ) {
        use smithay::reexports::wayland_protocols::xdg::decoration::zv1::server::zxdg_toplevel_decoration_v1::Mode;
        // O Genesi OS agora EXIGE Server-Side Decoration (SSD) para padronizar bordas
        toplevel.with_pending_state(|state| {
            state.decoration_mode = Some(Mode::ServerSide);
        });
        toplevel.send_configure();
    }

    fn unset_mode(&mut self, toplevel: ToplevelSurface) {
        use smithay::reexports::wayland_protocols::xdg::decoration::zv1::server::zxdg_toplevel_decoration_v1::Mode;
        toplevel.with_pending_state(|state| {
            state.decoration_mode = Some(Mode::ServerSide);
        });
        toplevel.send_configure();
    }
}

// 5. Seat e Data Device (Teclado, Mouse e Copiar/Colar)
impl SeatHandler for GenesiState {
    type KeyboardFocus = WlSurface;
    type PointerFocus = WlSurface;
    type TouchFocus = WlSurface;

    fn seat_state(&mut self) -> &mut SeatState<Self> {
        &mut self.seat_state
    }
    fn focus_changed(&mut self, _seat: &Seat<Self>, focused: Option<&WlSurface>) {
        // Quando uma janela recebe foco (ex: unminimize via Tauri), desminimiza ela
        if let Some(surface) = focused {
            self.minimized_windows.retain(|s| s != surface);
        }
    }
    fn cursor_image(&mut self, _seat: &Seat<Self>, image: smithay::input::pointer::CursorImageStatus) {
        self.cursor_status = image;
    }
}

impl SelectionHandler for GenesiState {
    type SelectionUserData = ();
}
impl DataDeviceHandler for GenesiState {
    fn data_device_state(&mut self) -> &mut DataDeviceState {
        &mut self.data_device_state
    }
}
impl WaylandDndGrabHandler for GenesiState {}

// 6. Output (Monitor)
impl OutputHandler for GenesiState {}

// Macros
delegate_compositor!(GenesiState);
delegate_shm!(GenesiState);
delegate_xdg_shell!(GenesiState);
smithay::delegate_xdg_decoration!(GenesiState);
delegate_seat!(GenesiState);
delegate_data_device!(GenesiState);
delegate_output!(GenesiState);

pub fn send_frames_surface_tree(surface: &WlSurface, time: u32) {
    with_surface_tree_downward(
        surface,
        (),
        |_, _, &()| TraversalAction::DoChildren(()),
        |_surf, states, &()| {
            for callback in states.cached_state.get::<SurfaceAttributes>().current().frame_callbacks.drain(..) {
                callback.done(time);
            }
        },
        |_, _, &()| true,
    );
}

/// Detecta se o app é um navegador que já tem controles de janela integrados na tab strip.
/// Esses apps não precisam de SSD porque a tab strip já funciona como barra de título.
fn is_self_decorating_browser(app_id: &str) -> bool {
    let id = app_id.to_lowercase();
    id.contains("chromium") ||
    id.contains("chrome") ||
    id.contains("google-chrome") ||
    id.contains("brave") ||
    id.contains("microsoft-edge") ||
    id.contains("vivaldi") ||
    id.contains("opera")
    // Firefox NÃO está aqui: ele usa libdecor CSD e precisa de nocsd + nosso SSD
}

/// Compila nocsd.c como biblioteca LD_PRELOAD e salva o path em /tmp.
/// O caminho é depois lido pelo Tauri para injetar o LD_PRELOAD ao lançar apps externos.
fn compile_nocsd_lib() -> Option<String> {
    const SO_PATH:  &str = "/tmp/genesi_nocsd.so";
    const SRC_PATH: &str = "/tmp/genesi_nocsd.c";

    // O código-fonte do nocsd.c é embutido no binário em tempo de compilação
    let src = include_str!("../nocsd.c");

    // Escreve o fonte e compila
    std::fs::write(SRC_PATH, src).ok()?;
    let status = std::process::Command::new("cc")
        .args(&["-shared", "-fPIC", "-ldl", "-o", SO_PATH, SRC_PATH])
        .status()
        .ok()?;

    if status.success() {
        Some(SO_PATH.to_string())
    } else {
        None
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Força a renderização via processador (Software) para evitar travamentos em Máquinas Virtuais
    std::env::set_var("LIBGL_ALWAYS_SOFTWARE", "1");
    // Forçamos o winit a usar Wayland, que é o ambiente principal do WSLg
    std::env::set_var("WINIT_UNIX_BACKEND", "wayland");
    
    // Variáveis Globais do Genesi OS (Forçando SSD / Amordaçando CSD)
    std::env::set_var("MOZ_ENABLE_WAYLAND", "1");
    std::env::set_var("MOZ_GTK_TITLEBAR_DECORATION", "system"); // Força o Firefox a não usar a própria barra
    std::env::set_var("GTK_CSD", "0"); // Desativa CSD em apps GTK3/GTK4
    std::env::set_var("QT_WAYLAND_DISABLE_WINDOWDECORATION", "1"); // Desativa CSD no Qt
    std::env::set_var("LIBDECOR_PLUGIN_DIR", "/dev/null"); // Força libdecor a não carregar plugins CSD
    
    // Precisamos de acesso ao display do host para o winit, não apagar o WAYLAND_DISPLAY original
    // antes de instanciar a janela.
    let host_wayland_display = std::env::var("WAYLAND_DISPLAY").unwrap_or_else(|_| "wayland-0".to_string());

    let subscriber = FmtSubscriber::builder()
        .with_max_level(Level::INFO)
        .finish();
    tracing::subscriber::set_global_default(subscriber)
        .expect("Falha ao inicializar o sistema de log do Genesi WM");

    // Compila e instala o interceptor nocsd.so para suprimir CSD em apps teimosos
    match compile_nocsd_lib() {
        Some(ref path) => {
            // Salva o path para que o Tauri possa usar ao lançar browsers/apps
            std::fs::write("/tmp/genesi-nocsd-path.txt", path)
                .unwrap_or_else(|e| tracing::warn!("Falha ao salvar nocsd path: {}", e));
            info!("🛡️  nocsd.so compilado → LD_PRELOAD pronto em {}", path);
        }
        None => {
            tracing::warn!("⚠️  Falha ao compilar nocsd.so — apps CSD teimosos não serão controlados");
        }
    }

    info!("===========================================");
    info!("🚀 Iniciando o Genesi OS Window Manager...");
    info!("🛡️ Motor Wayland (Smithay): Carregando módulos Base");
    info!("===========================================");

    // Ouroboros Deadlock Fix: 
    // Precisamos inicializar a janela DO NOSSO SO antes de abrir as portas para os apps conectarem.
    // Se a gente abrir a porta do Wayland primeiro, o Winit (Mesa EGL) vai achar a porta aberta, 
    // vai tentar se conectar no próprio Genesi OS, e os dois vão ficar congelados esperando um pelo outro!
    info!("🖥️  Iniciando a Janela Hospedeira (Backend Winit)...");
    let (mut backend, mut winit) = smithay_winit::init::<GlesRenderer>()?;
    backend.window().set_title("Genesi OS - Monitor Virtual");
    let _ = backend.window().request_inner_size(::winit::dpi::LogicalSize::new(1280.0, 720.0));
    
    let start_time = std::time::Instant::now();

    let display: Display<GenesiState> = Display::new()?;
    let mut display_handle = display.handle();

    let mut event_loop: EventLoop<GenesiState> = EventLoop::try_new()?;
    let loop_handle = event_loop.handle();

    let compositor_state = CompositorState::new::<GenesiState>(&display_handle);
    let shm_state = ShmState::new::<GenesiState>(&display_handle, vec![
        smithay::reexports::wayland_server::protocol::wl_shm::Format::Xrgb8888,
        smithay::reexports::wayland_server::protocol::wl_shm::Format::Argb8888,
    ]);
    let xdg_shell_state = XdgShellState::new::<GenesiState>(&display_handle);
    let xdg_decoration_state = XdgDecorationState::new::<GenesiState>(&display_handle);
    let output_manager_state = OutputManagerState::new_with_xdg_output::<GenesiState>(&display_handle);
    let mut seat_state = SeatState::new();
    let mut seat = seat_state.new_wl_seat(&display_handle, "winit");
    let data_device_state = DataDeviceState::new::<GenesiState>(&display_handle);

    // Cria o Monitor Virtual (Output)
    let output = Output::new(
        "Genesi-Monitor-1".into(),
        PhysicalProperties {
            size: (0, 0).into(),
            subpixel: Subpixel::Unknown,
            make: "Genesi".into(),
            model: "Virtual Display".into(),
            serial_number: "000001".into(),
        },
    );
    let _global = output.create_global::<GenesiState>(&display_handle);
    
    // Configura o tamanho e taxa de atualização do monitor
    let mode = OutputMode {
        size: (1280, 720).into(),
        refresh: 60_000,
    };
    output.change_current_state(
        Some(mode),
        Some(smithay::utils::Transform::Normal),
        Some(smithay::output::Scale::Integer(1)),
        Some((0, 0).into())
    );
    output.set_preferred(mode);

    let mut state = GenesiState {
        compositor_state,
        shm_state,
        xdg_shell_state,
        xdg_decoration_state,
        seat_state,
        seat: seat.clone(),
        data_device_state,
        output_manager_state,
        output: output.clone(),
        pointer_location: (0.0, 0.0).into(),
        window_positions: std::collections::HashMap::new(),
        window_order: Vec::new(),
        moving_window: None,
        resizing_window: None,
        minimized_windows: Vec::new(),
        unmaximized_state: std::collections::HashMap::new(),
        hovered_edge: None,
        cursor_status: smithay::input::pointer::CursorImageStatus::default_named(),
    };

    use smithay::wayland::socket::ListeningSocketSource;
    let source = ListeningSocketSource::new_auto()?;
    let socket_name = source.socket_name().to_string_lossy().into_owned();
    
    std::env::set_var("WAYLAND_DISPLAY", &socket_name);
    info!("🌐 Servidor Wayland escutando no socket: {}", socket_name);
    
    // Salva o socket em um arquivo para que o Tauri no Windows possa encontrar
    std::fs::write("/tmp/genesi-wayland-socket.txt", &socket_name)
        .unwrap_or_else(|e| tracing::warn!("Falha ao salvar socket: {}", e));

    let mut display_handle_clone = display_handle.clone();
    loop_handle.insert_source(source, move |client_stream, _, _state| {
        if let Err(err) = display_handle_clone.insert_client(client_stream, std::sync::Arc::new(ClientState::default())) {
            tracing::warn!("Erro ao adicionar cliente: {}", err);
        }
    }).map_err(|e| Box::new(e) as Box<dyn std::error::Error>)?;

    loop_handle.insert_source(
        Generic::new(display, Interest::READ, Mode::Level),
        |_, display, data| {
            unsafe {
                display.get_mut().dispatch_clients(data).unwrap();
            }
            Ok(PostAction::Continue)
        },
    ).map_err(|e| Box::new(e) as Box<dyn std::error::Error>)?;

    info!("✅ Genesi OS ativo e aguardando aplicativos!");
    info!("Pressione Ctrl+C para encerrar.");

    let keyboard = seat.add_keyboard(Default::default(), 200, 200).unwrap();
    let pointer = seat.add_pointer();

    loop {
        let status = winit.dispatch_new_events(|event| match event {
            WinitEvent::Input(event) => match event {
                InputEvent::Keyboard { event } => {
                    use smithay::backend::input::Event;
                    keyboard.input::<(), _>(
                        &mut state,
                        event.key_code(),
                        event.state(),
                        0.into(),
                        event.time_msec(),
                        |_, _, _| FilterResult::Forward,
                    );
                }
                InputEvent::PointerMotionAbsolute { event } => {
                    use smithay::backend::input::{Event, AbsolutePositionEvent};
                    let size = backend.window_size();
                    let position = event.position_transformed(size.to_logical(1));
                    state.pointer_location = position;
                    
                    // Se estiver arrastando uma janela, movemos ela!
                    if let Some((ref surface, offset)) = state.moving_window {
                        let new_x = position.x as i32 - offset.x;
                        let new_y = position.y as i32 - offset.y;
                        state.window_positions.insert(surface.clone(), (new_x, new_y).into());
                    }
                    
                    // Se estiver redimensionando uma janela, calculamos o novo tamanho
                    if let Some(ref rs) = state.resizing_window.clone() {
                        let dx = position.x - rs.start_pointer.x;
                        let dy = position.y - rs.start_pointer.y;
                        let mut new_x = rs.start_rect.loc.x;
                        let mut new_y = rs.start_rect.loc.y;
                        let mut new_w = rs.start_rect.size.w;
                        let mut new_h = rs.start_rect.size.h;
                        
                        // ResizeEdge é um enum, não bitflags — tratar cada variante
                        use xdg_toplevel::ResizeEdge as RE;
                        let (resize_left, resize_right, resize_top, resize_bottom) = match rs.edges {
                            RE::Top         => (false, false, true,  false),
                            RE::Bottom      => (false, false, false, true),
                            RE::Left        => (true,  false, false, false),
                            RE::Right       => (false, true,  false, false),
                            RE::TopLeft     => (true,  false, true,  false),
                            RE::TopRight    => (false, true,  true,  false),
                            RE::BottomLeft  => (true,  false, false, true),
                            RE::BottomRight => (false, true,  false, true),
                            _ => (false, false, false, false),
                        };
                        
                        if resize_right {
                            new_w = (rs.start_rect.size.w as f64 + dx).max(200.0) as i32;
                        }
                        if resize_bottom {
                            new_h = (rs.start_rect.size.h as f64 + dy).max(150.0) as i32;
                        }
                        if resize_left {
                            let max_delta_x = rs.start_rect.size.w - 200;
                            let delta = (dx as i32).min(max_delta_x);
                            new_x = rs.start_rect.loc.x + delta;
                            new_w = rs.start_rect.size.w - delta;
                        }
                        if resize_top {
                            let max_delta_y = rs.start_rect.size.h - 150;
                            let delta = (dy as i32).min(max_delta_y);
                            new_y = rs.start_rect.loc.y + delta;
                            new_h = rs.start_rect.size.h - delta;
                        }
                        
                        state.window_positions.insert(rs.surface.clone(), (new_x, new_y).into());
                        
                        // Envia o novo tamanho para o cliente via configure
                        if let Some(toplevel) = state.xdg_shell_state.toplevel_surfaces().iter().find(|t| t.wl_surface() == &rs.surface) {
                            toplevel.with_pending_state(|s| {
                                s.size = Some((new_w, new_h).into());
                            });
                            toplevel.send_configure();
                        }
                    }
                    
                    let mut under = None;
                    
                    // Verifica popups (menus) primeiro, pois eles ficam no topo absoluto
                    for surface in state.xdg_shell_state.popup_surfaces().into_iter().rev() {
                        let loc = surface.with_pending_state(|s| s.geometry.loc);
                        let size = surface.with_pending_state(|s| s.geometry.size);
                        if position.x >= loc.x as f64 && position.y >= loc.y as f64 && position.x < (loc.x + size.w) as f64 && position.y < (loc.y + size.h) as f64 {
                            under = Some((surface.wl_surface().clone(), Point::from((loc.x as f64, loc.y as f64))));
                            break;
                        }
                    }
                    
                    if under.is_none() {
                        // O Z-Index das janelas: O desktop fica atrás, as outras ficam na frente
                        // Iteramos do topo (fim da lista) para o fundo (começo da lista) para focar
                        // na janela que está visualmente na frente do mouse
                        for surface in state.window_order.iter().rev() {
                            // Pega as informações de tamanho através do toplevel real
                            if let Some(toplevel) = state.xdg_shell_state.toplevel_surfaces().iter().find(|t| t.wl_surface() == surface) {
                                let is_desktop = state.window_order.first() == Some(surface);
                                
                                let pos = state.window_positions.get(surface).cloned().unwrap_or((0, 0).into());
                                let base_x = if is_desktop { 0.0 } else { pos.x as f64 };
                                let base_y = if is_desktop { 0.0 } else { pos.y as f64 };
                                
                                use smithay::wayland::compositor::with_states;
                                use smithay::wayland::shell::xdg::SurfaceCachedState;
                                
                                let geometry = with_states(toplevel.wl_surface(), |states| {
                                    states.cached_state.get::<SurfaceCachedState>()
                                        .current()
                                        .geometry
                                        .unwrap_or(Rectangle::new((0, 0).into(), (800, 600).into()))
                                });
                                
                                let visual_x = base_x + geometry.loc.x as f64;
                                let visual_y = base_y + geometry.loc.y as f64;
                                let visual_w = geometry.size.w as f64;
                                let visual_h = geometry.size.h as f64;
                                
                                let titlebar_height = if is_desktop { 0.0 } else {
                                    // Verifica se o app usa CSD (ou se é o genesi)
                                    let app_id = smithay::wayland::compositor::with_states(toplevel.wl_surface(), |states| {
                                        states.data_map.get::<std::sync::Mutex<smithay::wayland::shell::xdg::XdgToplevelSurfaceRoleAttributes>>()
                                            .map(|attrs| attrs.lock().unwrap().app_id.clone())
                                            .flatten()
                                    }).unwrap_or_default();
                                    
                                    use smithay::reexports::wayland_protocols::xdg::decoration::zv1::server::zxdg_toplevel_decoration_v1::Mode as DecoMode;
                                    let client_handles_decorations = toplevel.with_pending_state(|s| s.decoration_mode) == Some(DecoMode::ClientSide);
                                    
                                    if app_id.contains("genesi") || client_handles_decorations { 0.0 } else { 30.0 }
                                };
                                
                                // A bounding box deve incluir a barra de título (y - titlebar_height)
                                if position.x >= visual_x && position.y >= visual_y - titlebar_height && position.x < visual_x + visual_w && position.y < visual_y + visual_h {
                                    under = Some((surface.clone(), Point::from((base_x, base_y))));
                                    break;
                                }
                            }
                        }
                    }
                    
                    let mut new_hovered_edge = None;
                    if let Some((surface, _)) = under.as_ref() {
                        if let Some(toplevel) = state.xdg_shell_state.toplevel_surfaces().iter().find(|t| t.wl_surface() == surface) {
                            let is_desktop = state.window_order.first() == Some(surface);
                            if !is_desktop && state.moving_window.is_none() && state.resizing_window.is_none() {
                                let pos = state.window_positions.get(surface).cloned().unwrap_or((0, 0).into());
                                use smithay::wayland::compositor::with_states;
                                use smithay::wayland::shell::xdg::SurfaceCachedState;
                                let geometry = with_states(surface, |states| {
                                    states.cached_state.get::<SurfaceCachedState>()
                                        .current()
                                        .geometry
                                        .unwrap_or(Rectangle::new((0, 0).into(), (800, 600).into()))
                                });
                                let visual_x = pos.x as f64 + geometry.loc.x as f64;
                                let visual_y = pos.y as f64 + geometry.loc.y as f64;
                                let visual_w = geometry.size.w as f64;
                                let visual_h = geometry.size.h as f64;
                                
                                let titlebar_height = {
                                    let app_id = with_states(surface, |states| {
                                        states.data_map.get::<std::sync::Mutex<smithay::wayland::shell::xdg::XdgToplevelSurfaceRoleAttributes>>()
                                            .map(|attrs| attrs.lock().unwrap().app_id.clone())
                                            .flatten()
                                    }).unwrap_or_default();
                                    use smithay::reexports::wayland_protocols::xdg::decoration::zv1::server::zxdg_toplevel_decoration_v1::Mode as DecoMode;
                                    let client_handles_decorations = toplevel.with_pending_state(|s| s.decoration_mode) == Some(DecoMode::ClientSide);
                                    if app_id.contains("genesi") || client_handles_decorations { 0.0 } else { 30.0 }
                                };
                                
                                let edge_thickness = 8.0;
                                let mut edges = xdg_toplevel::ResizeEdge::None;
                                
                                if position.x >= visual_x && position.x < visual_x + edge_thickness {
                                    edges = xdg_toplevel::ResizeEdge::Left;
                                } else if position.x >= visual_x + visual_w - edge_thickness && position.x < visual_x + visual_w {
                                    edges = xdg_toplevel::ResizeEdge::Right;
                                }
                                
                                if position.y >= visual_y - titlebar_height && position.y < visual_y - titlebar_height + edge_thickness {
                                    if edges == xdg_toplevel::ResizeEdge::Left {
                                        edges = xdg_toplevel::ResizeEdge::TopLeft;
                                    } else if edges == xdg_toplevel::ResizeEdge::Right {
                                        edges = xdg_toplevel::ResizeEdge::TopRight;
                                    } else {
                                        edges = xdg_toplevel::ResizeEdge::Top;
                                    }
                                } else if position.y >= visual_y + visual_h - edge_thickness && position.y < visual_y + visual_h {
                                    if edges == xdg_toplevel::ResizeEdge::Left {
                                        edges = xdg_toplevel::ResizeEdge::BottomLeft;
                                    } else if edges == xdg_toplevel::ResizeEdge::Right {
                                        edges = xdg_toplevel::ResizeEdge::BottomRight;
                                    } else {
                                        edges = xdg_toplevel::ResizeEdge::Bottom;
                                    }
                                }
                                
                                if edges != xdg_toplevel::ResizeEdge::None {
                                    new_hovered_edge = Some((surface.clone(), edges));
                                }
                            }
                        }
                    }
                    
                    if state.hovered_edge != new_hovered_edge {
                        state.hovered_edge = new_hovered_edge.clone();
                        let cursor_icon = match new_hovered_edge {
                            Some((_, edges)) => {
                                use xdg_toplevel::ResizeEdge as RE;
                                match edges {
                                    RE::Top => ::winit::window::CursorIcon::NResize,
                                    RE::Bottom => ::winit::window::CursorIcon::SResize,
                                    RE::Left => ::winit::window::CursorIcon::WResize,
                                    RE::Right => ::winit::window::CursorIcon::EResize,
                                    RE::TopLeft => ::winit::window::CursorIcon::NwResize,
                                    RE::TopRight => ::winit::window::CursorIcon::NeResize,
                                    RE::BottomLeft => ::winit::window::CursorIcon::SwResize,
                                    RE::BottomRight => ::winit::window::CursorIcon::SeResize,
                                    _ => ::winit::window::CursorIcon::Default,
                                }
                            },
                            None => ::winit::window::CursorIcon::Default,
                        };
                        backend.window().set_cursor(cursor_icon);
                    }
                    
                    if let Some((surface, _)) = under.as_ref() {
                        // Focus on mouse move? Usually we focus on click.
                        // Let's only set keyboard focus here if we want follow-mouse, otherwise just pointer.
                        // For a real OS, we focus on click. But we can leave it or just set pointer focus.
                        keyboard.set_focus(&mut state, Some(surface.clone()), 0.into());
                    }

                    pointer.motion(
                        &mut state,
                        under,
                        &smithay::input::pointer::MotionEvent {
                            location: position,
                            serial: 0.into(),
                            time: event.time_msec(),
                        },
                    );
                    pointer.frame(&mut state);
                }
                InputEvent::PointerButton { event } => {
                    use smithay::backend::input::{Event, PointerButtonEvent};
                    let serial = 0.into();
                    let button = event.button_code();
                    let state_btn = event.state();

                    // Lógica de arrastar e Z-Index no clique do mouse
                    if state_btn == smithay::backend::input::ButtonState::Pressed {
                        let position = state.pointer_location;
                        let mut clicked_surface = None;
                        
                        if let Some((ref surface, edges)) = state.hovered_edge.clone() {
                            let pos = state.window_positions.get(surface).cloned().unwrap_or((0, 0).into());
                            use smithay::wayland::compositor::with_states;
                            use smithay::wayland::shell::xdg::SurfaceCachedState;
                            let geometry = with_states(surface, |states| {
                                states.cached_state.get::<SurfaceCachedState>()
                                    .current()
                                    .geometry
                                    .unwrap_or(Rectangle::new((0, 0).into(), (800, 600).into()))
                            });
                            state.resizing_window = Some(ResizingState {
                                surface: surface.clone(),
                                edges,
                                start_pointer: position,
                                start_rect: Rectangle::new(pos, geometry.size),
                            });
                            clicked_surface = Some(surface.clone());
                        } else {
                            // Descobre em qual janela clicamos (do topo para o fundo)
                            for surface in state.window_order.iter().rev() {
                            if let Some(toplevel) = state.xdg_shell_state.toplevel_surfaces().iter().find(|t| t.wl_surface() == surface) {
                                let is_desktop = state.window_order.first() == Some(surface);
                                let pos = state.window_positions.get(surface).cloned().unwrap_or((0, 0).into());
                                let base_x = if is_desktop { 0.0 } else { pos.x as f64 };
                                let base_y = if is_desktop { 0.0 } else { pos.y as f64 };
                                
                                use smithay::wayland::compositor::with_states;
                                use smithay::wayland::shell::xdg::SurfaceCachedState;
                                
                                let geometry = with_states(toplevel.wl_surface(), |states| {
                                    states.cached_state.get::<SurfaceCachedState>()
                                        .current()
                                        .geometry
                                        .unwrap_or(Rectangle::new((0, 0).into(), (800, 600).into()))
                                });
                                
                                let visual_x = base_x + geometry.loc.x as f64;
                                let visual_y = base_y + geometry.loc.y as f64;
                                let visual_w = geometry.size.w as f64;
                                let visual_h = geometry.size.h as f64;
                                
                                let app_id = smithay::wayland::compositor::with_states(toplevel.wl_surface(), |states| {
                                    states.data_map.get::<std::sync::Mutex<smithay::wayland::shell::xdg::XdgToplevelSurfaceRoleAttributes>>()
                                        .map(|attrs| attrs.lock().unwrap().app_id.clone())
                                        .flatten()
                                }).unwrap_or_default();
                                let is_genesi_app = app_id.contains("genesi");
                                // Verifica se o cliente negociou CSD
                                use smithay::reexports::wayland_protocols::xdg::decoration::zv1::server::zxdg_toplevel_decoration_v1::Mode as DecoMode;
                                let client_handles_decorations = toplevel.with_pending_state(|s| s.decoration_mode) == Some(DecoMode::ClientSide);
                                
                                let titlebar_height = if is_desktop || is_genesi_app || client_handles_decorations { 0.0 } else { 30.0 };
                                
                                // Verifica se o clique está na barra de título do SO (apenas para apps externos)
                                if !is_desktop && !is_genesi_app && position.x >= visual_x && position.y >= visual_y - titlebar_height && position.x < visual_x + visual_w && position.y < visual_y {
                                    
                                    // Layout macOS: botões coloridos no lado ESQUERDO
                                    // [10px] [●close 12px] [6px] [●minimize 12px] [6px] [●maximize 12px]
                                    let btn_y_top = visual_y - titlebar_height;
                                    let btn_y_bot = btn_y_top + 30.0;
                                    let close_x1 = visual_x + 10.0;
                                    let close_x2 = close_x1 + 12.0;
                                    let minim_x1 = close_x2 + 6.0;
                                    let minim_x2 = minim_x1 + 12.0;
                                    let maxim_x1 = minim_x2 + 6.0;
                                    let maxim_x2 = maxim_x1 + 12.0;
                                    
                                    if position.x >= close_x1 && position.x < close_x2 {
                                        // Botão vermelho = Fechar
                                        toplevel.send_close();
                                        clicked_surface = None;
                                        break;
                                    } else if position.x >= minim_x1 && position.x < minim_x2 {
                                        // Botão amarelo = Minimizar
                                        let wl = surface.clone();
                                        if !state.minimized_windows.contains(&wl) {
                                            state.minimized_windows.push(wl);
                                        }
                                        clicked_surface = None;
                                        break;
                                    } else if position.x >= maxim_x1 && position.x < maxim_x2 {
                                        // Botão verde = Maximizar/Restaurar
                                        let is_maximized = toplevel.with_pending_state(|s| s.states.contains(xdg_toplevel::State::Maximized));
                                        let wl = surface.clone();
                                        
                                        if is_maximized {
                                            // Restaurar
                                            if let Some((old_pos, old_size)) = state.unmaximized_state.remove(&wl) {
                                                state.window_positions.insert(wl, old_pos);
                                                toplevel.with_pending_state(|s| {
                                                    s.states.unset(xdg_toplevel::State::Maximized);
                                                    s.size = Some(old_size);
                                                });
                                            } else {
                                                toplevel.with_pending_state(|s| {
                                                    s.states.unset(xdg_toplevel::State::Maximized);
                                                    s.size = None;
                                                });
                                            }
                                        } else {
                                            // Maximizar
                                            let current_size = geometry.size;
                                            state.unmaximized_state.insert(wl.clone(), (pos, current_size));
                                            let screen_size = backend.window_size().to_logical(1);
                                            let max_size = smithay::utils::Size::from((screen_size.w, screen_size.h - titlebar_height as i32));
                                            
                                            state.window_positions.insert(wl, (0, titlebar_height as i32).into());
                                            toplevel.with_pending_state(|s| {
                                                s.states.set(xdg_toplevel::State::Maximized);
                                                s.size = Some(max_size);
                                            });
                                        }
                                        toplevel.send_configure();
                                        clicked_surface = Some(surface.clone());
                                        break;
                                    } else {
                                        // Clicou na barra fora dos botões = Arraste
                                        let offset_x = position.x as i32 - pos.x;
                                        let offset_y = position.y as i32 - pos.y;
                                        state.moving_window = Some((surface.clone(), (offset_x, offset_y).into()));
                                        clicked_surface = Some(surface.clone());
                                        break;
                                    }
                                }
                                
                                // Verifica se clicou no corpo da janela
                                if position.x >= visual_x && position.y >= visual_y && position.x < visual_x + visual_w && position.y < visual_y + visual_h {
                                    clicked_surface = Some(surface.clone());
                                    break;
                                }
                            }
                        }
                        }
                        
                        // Z-Index: traz a janela clicada para a frente (se não for o desktop)
                        if let Some(surface) = clicked_surface {
                            let is_desktop = state.window_order.first() == Some(&surface);
                            if !is_desktop {
                                if let Some(index) = state.window_order.iter().position(|s| s == &surface) {
                                    let s = state.window_order.remove(index);
                                    state.window_order.push(s);
                                }
                            }
                        }
                    } else if state_btn == smithay::backend::input::ButtonState::Released {
                        // Para de arrastar e redimensionar
                        state.moving_window = None;
                        state.resizing_window = None;
                    }

                    pointer.button(
                        &mut state,
                        &smithay::input::pointer::ButtonEvent {
                            button,
                            state: state_btn,
                            serial,
                            time: event.time_msec(),
                        },
                    );
                    pointer.frame(&mut state);
                }
                _ => {}
            },
            _ => (),
        });

        match status {
            PumpStatus::Continue => (),
            PumpStatus::Exit(_) => return Ok(()),
        };

        let size = backend.window_size();
        let size_logical = size.to_logical(1);

        // Atualiza a resolução do monitor virtual se a janela Winit mudar de tamanho
        let mode = OutputMode {
            size: (size_logical.w, size_logical.h).into(),
            refresh: 60_000,
        };
        output.change_current_state(
            Some(mode),
            Some(smithay::utils::Transform::Normal),
            Some(smithay::output::Scale::Integer(1)),
            Some((0, 0).into())
        );

        let damage = Rectangle::from_size(size);
        let mut set_cursor_visible = true;
        
        {
            let (renderer, mut framebuffer) = backend.bind().unwrap();
            let mut elements: Vec<CustomRenderElements<GlesRenderer>> = Vec::new();

            // Desenha as janelas principais
            // A ordem em state.window_order é de Fundo para o Topo
            for surface in state.window_order.iter() {
                // Não renderiza janelas minimizadas
                if state.minimized_windows.contains(surface) {
                    continue;
                }
                
                if let Some(toplevel) = state.xdg_shell_state.toplevel_surfaces().iter().find(|t| t.wl_surface() == surface) {
                    let is_desktop = state.window_order.first() == Some(surface);

                    if is_desktop {
                        let current_size = toplevel.with_pending_state(|s| s.size);
                        let target_size = Some((size_logical.w, size_logical.h).into());
                        
                        // Força a janela do desktop a ficar do tamanho da tela e em estado de fullscreen
                        toplevel.with_pending_state(|s| {
                            s.size = target_size;
                            s.states.set(xdg_toplevel::State::Fullscreen);
                            s.states.set(xdg_toplevel::State::Maximized);
                        });
                        
                        if current_size != target_size {
                            toplevel.send_configure();
                        }
                    }

                    // Desenha a janela
                    let pos = state.window_positions.get(surface).cloned().unwrap_or((0, 0).into());
                    let x = if is_desktop { 0.0 } else { pos.x as f64 };
                    let y = if is_desktop { 0.0 } else { pos.y as f64 };
                    let x_i32 = x as i32;
                    let y_i32 = y as i32;
                    
                    // Ensure surface is mapped (has a buffer attached) before rendering
                    if !toplevel.is_initial_configure_sent() {
                        continue;
                    }

                    // O Smithay (dependendo da versão) pode desenhar de FRENTE para TRÁS (Front-to-Back) na lista de elements!
                    // Ou seja, o elemento no índice 0 fica no TOPO, e o elemento no índice N fica no FUNDO.
                    // Para garantir que o desktop fique no FUNDO, devemos adicioná-lo ao FINAL da lista (extend).
                    // E as janelas dos apps devem ser adicionadas ao COMEÇO da lista (splice 0..0) para ficarem no TOPO.
                    // ATENÇÃO: Como estamos iterando do Fundo pro Topo em `window_order`, a ÚLTIMA janela iterada
                    // será a que vai ficar no índice 0 de elements se continuarmos fazendo splice(0..0)!
                    if is_desktop {
                        let desktop_elements: Vec<WaylandSurfaceRenderElement<GlesRenderer>> = render_elements_from_surface_tree(
                            renderer,
                            toplevel.wl_surface(),
                            (x_i32, y_i32), // Posiciona a janela no desktop
                            1.0,
                            1.0,
                            Kind::Unspecified,
                        );
                        // Desktop no fundo -> final da lista
                        elements.extend(desktop_elements.into_iter().map(CustomRenderElements::from));
                    } else {
                        // Verifica se o cliente usa CSD (Client-Side Decorations)
                        use smithay::reexports::wayland_protocols::xdg::decoration::zv1::server::zxdg_toplevel_decoration_v1::Mode as DecoMode;
                        let decoration_mode = toplevel.with_pending_state(|s| s.decoration_mode);
                        let client_handles_decorations = decoration_mode == Some(DecoMode::ClientSide);
                        
                        // DESENHAR BARRA DE TÍTULO (SSD) apenas para apps que NÃO são nativos do Genesi OS
                        // E que necessitam ser desenhados pelo Window Manager
                        let app_id = smithay::wayland::compositor::with_states(toplevel.wl_surface(), |states| {
                            states.data_map.get::<std::sync::Mutex<smithay::wayland::shell::xdg::XdgToplevelSurfaceRoleAttributes>>()
                                .map(|attrs| attrs.lock().unwrap().app_id.clone())
                                .flatten()
                        }).unwrap_or_default();
                        let is_genesi_app = app_id.contains("genesi");
                        let needs_ssd = !is_genesi_app && !client_handles_decorations;
                        let titlebar_height = if needs_ssd { 30 } else { 0 };
                        
                        // Pegamos a geometria exata da janela (se o cliente tiver sombra/CSD, isso ignora as sombras invisíveis!)
                        use smithay::wayland::compositor::with_states;
                        use smithay::wayland::shell::xdg::SurfaceCachedState;
                        
                        let geometry = with_states(toplevel.wl_surface(), |states| {
                            states.cached_state.get::<SurfaceCachedState>()
                                .current()
                                .geometry
                                .unwrap_or(Rectangle::new((0, 0).into(), (800, 600).into()))
                        });
                        
                        let surface_size = toplevel.with_pending_state(|s| s.size).unwrap_or((800, 600).into());
                        
                        // O x_i32 e y_i32 do loop principal referem-se à posição base da surface.
                        // Mas a geometria do cliente (geometry.loc) nos diz onde a janela 'real' começa!
                        let visual_x = x_i32 + geometry.loc.x;
                        let visual_y = y_i32 + geometry.loc.y;
                        let visual_w = geometry.size.w;
                        
                        use smithay::backend::renderer::{element::Id, utils::CommitCounter};
                        
                        let app_surfaces: Vec<WaylandSurfaceRenderElement<GlesRenderer>> = render_elements_from_surface_tree(
                            renderer,
                            toplevel.wl_surface(),
                            (x_i32, y_i32), // Posiciona a janela normal
                            1.0,
                            1.0,
                            Kind::Unspecified,
                        );
                        let mut app_elements: Vec<CustomRenderElements<GlesRenderer>> = app_surfaces.into_iter().map(CustomRenderElements::from).collect();
                        
                        // ═══════════════════════════════════════════════════════════
                        // DESENHO DA BARRA DE TÍTULO (SSD) — Estilo macOS/Genesi
                        // Layout: [10px] [●close] [6px] [●min] [6px] [●max] [...]
                        // ═══════════════════════════════════════════════════════════
                        if needs_ssd {
                            // Barra de fundo escura (padrão Genesi)
                            let titlebar_geom = Rectangle::new(
                                (visual_x, visual_y - titlebar_height).into(), 
                                (visual_w, titlebar_height).into()
                            ).to_physical(1);
                            
                            let titlebar = SolidColorRenderElement::new(
                                Id::new(),
                                titlebar_geom,
                                CommitCounter::default(),
                                Color32F::new(0.12, 0.12, 0.13, 1.0), // #1E1E21 — Escuro premium
                                Kind::Unspecified,
                            );
                            app_elements.push(CustomRenderElements::from(titlebar));
                            
                            // Dimensões dos botõezinhos estilo macOS
                            let btn_size = 12;
                            let btn_y = visual_y - titlebar_height + (titlebar_height - btn_size) / 2; // Centralizado vertical
                            let btn_gap = 6;
                            let btn_start_x = visual_x + 10; // 10px de padding esquerdo
                            
                            // ● Botão FECHAR (Vermelho) — #ED544F
                            let close_geom = Rectangle::new(
                                (btn_start_x, btn_y).into(),
                                (btn_size, btn_size).into()
                            ).to_physical(1);
                            let close_btn = SolidColorRenderElement::new(
                                Id::new(), close_geom, CommitCounter::default(),
                                Color32F::new(0.93, 0.33, 0.31, 1.0),
                                Kind::Unspecified,
                            );
                            app_elements.push(CustomRenderElements::from(close_btn));
                            
                            // ● Botão MINIMIZAR (Amarelo/Âmbar) — #F7BD2E
                            let min_x = btn_start_x + btn_size + btn_gap;
                            let min_geom = Rectangle::new(
                                (min_x, btn_y).into(),
                                (btn_size, btn_size).into()
                            ).to_physical(1);
                            let min_btn = SolidColorRenderElement::new(
                                Id::new(), min_geom, CommitCounter::default(),
                                Color32F::new(0.97, 0.74, 0.18, 1.0),
                                Kind::Unspecified,
                            );
                            app_elements.push(CustomRenderElements::from(min_btn));
                            
                            // ● Botão MAXIMIZAR (Verde) — #27C93F
                            let max_x = min_x + btn_size + btn_gap;
                            let max_geom = Rectangle::new(
                                (max_x, btn_y).into(),
                                (btn_size, btn_size).into()
                            ).to_physical(1);
                            let max_btn = SolidColorRenderElement::new(
                                Id::new(), max_geom, CommitCounter::default(),
                                Color32F::new(0.15, 0.79, 0.25, 1.0),
                                Kind::Unspecified,
                            );
                            app_elements.push(CustomRenderElements::from(max_btn));
                        }
                        
                        // App no topo -> início da lista (O último do window_order será o 0 na elements!)
                        elements.splice(0..0, app_elements);
                    }
                }
            }

            // Desenha os popups (menus e tooltips do Firefox)
            for surface in state.xdg_shell_state.popup_surfaces() {
                // Ensure popup is mapped
                if !surface.is_initial_configure_sent() {
                    continue;
                }
                
                let location: Point<i32, Logical> = surface.with_pending_state(|state| state.geometry.loc);
                let popup_surfaces: Vec<WaylandSurfaceRenderElement<GlesRenderer>> = render_elements_from_surface_tree(
                    renderer,
                    surface.wl_surface(),
                    (location.x, location.y), // Posiciona o menu no lugar exato que o cliente pediu (sem bordas da janela principal)
                    1.0,
                    1.0,
                    Kind::Unspecified,
                );
                let popup_elements: Vec<CustomRenderElements<GlesRenderer>> = popup_surfaces.into_iter().map(CustomRenderElements::from).collect();
                // Popups no topo absoluto -> início da lista
                elements.splice(0..0, popup_elements);
            }

            // Desenha o Cursor customizado (se fornecido pelo cliente Wayland)
             let mut reset_cursor = false;
             let mut custom_cursor_drawn = false;
             let force_native_cursor = state.hovered_edge.is_some();
             
             if !force_native_cursor {
                 if let smithay::input::pointer::CursorImageStatus::Surface(ref surface) = state.cursor_status {
                     if !smithay::utils::IsAlive::alive(surface) {
                         reset_cursor = true;
                     } else {
                         use std::sync::Mutex;
                         use smithay::input::pointer::CursorImageAttributes;
                         use smithay::wayland::compositor;
                         
                         let hotspot = compositor::with_states(surface, |states| {
                             states.data_map
                                 .get::<Mutex<CursorImageAttributes>>()
                                 .map(|attrs| attrs.lock().unwrap().hotspot)
                                 .unwrap_or_else(|| (0, 0).into())
                         });
                         
                         let cursor_pos = state.pointer_location;
                         let cursor_x = cursor_pos.x as i32 - hotspot.x;
                         let cursor_y = cursor_pos.y as i32 - hotspot.y;
                         
                         let cursor_surfaces: Vec<WaylandSurfaceRenderElement<GlesRenderer>> = render_elements_from_surface_tree(
                             renderer,
                             surface,
                             (cursor_x, cursor_y),
                             1.0,
                             1.0,
                             Kind::Cursor,
                         );
                         let cursor_elements: Vec<CustomRenderElements<GlesRenderer>> = cursor_surfaces.into_iter().map(CustomRenderElements::from).collect();
                         elements.splice(0..0, cursor_elements);
                         
                         custom_cursor_drawn = true;
                     }
                 }
             }
             
             if reset_cursor {
                 state.cursor_status = smithay::input::pointer::CursorImageStatus::default_named();
             }
             
             // Oculta o cursor nativo se o app Wayland estiver desenhando o próprio cursor
              if force_native_cursor {
                  set_cursor_visible = true;
              } else if custom_cursor_drawn || matches!(state.cursor_status, smithay::input::pointer::CursorImageStatus::Hidden) {
                  set_cursor_visible = false;
              } else {
                  set_cursor_visible = true;
              }
  
              let mut frame = renderer.render(&mut framebuffer, size, Transform::Flipped180).unwrap();
            frame.clear(Color32F::new(0.05, 0.05, 0.1, 1.0), &[damage]).unwrap();
            draw_render_elements(&mut frame, 1.0, &elements, &[damage]).unwrap();
            let _ = frame.finish().unwrap();

            for surface in state.xdg_shell_state.toplevel_surfaces() {
                send_frames_surface_tree(surface.wl_surface(), start_time.elapsed().as_millis() as u32);
            }

            let _ = display_handle.flush_clients();
        }

        backend.window().set_cursor_visible(set_cursor_visible);
        backend.submit(Some(&[damage])).unwrap();
        event_loop.dispatch(Some(std::time::Duration::from_millis(16)), &mut state)?;
    }
}
