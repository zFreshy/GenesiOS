use gtk4::prelude::*;
use gtk4::{Application, ApplicationWindow, Box, Orientation};

use crate::components::{panel::Panel, dock::Dock, desktop::Desktop};
use crate::config::load_css;

pub fn build_ui(app: &Application) {
    tracing::info!("🎨 Construindo interface do Genesi OS");

    // Carrega CSS customizado
    load_css();

    // Janela principal (fullscreen)
    let window = ApplicationWindow::builder()
        .application(app)
        .title("Genesi OS")
        .default_width(1280)
        .default_height(720)
        .fullscreened(true)
        .decorated(false)
        .build();

    // Container principal
    let main_box = Box::new(Orientation::Vertical, 0);
    main_box.add_css_class("main-container");

    // Componentes
    let panel = Panel::new();
    let desktop = Desktop::new();
    let dock = Dock::new();

    // Monta interface
    main_box.append(&panel.widget());
    main_box.append(&desktop.widget());
    main_box.append(&dock.widget());

    window.set_child(Some(&main_box));
    window.present();

    tracing::info!("✅ Interface construída com sucesso!");
}
