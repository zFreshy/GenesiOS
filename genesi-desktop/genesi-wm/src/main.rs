use tracing::{info, warn, Level};
use tracing_subscriber::FmtSubscriber;
use wayland_server::Display;
use calloop::{EventLoop, loop_utils::Signals};
use std::os::unix::io::AsRawFd;

// Estrutura principal do nosso Sistema Operacional
pub struct GenesiState {
    // Aqui no futuro vamos guardar:
    // - Lista de janelas abertas (Firefox, VSCode, etc)
    // - Posição do mouse
    // - Monitores detectados
}

// Estrutura wrapper para transformar o Display numa Source válida do Calloop
struct WaylandSource(Display<GenesiState>);

impl calloop::EventSource for WaylandSource {
    type Event = ();
    type Metadata = ();
    type Ret = ();
    type Error = std::io::Error;

    fn process_events(
        &mut self,
        _readiness: calloop::Readiness,
        _token: calloop::Token,
        mut callback: impl FnMut((), &mut ()) -> Result<(), std::io::Error>,
    ) -> Result<calloop::PostAction, std::io::Error> {
        self.0.dispatch_clients(&mut GenesiState {}).map_err(|_| std::io::Error::last_os_error())?;
        self.0.flush_clients().map_err(|_| std::io::Error::last_os_error())?;
        callback((), &mut ())?;
        Ok(calloop::PostAction::Continue)
    }

    fn register(&mut self, poll: &mut calloop::Poll, tokenFactory: &mut calloop::TokenFactory) -> calloop::Result<()> {
        // Implementação básica usando o File Descriptor do Wayland Server
        let fd = self.0.backend().poll_fd();
        poll.register(
            fd.as_raw_fd(),
            calloop::Interest::READ,
            calloop::Mode::Level,
            tokenFactory.token(),
        )
    }

    fn reregister(&mut self, poll: &mut calloop::Poll, tokenFactory: &mut calloop::TokenFactory) -> calloop::Result<()> {
        let fd = self.0.backend().poll_fd();
        poll.reregister(
            fd.as_raw_fd(),
            calloop::Interest::READ,
            calloop::Mode::Level,
            tokenFactory.token(),
        )
    }

    fn unregister(&mut self, poll: &mut calloop::Poll) -> calloop::Result<()> {
        let fd = self.0.backend().poll_fd();
        poll.unregister(fd.as_raw_fd())
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    // 1. Inicializa o sistema de logs
    let subscriber = FmtSubscriber::builder()
        .with_max_level(Level::INFO)
        .finish();
    tracing::subscriber::set_global_default(subscriber)
        .expect("Falha ao inicializar o sistema de log do Genesi WM");

    info!("===========================================");
    info!("🚀 Iniciando o Genesi OS Window Manager...");
    info!("🛡️ Motor Wayland (Smithay): Carregando");
    info!("===========================================");

    // 2. Cria o servidor de Display do Wayland (onde os apps vão se conectar)
    let mut display: Display<GenesiState> = Display::new()?;
    let display_handle = display.handle();

    // 3. Cria o Loop de Eventos (O coração que pulsa o sistema)
    let mut event_loop: EventLoop<GenesiState> = EventLoop::try_new()?;
    let loop_handle = event_loop.handle();

    // 4. Cria o socket do Wayland (ex: wayland-1) para os apps acharem a gente
    let socket_name = wayland_server::ListeningSocket::bind_auto("wayland", 1..10)?;
    let socket_name_str = socket_name.socket_name().to_string_lossy().into_owned();
    
    // Configura para que o sistema saiba qual é o display do Genesi OS
    std::env::set_var("WAYLAND_DISPLAY", &socket_name_str);
    info!("🌐 Servidor Wayland escutando no socket: {}", socket_name_str);

    // 5. Configura interrupções de sistema (Ctrl+C para desligar limpo)
    let signals = Signals::new(&[calloop::loop_utils::Signal::SIGINT, calloop::loop_utils::Signal::SIGTERM])?;
    loop_handle.insert_source(signals, |_, _, _| {
        warn!("Sinal recebido, desligando o Genesi OS...");
        // Para parar o event_loop, nós não retornamos nada que chame stop aqui,
        // apenas tratamos a lógica se precisasse, mas no Calloop 0.13, 
        // precisamos de um token ou tratar o erro fora.
    })?;

    // Insere o Display no Event Loop usando o generic impl do calloop para Wayland Display
    loop_handle.insert_source(
        calloop::Generic::new(display_handle.clone().into(), calloop::Interest::READ, calloop::Mode::Level),
        |_, _, _state| {
            // Callback do Display do Wayland (Quando chegam conexões)
            Ok(calloop::PostAction::Continue)
        },
    ).map_err(|e| Box::new(e) as Box<dyn std::error::Error>)?;

    // 6. O Estado principal que será passado para os callbacks
    let mut state = GenesiState {};

    info!("✅ Genesi OS ativo e aguardando aplicativos!");
    info!("Pressione Ctrl+C para encerrar.");

    // 7. Roda o loop principal (aqui ele fica vivo e escutando conexões)
    // O timeout None significa que ele vai dormir até algum evento acontecer (ex: mexer o mouse, app conectar)
    event_loop.run(None, &mut state, |_| {})?;

    Ok(())
}
