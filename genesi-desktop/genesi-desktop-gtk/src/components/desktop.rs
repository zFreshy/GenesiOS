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

        // Wallpaper/Background
        let welcome_label = Label::new(Some("🔥 Bem-vindo ao Genesi OS"));
        welcome_label.add_css_class("welcome-text");
        welcome_label.set_valign(gtk4::Align::Center);
        welcome_label.set_halign(gtk4::Align::Center);

        container.append(&welcome_label);

        Self { container }
    }

    pub fn widget(&self) -> Box {
        self.container.clone()
    }
}
