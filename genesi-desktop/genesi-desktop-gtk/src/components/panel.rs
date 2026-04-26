use gtk4::prelude::*;
use gtk4::{Box, Label, Orientation};
use chrono::Local;
use glib;

pub struct Panel {
    container: Box,
}

impl Panel {
    pub fn new() -> Self {
        let container = Box::new(Orientation::Horizontal, 0);
        container.add_css_class("panel");
        container.set_height_request(40);
        container.set_hexpand(true);

        // Logo (estilo Tauri: ícone + texto)
        let logo_box = Box::new(Orientation::Horizontal, 8);
        logo_box.set_margin_start(16);
        
        let logo_icon = Label::new(Some("⚡"));
        logo_icon.add_css_class("panel-logo-icon");
        logo_box.append(&logo_icon);
        
        let logo_text = Label::new(Some("Genesi"));
        logo_text.add_css_class("panel-logo-text");
        logo_box.append(&logo_text);
        
        container.append(&logo_box);

        // Spacer (empurra o relógio para a direita)
        let spacer = Box::new(Orientation::Horizontal, 0);
        spacer.set_hexpand(true);
        container.append(&spacer);

        // Relógio (formato HH:MM:SS igual ao Tauri)
        let clock_label = Label::new(Some(&Self::get_time()));
        clock_label.add_css_class("clock");
        clock_label.set_margin_end(16);
        container.append(&clock_label);
        
        // Atualiza relógio a cada segundo
        let clock_clone = clock_label.clone();
        glib::timeout_add_seconds_local(1, move || {
            clock_clone.set_text(&Self::get_time());
            glib::ControlFlow::Continue
        });

        Self { container }
    }

    fn get_time() -> String {
        Local::now().format("%H:%M:%S").to_string()
    }

    pub fn widget(&self) -> Box {
        self.container.clone()
    }
}
