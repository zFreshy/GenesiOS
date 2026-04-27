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

    // Panel no topo
    let panel = Panel::new();
    main_box.append(&panel.widget());

    // Desktop no centro (com overlay para dock)
    let overlay = gtk4::Overlay::new();
    
    let desktop = Desktop::new(app);
    overlay.set_child(Some(&desktop.widget()));

    // Dock flutuante
    let dock = Dock::new();
    let dock_widget = dock.widget();
    dock_widget.set_valign(gtk4::Align::End);
    dock_widget.set_halign(gtk4::Align::Center);
    dock_widget.set_margin_bottom(20);
    overlay.add_overlay(&dock_widget);

    main_box.append(&overlay);

    window.set_child(Some(&main_box));
    window.present();

    tracing::info!("✅ Interface construída com sucesso!");
}
