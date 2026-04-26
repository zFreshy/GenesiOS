use gtk4::prelude::*;
use gtk4::{Box, Button, Label, Orientation, Separator};
use crate::utils::launch_app;
use chrono::Local;
use glib;

pub struct Dock {
    container: Box,
}

impl Dock {
    pub fn new() -> Self {
        let container = Box::new(Orientation::Horizontal, 8);
        container.add_css_class("dock");
        container.set_height_request(64);
        container.set_halign(gtk4::Align::Center);
        container.set_valign(gtk4::Align::End);
        container.set_margin_bottom(16);

        // Botão G (estilo macOS)
        let g_button = Button::with_label("G");
        g_button.add_css_class("dock-g-button");
        container.append(&g_button);

        // Separador vertical
        let separator = Separator::new(Orientation::Vertical);
        separator.add_css_class("dock-separator");
        container.append(&separator);

        // Apps principais (ícones grandes estilo macOS)
        let apps = vec![
            ("🌐", "Browser", "chromium-browser"),
            ("📁", "Files", "nautilus"),
            ("⚙️", "Settings", "gnome-control-center"),
            ("📊", "System", "gnome-system-monitor"),
            ("🎨", "Terminal", "gnome-terminal"),
            ("⚡", "About", "gnome-control-center"),
        ];

        for (icon, name, command) in apps {
            let button = Button::with_label(icon);
            button.add_css_class("dock-app-button");
            button.set_tooltip_text(Some(name));
            
            let cmd = command.to_string();
            button.connect_clicked(move |_| {
                launch_app(&cmd);
            });
            
            container.append(&button);
        }

        // Spacer antes do system tray
        let spacer = Box::new(Orientation::Horizontal, 0);
        spacer.set_hexpand(true);
        container.append(&spacer);

        // System tray (direita)
        let system_box = Box::new(Orientation::Horizontal, 8);
        system_box.add_css_class("dock-system-tray");
        
        // Relógio e data
        let time_box = Box::new(Orientation::Vertical, 0);
        time_box.add_css_class("dock-time-box");
        
        let time_label = Label::new(Some(&Self::get_time()));
        time_label.add_css_class("dock-time-label");
        time_box.append(&time_label);
        
        let date_label = Label::new(Some(&Self::get_date()));
        date_label.add_css_class("dock-date-label");
        time_box.append(&date_label);
        
        system_box.append(&time_box);
        
        // Ícones do sistema
        let battery_icon = Label::new(Some("🔋"));
        battery_icon.add_css_class("dock-system-icon");
        system_box.append(&battery_icon);
        
        let wifi_icon = Label::new(Some("📶"));
        wifi_icon.add_css_class("dock-system-icon");
        system_box.append(&wifi_icon);
        
        let locale_label = Label::new(Some("BR"));
        locale_label.add_css_class("dock-locale-label");
        system_box.append(&locale_label);
        
        container.append(&system_box);
        
        // Atualiza relógio
        let time_clone = time_label.clone();
        let date_clone = date_label.clone();
        glib::timeout_add_seconds_local(1, move || {
            time_clone.set_text(&Self::get_time());
            date_clone.set_text(&Self::get_date());
            glib::ControlFlow::Continue
        });

        Self { container }
    }

    fn get_time() -> String {
        Local::now().format("%I:%M %p").to_string()
    }

    fn get_date() -> String {
        Local::now().format("%m/%d/%Y").to_string()
    }

    pub fn widget(&self) -> Box {
        self.container.clone()
    }
}
