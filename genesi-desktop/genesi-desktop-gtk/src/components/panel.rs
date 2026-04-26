use gtk4::prelude::*;
use gtk4::{Box, Label, Button, Orientation};
use chrono::Local;
use glib;

pub struct Panel {
    container: Box,
}

impl Panel {
    pub fn new() -> Self {
        let container = Box::new(Orientation::Horizontal, 10);
        container.add_css_class("panel");
        container.set_height_request(40);
        container.set_hexpand(true);

        // Logo/Menu
        let menu_button = Button::with_label("🔥 Genesi");
        menu_button.add_css_class("panel-button");
        menu_button.add_css_class("logo-button");
        
        // Relógio
        let clock_label = Label::new(Some(&Self::get_time()));
        clock_label.add_css_class("clock");
        
        // Atualiza relógio a cada segundo
        let clock_clone = clock_label.clone();
        glib::timeout_add_seconds_local(1, move || {
            clock_clone.set_text(&Self::get_time());
            glib::ControlFlow::Continue
        });

        // System tray (placeholder)
        let system_box = Box::new(Orientation::Horizontal, 5);
        system_box.add_css_class("system-tray");
        
        let wifi_button = Button::with_label("📶");
        wifi_button.add_css_class("panel-button");
        
        let battery_button = Button::with_label("🔋");
        battery_button.add_css_class("panel-button");
        
        let power_button = Button::with_label("⚡");
        power_button.add_css_class("panel-button");
        
        system_box.append(&wifi_button);
        system_box.append(&battery_button);
        system_box.append(&power_button);

        // Layout
        container.append(&menu_button);
        container.append(&clock_label);
        container.append(&system_box);
        
        // Centraliza o relógio
        clock_label.set_hexpand(true);
        clock_label.set_halign(gtk4::Align::Center);
        
        // System tray à direita
        system_box.set_halign(gtk4::Align::End);

        Self { container }
    }

    fn get_time() -> String {
        Local::now().format("%H:%M:%S").to_string()
    }

    pub fn widget(&self) -> Box {
        self.container.clone()
    }
}
