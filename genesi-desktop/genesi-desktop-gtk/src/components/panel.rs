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
        container.set_height_request(32);
        container.set_hexpand(true);

        // Logo simples à esquerda
        let logo = Label::new(Some("⚡ Genesi"));
        logo.add_css_class("panel-logo");
        logo.set_margin_start(16);
        container.append(&logo);

        // Spacer
        let spacer = Box::new(Orientation::Horizontal, 0);
        spacer.set_hexpand(true);
        container.append(&spacer);

        // Relógio à direita
        let clock = Label::new(Some(&Self::get_time()));
        clock.add_css_class("panel-clock");
        clock.set_margin_end(16);
        container.append(&clock);
        
        // Atualiza relógio
        let clock_clone = clock.clone();
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
