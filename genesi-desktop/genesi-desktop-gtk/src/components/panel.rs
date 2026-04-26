use gtk4::prelude::*;
use gtk4::{Box, Label, Button, Orientation, Separator};
use chrono::Local;
use glib;

pub struct Panel {
    container: Box,
}

impl Panel {
    pub fn new() -> Self {
        let container = Box::new(Orientation::Horizontal, 8);
        container.add_css_class("panel");
        container.set_height_request(40);
        container.set_hexpand(true);

        // Botão G (estilo macOS)
        let g_button = Button::with_label("G");
        g_button.add_css_class("g-button");
        container.append(&g_button);

        // Separador vertical
        let separator1 = Separator::new(Orientation::Vertical);
        separator1.add_css_class("panel-separator");
        container.append(&separator1);

        // Ícones de apps (estilo macOS)
        let app_icons = vec!["🌐", "📁", "⚙️", "📊", "🎨", "⚡"];
        for icon in app_icons {
            let button = Button::with_label(icon);
            button.add_css_class("panel-app-icon");
            container.append(&button);
        }

        // Spacer (empurra o relógio para a direita)
        let spacer = Box::new(Orientation::Horizontal, 0);
        spacer.set_hexpand(true);
        container.append(&spacer);

        // System tray (direita)
        let system_box = Box::new(Orientation::Horizontal, 8);
        system_box.add_css_class("system-tray");
        
        // Relógio e data
        let time_box = Box::new(Orientation::Vertical, 0);
        time_box.add_css_class("time-box");
        
        let time_label = Label::new(Some(&Self::get_time()));
        time_label.add_css_class("time-label");
        time_box.append(&time_label);
        
        let date_label = Label::new(Some(&Self::get_date()));
        date_label.add_css_class("date-label");
        time_box.append(&date_label);
        
        system_box.append(&time_box);
        
        // Ícones do sistema
        let battery_icon = Label::new(Some("🔋"));
        battery_icon.add_css_class("system-icon");
        system_box.append(&battery_icon);
        
        let wifi_icon = Label::new(Some("📶"));
        wifi_icon.add_css_class("system-icon");
        system_box.append(&wifi_icon);
        
        let locale_label = Label::new(Some("BR"));
        locale_label.add_css_class("locale-label");
        system_box.append(&locale_label);
        
        container.append(&system_box);
        
        // Atualiza relógio a cada segundo
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
