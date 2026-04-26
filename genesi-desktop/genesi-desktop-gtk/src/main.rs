mod app;
mod components;
mod config;
mod utils;

use gtk4::prelude::*;
use gtk4::{glib, Application};
use tracing_subscriber;

const APP_ID: &str = "com.genesi.desktop";

fn main() -> glib::ExitCode {
    // Inicializa logging
    tracing_subscriber::fmt()
        .with_max_level(tracing::Level::INFO)
        .init();

    tracing::info!("🔥 Iniciando Genesi OS Desktop Environment");

    // Força Wayland
    std::env::set_var("GDK_BACKEND", "wayland");
    
    // Cria aplicação GTK4
    let app = Application::builder()
        .application_id(APP_ID)
        .build();

    app.connect_activate(app::build_ui);

    app.run()
}
