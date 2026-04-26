use gtk4::prelude::*;
use gtk4::{Box, Button, Orientation};
use crate::utils::{launch_app, get_installed_apps};

pub struct Dock {
    container: Box,
}

impl Dock {
    pub fn new() -> Self {
        let container = Box::new(Orientation::Horizontal, 12);
        container.add_css_class("dock");
        container.set_height_request(72);
        container.set_halign(gtk4::Align::Center);
        container.set_valign(gtk4::Align::End);
        container.set_margin_bottom(16);

        // Apps principais (estilo Tauri)
        let apps = vec![
            ("🌐", "Browser", "chromium-browser"),
            ("📁", "Files", "nautilus"),
            ("⚙️", "Settings", "gnome-control-center"),
            ("📊", "System", "gnome-system-monitor"),
            ("🎨", "Terminal", "gnome-terminal"),
        ];

        for (icon, name, command) in apps {
            let button = Button::with_label(icon);
            button.add_css_class("dock-button");
            button.set_tooltip_text(Some(name));
            
            let cmd = command.to_string();
            button.connect_clicked(move |_| {
                launch_app(&cmd);
            });
            
            container.append(&button);
        }

        Self { container }
    }

    pub fn widget(&self) -> Box {
        self.container.clone()
    }
}
