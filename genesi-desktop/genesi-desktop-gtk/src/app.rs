use gtk4::prelude::*;
use gtk4::{Application, ApplicationWindow, Box, Overlay, Orientation};

use crate::components::{dock::Dock, desktop::Desktop};
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

    // Container principal como Overlay
    let overlay = Overlay::new();
    overlay.add_css_class("main-container");

    // Imagem de fundo
    let bg_image = gtk4::Picture::for_filename("resources/wallpaper1.png");
    bg_image.set_can_shrink(true);
    bg_image.set_content_fit(gtk4::ContentFit::Cover);
    overlay.set_child(Some(&bg_image));

    // Desktop ao fundo
    let desktop = Desktop::new();
    overlay.add_overlay(&desktop.widget());

    // Dock (Taskbar) flutuante
    let dock = Dock::new();
    let dock_widget = dock.widget();
    dock_widget.set_valign(gtk4::Align::End);
    dock_widget.set_halign(gtk4::Align::Center);
    dock_widget.set_margin_bottom(20);
    
    overlay.add_overlay(&dock_widget);

    window.set_child(Some(&overlay));
    window.present();

    tracing::info!("✅ Interface construída com sucesso!");
}
