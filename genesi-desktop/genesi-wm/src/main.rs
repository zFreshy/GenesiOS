use tracing::{info, Level};
use tracing_subscriber::FmtSubscriber;
use std::thread;
use std::time::Duration;

fn main() {
    // Inicializa o sistema de logs (isso vai ser vital pra gente debugar o sistema operacional)
    let subscriber = FmtSubscriber::builder()
        .with_max_level(Level::INFO)
        .finish();
    tracing::subscriber::set_global_default(subscriber)
        .expect("Falha ao inicializar o sistema de log do Genesi WM");

    info!("===========================================");
    info!("🚀 Iniciando o Genesi OS Window Manager...");
    info!("🛡️ Módulo de Privacidade e Finanças: Pronto para injetar");
    info!("===========================================");
    info!("O servidor Wayland base está sendo construído.");
    
    // Loop principal infinito para manter o "Sistema Operacional" vivo
    info!("Pressione Ctrl+C para desligar o Genesi OS.");
    loop {
        // O Smithay/Calloop vão assumir esse loop mais pra frente.
        // Por enquanto, apenas mantemos o processo vivo simulando o "idle" do SO.
        thread::sleep(Duration::from_secs(1));
    }
}
