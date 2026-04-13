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
    fn focus_changed(&mut self, _seat: &Seat<Self>, _focused: Option<&WlSurface>) {}
    fn cursor_image(&mut self, _seat: &Seat<Self>, _image: smithay::input::pointer::CursorImageStatus) {}
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

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // Força a renderização via processador (Software) para evitar travamentos em Máquinas Virtuais
    // É o equivalente ao WEBKIT_DISABLE_DMABUF_RENDERER que você usava no Tauri!
    std::env::set_var("LIBGL_ALWAYS_SOFTWARE", "1");
    // O Winit do Smithay precisa rodar no ambiente do host (seja ele Wayland nativo do WSLg ou X11)
    // O WSLg usa o Wayland por padrão, e tentar usar X11 sem o xkbcommon-x11 vai causar panic.
    // Forçamos o winit a usar Wayland, que é o ambiente principal do WSLg
    std::env::set_var("WINIT_UNIX_BACKEND", "wayland");
    
    // Precisamos de acesso ao display do host para o winit, não apagar o WAYLAND_DISPLAY original
    // antes de instanciar a janela.
    let host_wayland_display = std::env::var("WAYLAND_DISPLAY").unwrap_or_else(|_| "wayland-0".to_string());

    let subscriber = FmtSubscriber::builder()
        .with_max_level(Level::INFO)
        .finish();
    tracing::subscriber::set_global_default(subscriber)
        .expect("Falha ao inicializar o sistema de log do Genesi WM");

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

    info!("✨ Iniciando o Genesi Desktop Environment automaticamente...");
    let _desktop_process = std::process::Command::new("npm")
        .arg("run")
        .arg("tauri")
        .arg("dev")
        .current_dir("../") // Volta para a raiz do genesi-desktop
        .env("WAYLAND_DISPLAY", &socket_name)
        .spawn()
        .expect("Falha ao iniciar o Genesi Desktop");

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
                                use smithay::wayland::shell::xdg::XdgToplevelSurfaceData;
                                
                                let geometry = with_states(toplevel.wl_surface(), |states| {
                                    states.data_map.get::<XdgToplevelSurfaceData>()
                                        .unwrap()
                                        .lock()
                                        .unwrap()
                                        .current
                                        .geometry
                                        .unwrap_or(Rectangle::from_size((800, 600)).with_loc((0, 0).into()))
                                });
                                
                                let visual_x = base_x + geometry.loc.x as f64;
                                let visual_y = base_y + geometry.loc.y as f64;
                                let visual_w = geometry.size.w as f64;
                                let visual_h = geometry.size.h as f64;
                                
                                let titlebar_height = if is_desktop { 0.0 } else { 30.0 }; // Barra do topo
                                
                                // A bounding box deve incluir a barra de título (y - titlebar_height)
                                if position.x >= visual_x && position.y >= visual_y - titlebar_height && position.x < visual_x + visual_w && position.y < visual_y + visual_h {
                                    under = Some((surface.clone(), Point::from((base_x, base_y))));
                                    break;
                                }
                            }
                        }
                    }
                    
                    if let Some((surface, _)) = under.as_ref() {
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
                        
                        // Descobre em qual janela clicamos (do topo para o fundo)
                        for surface in state.window_order.iter().rev() {
                            if let Some(toplevel) = state.xdg_shell_state.toplevel_surfaces().iter().find(|t| t.wl_surface() == surface) {
                                let is_desktop = state.window_order.first() == Some(surface);
                                let pos = state.window_positions.get(surface).cloned().unwrap_or((0, 0).into());
                                let base_x = if is_desktop { 0.0 } else { pos.x as f64 };
                                let base_y = if is_desktop { 0.0 } else { pos.y as f64 };
                                
                                use smithay::wayland::compositor::with_states;
                                use smithay::wayland::shell::xdg::XdgToplevelSurfaceData;
                                
                                let geometry = with_states(toplevel.wl_surface(), |states| {
                                    states.data_map.get::<XdgToplevelSurfaceData>()
                                        .unwrap()
                                        .lock()
                                        .unwrap()
                                        .current
                                        .geometry
                                        .unwrap_or(Rectangle::from_size((800, 600)).with_loc((0, 0).into()))
                                });
                                
                                let visual_x = base_x + geometry.loc.x as f64;
                                let visual_y = base_y + geometry.loc.y as f64;
                                let visual_w = geometry.size.w as f64;
                                let visual_h = geometry.size.h as f64;
                                
                                let titlebar_height = if is_desktop { 0.0 } else { 30.0 };
                                
                                // Verifica se o clique está na barra de título (y - titlebar_height até y)
                                if !is_desktop && position.x >= visual_x && position.y >= visual_y - titlebar_height && position.x < visual_x + visual_w && position.y < visual_y {
                                    // Inicia o arrasto
                                    let offset_x = position.x as i32 - pos.x;
                                    let offset_y = position.y as i32 - pos.y;
                                    state.moving_window = Some((surface.clone(), (offset_x, offset_y).into()));
                                    clicked_surface = Some(surface.clone());
                                    break;
                                }
                                
                                // Verifica se clicou no corpo da janela
                                if position.x >= visual_x && position.y >= visual_y && position.x < visual_x + visual_w && position.y < visual_y + visual_h {
                                    clicked_surface = Some(surface.clone());
                                    break;
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
                        // Para de arrastar
                        state.moving_window = None;
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
        
        {
            let (renderer, mut framebuffer) = backend.bind().unwrap();
            let mut elements: Vec<CustomRenderElements<GlesRenderer>> = Vec::new();

            // Desenha as janelas principais
            // A ordem em state.window_order é de Fundo para o Topo
            for surface in state.window_order.iter() {
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
                        // DESENHAR BARRA DE TÍTULO (SSD)
                        let titlebar_height = 30;
                        // Pegamos a geometria exata da janela (se o cliente tiver sombra/CSD, isso ignora as sombras invisíveis!)
                        use smithay::wayland::compositor::with_states;
                        use smithay::wayland::shell::xdg::XdgToplevelSurfaceData;
                        
                        let geometry = with_states(toplevel.wl_surface(), |states| {
                            states.data_map.get::<XdgToplevelSurfaceData>()
                                .unwrap()
                                .lock()
                                .unwrap()
                                .current
                                .geometry
                                .unwrap_or(Rectangle::from_size((800, 600)).with_loc((0, 0).into()))
                        });
                        
                        let surface_size = toplevel.with_pending_state(|s| s.size).unwrap_or((800, 600).into());
                        
                        // O x_i32 e y_i32 do loop principal referem-se à posição base da surface.
                        // Mas a geometria do cliente (geometry.loc) nos diz onde a janela 'real' começa!
                        let visual_x = x_i32 + geometry.loc.x;
                        let visual_y = y_i32 + geometry.loc.y;
                        let visual_w = geometry.size.w;
                        
                        use smithay::backend::renderer::{element::Id, utils::CommitCounter};
                        
                        let titlebar_geom = Rectangle::from_size(
                            (visual_w, titlebar_height)
                        ).to_physical(1).with_loc((visual_x, visual_y - titlebar_height).into());
                        
                        // Criamos o retângulo da barra de título
                        let titlebar = SolidColorRenderElement::new(
                            Id::new(),
                            titlebar_geom,
                            CommitCounter::default(),
                            Color32F::new(0.15, 0.15, 0.15, 1.0), // Cor: Cinza Escuro (Padrão Genesi)
                            Kind::Unspecified,
                        );
                        
                        let app_surfaces: Vec<WaylandSurfaceRenderElement<GlesRenderer>> = render_elements_from_surface_tree(
                            renderer,
                            toplevel.wl_surface(),
                            (x_i32, y_i32), // Posiciona a janela normal
                            1.0,
                            1.0,
                            Kind::Unspecified,
                        );
                        let mut app_elements: Vec<CustomRenderElements<GlesRenderer>> = app_surfaces.into_iter().map(CustomRenderElements::from).collect();
                        
                        // Adiciona a titlebar na lista de elementos do app
                        app_elements.push(CustomRenderElements::from(titlebar));
                        
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

            let mut frame = renderer.render(&mut framebuffer, size, Transform::Flipped180).unwrap();
            frame.clear(Color32F::new(0.05, 0.05, 0.1, 1.0), &[damage]).unwrap();
            draw_render_elements(&mut frame, 1.0, &elements, &[damage]).unwrap();
            let _ = frame.finish().unwrap();

            for surface in state.xdg_shell_state.toplevel_surfaces() {
                send_frames_surface_tree(surface.wl_surface(), start_time.elapsed().as_millis() as u32);
            }

            let _ = display_handle.flush_clients();
        }

        backend.submit(Some(&[damage])).unwrap();
        event_loop.dispatch(Some(std::time::Duration::from_millis(16)), &mut state)?;
    }
}
