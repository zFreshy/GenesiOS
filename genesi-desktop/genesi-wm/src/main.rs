use tracing::{info, Level};
use tracing_subscriber::FmtSubscriber;
use wayland_server::{Display, Client};
use calloop::{EventLoop, Interest, Mode, PostAction, generic::Generic};

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

    let display: Display<GenesiState> = Display::new()?;
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

    use smithay::wayland::socket::ListeningSocketSource;
    let source = ListeningSocketSource::new_auto()?;
    let socket_name = source.socket_name().to_string_lossy().into_owned();
    
    std::env::set_var("WAYLAND_DISPLAY", &socket_name);
    info!("🌐 Servidor Wayland escutando no socket: {}", socket_name);

    let mut display_handle_clone = display_handle.clone();
    loop_handle.insert_source(source, move |client_stream, _, _state| {
        // Quando um app tentar conectar, aceitamos a conexão e registramos no servidor
        if let Err(err) = display_handle_clone.insert_client(client_stream, std::sync::Arc::new(ClientState::default())) {
            tracing::warn!("Erro ao adicionar cliente: {}", err);
        }
    }).map_err(|e| Box::new(e) as Box<dyn std::error::Error>)?;

    // O wayland-server moderno precisa do Generic e unsafe para gerenciar o display no calloop
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

    event_loop.run(None, &mut state, |_| {})?;

    Ok(())
}
