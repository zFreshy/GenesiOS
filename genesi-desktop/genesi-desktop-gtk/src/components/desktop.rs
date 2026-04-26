use gtk4::prelude::*;
use gtk4::{Box, Label, Orientation};

pub struct Desktop {
    container: Box,
}

impl Desktop {
    pub fn new() -> Self {
        let container = Box::new(Orientation::Vertical, 0);
        container.add_css_class("desktop");
        container.set_vexpand(true);
        container.set_hexpand(true);

        // Welcome message (estilo Tauri: ícone + texto grande)
        let welcome_box = Box::new(Orientation::Horizontal, 16);
        welcome_box.set_valign(gtk4::Align::Center);
        welcome_box.set_halign(gtk4::Align::Center);
        
        let icon_label = Label::new(Some("⚡"));
        icon_label.add_css_class("welcome-icon");
        welcome_box.append(&icon_label);
        
        let text_label = Label::new(Some("Bem-vindo ao Genesi"));
        text_label.add_css_class("welcome-text");
        welcome_box.append(&text_label);

        container.append(&welcome_box);

        Self { container }
    }

    pub fn widget(&self) -> Box {
        self.container.clone()
    }
}
