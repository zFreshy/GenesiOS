use gtk4::prelude::*;
use gtk4::{Box, Button, Orientation};
use crate::utils::{launch_app, get_installed_apps};

pub struct Dock {
    container: Box,
}

impl Dock {
    pub fn new() -> Self {
        let container = Box::new(Orientation::Horizontal, 10);
        container.add_css_class("dock");
        container.set_height_request(64);
        container.set_halign(gtk4::Align::Center);
        container.set_valign(gtk4::Align::End);
        container.set_margin_bottom(10);

        // Adiciona apps
        for app in get_installed_apps() {
            let button = Button::with_label(&app.icon);
            button.add_css_class("dock-button");
            button.set_tooltip_text(Some(&app.name));
            
            let command = app.command.clone();
            button.connect_clicked(move |_| {
                launch_app(&command);
            });
            
            container.append(&button);
        }

        Self { container }
    }

    pub fn widget(&self) -> Box {
        self.container.clone()
    }
}
