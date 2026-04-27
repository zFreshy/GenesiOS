use gtk4::prelude::*;
use gtk4::{Box, Button, Label, Orientation};
use crate::utils::launch_app;
use chrono::Local;
use glib;

pub struct Dock {
    container: Box,
}

impl Dock {
    pub fn new() -> Self {
        let container = Box::new(Orientation::Horizontal, 16);
        container.add_css_class("taskbar");
        container.set_height_request(60);
        container.set_width_request(1100); // Max width from React
        
        tracing::info!("🎯 Criando Dock...");

        // Lado esquerdo: Botão G + Separador + Apps
        let left_box = Box::new(Orientation::Horizontal, 8);
        left_box.set_valign(gtk4::Align::Center);

        // Botão G
        let g_button = Button::with_label("G");
        g_button.add_css_class("start-btn");
        left_box.append(&g_button);
        
        tracing::info!("  ✅ Botão G criado");

        // Separador vertical
        let separator = Box::new(Orientation::Vertical, 0);
        separator.add_css_class("taskbar-separator");
        left_box.append(&separator);

        // Apps principais
        let apps = vec![
            ("🌐", "Genesi Browser", "firefox"),
            ("📁", "File Explorer", "nautilus"),
            ("⚙️", "Configurações", "gnome-control-center"),
            ("📊", "Task Manager", "gnome-system-monitor"),
            ("🔵", "Google Chrome", "chromium-browser"),
        ];

        let apps_box = Box::new(Orientation::Horizontal, 8);
        for (icon, name, command) in apps {
            let button = Button::with_label(icon);
            button.add_css_class("taskbar-icon");
            button.set_tooltip_text(Some(name));
            
            let cmd = command.to_string();
            button.connect_clicked(move |_| {
                launch_app(&cmd);
            });
            
            apps_box.append(&button);
        }
        left_box.append(&apps_box);
        
        container.append(&left_box);

        // Spacer para empurrar o Tray para a direita
        let spacer = Box::new(Orientation::Horizontal, 0);
        spacer.set_hexpand(true);
        container.append(&spacer);

        // Lado Direito: Tray System
        let right_box = Box::new(Orientation::Horizontal, 12);
        right_box.set_valign(gtk4::Align::Center);
        
        // Relógio (TaskbarClock)
        let clock_box = Box::new(Orientation::Vertical, 0);
        clock_box.set_valign(gtk4::Align::Center);
        clock_box.add_css_class("taskbar-clock-box");
        
        let time_label = Label::new(Some(&Self::get_time()));
        time_label.add_css_class("taskbar-time");
        clock_box.append(&time_label);
        
        let date_label = Label::new(Some(&Self::get_date()));
        date_label.add_css_class("taskbar-date");
        clock_box.append(&date_label);
        
        right_box.append(&clock_box);

        // Ícones de sistema
        let tray_box = Box::new(Orientation::Horizontal, 8);
        tray_box.add_css_class("system-tray");
        
        let battery_icon = Label::new(Some("🔋"));
        tray_box.append(&battery_icon);
        
        let wifi_icon = Label::new(Some("📶"));
        tray_box.append(&wifi_icon);
        
        let locale_label = Label::new(Some("BR"));
        locale_label.add_css_class("font-medium");
        tray_box.append(&locale_label);
        
        right_box.append(&tray_box);

        // Separador para o "Show Desktop"
        let show_desktop_sep = Box::new(Orientation::Vertical, 0);
        show_desktop_sep.add_css_class("show-desktop-separator");
        right_box.append(&show_desktop_sep);

        container.append(&right_box);
        
        tracing::info!("✅ Dock criado com sucesso!");
        
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
