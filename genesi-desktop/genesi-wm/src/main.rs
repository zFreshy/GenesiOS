use tracing::{info, warn, Level};
use tracing_subscriber::FmtSubscriber;
use wayland_server::{Display, Client};
use calloop::EventLoop;

use smithay::{
    delegate_compositor, delegate_shm,
    wayland::{
        buffer::BufferHandler,
        compositor::{CompositorHandler, CompositorState, CompositorClientState},
        shm::{ShmHandler, ShmState},
    },
    reexports::wayland_server::protocol::{wl_surface::WlSurface, wl_buffer::WlBuffer},
};

// O estado do cliente conectado (ex: um processo do Firefox)
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
    // Aqui no futuro vamos guardar as janelas e ponteiro do mouse
}

// =================================================================================
// IMPLEMENTAÇÃO DOS PROTOCOLOS DO WAYLAND VIA SMITHAY
// =================================================================================

// 1. Compositor: Permite que os aplicativos criem "Superfícies" (Janelas invisíveis)
impl CompositorHandler for GenesiState {
    fn compositor_state(&mut self) -> &mut CompositorState {
        &mut self.compositor_state
    }
    fn client_compositor_state<'c>(&self, client: &'c Client) -> &'c CompositorClientState {
        &client.get_data::<ClientState>().unwrap().compositor_state
    }
    fn commit(&mut self, surface: &WlSurface) {
        // Chamado sempre que o app (ex: Firefox) atualiza os pixels da tela
        info!("Quadro de vídeo recebido do app: {:?}", surface);
    }
}

// 2. SHM (Shared Memory): Permite que os aplicativos enviem blocos de memória RAM contendo a imagem
impl ShmHandler for GenesiState {
    fn shm_state(&self) -> &ShmState {
        &self.shm_state
    }
}

// 3. Buffer: Gerencia quando a memória de vídeo deve ser liberada
impl BufferHandler for GenesiState {
    fn buffer_destroyed(&mut self, _buffer: &WlBuffer) {
        // Limpar a memória quando a janela fecha
    }
}

// Macros do Smithay que geram todo o código pesado de conexão do Wayland nos bastidores!
delegate_compositor!(GenesiState);
delegate_shm!(GenesiState);

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let subscriber = FmtSubscriber::builder()
        .with_max_level(Level::INFO)
        .finish();
    tracing::subscriber::set_global_default(subscriber)
        .expect("Falha ao inicializar o sistema de log do Genesi WM");

    info!("===========================================");
    info!("🚀 Iniciando o Genesi OS Window Manager...");
    info!("🛡️ Motor Wayland (Smithay): Carregando módulos Base");
    info!("===========================================");

    let mut display: Display<GenesiState> = Display::new()?;
    let display_handle = display.handle();

    let mut event_loop: EventLoop<GenesiState> = EventLoop::try_new()?;
    let loop_handle = event_loop.handle();

    // Cria as estruturas principais do Smithay
    let compositor_state = CompositorState::new::<GenesiState>(&display_handle);
    let shm_state = ShmState::new::<GenesiState>(&display_handle, vec![]);

    let mut state = GenesiState {
        compositor_state,
        shm_state,
    };

    let socket = wayland_server::ListeningSocket::bind_auto("wayland", 1..10)?;
    let socket_name = socket.socket_name().unwrap_or_default().to_string_lossy().into_owned();
    
    std::env::set_var("WAYLAND_DISPLAY", &socket_name);
    info!("🌐 Servidor Wayland escutando no socket: {}", socket_name);

    // Injeta o Wayland Display (Servidor) no event loop usando as abstrações do Calloop
    loop_handle.insert_source(
        calloop::generic::Generic::new(display_handle.clone().into(), calloop::Interest::READ, calloop::Mode::Level),
        |_, _: &mut calloop::generic::NoIoDrop<rustix::event::PollFd>, _state| {
            Ok(calloop::PostAction::Continue)
        },
    ).map_err(|e| Box::new(e) as Box<dyn std::error::Error>)?;

    info!("✅ Genesi OS ativo e aguardando aplicativos!");
    info!("Pressione Ctrl+C para encerrar.");

    event_loop.run(None, &mut state, |_| {})?;

    Ok(())
}
