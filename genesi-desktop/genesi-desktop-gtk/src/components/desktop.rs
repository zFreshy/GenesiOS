use gtk4::prelude::*;
use gtk4::{Application, Box, Button, FlowBox, Label, Orientation, Align, SelectionMode};
use crate::utils::launch_app;
use crate::components::file_explorer::FileExplorer;

pub struct Desktop {
    container: Box,
}

impl Desktop {
    pub fn new(app: &Application) -> Self {
        let container = Box::new(Orientation::Vertical, 0);
        container.add_css_class("desktop");
        container.set_vexpand(true);
        container.set_hexpand(true);

        // Grade de ícones usando FlowBox
        let flowbox = FlowBox::new();
        flowbox.set_orientation(Orientation::Vertical);
        flowbox.set_valign(Align::Start);
        flowbox.set_halign(Align::Start);
        flowbox.set_selection_mode(SelectionMode::None);
        flowbox.set_max_children_per_line(10); // Quantidade de linhas na grade
        flowbox.set_margin_top(20);
        flowbox.set_margin_start(20);
        flowbox.set_margin_end(20);
        flowbox.set_margin_bottom(100); // Espaço para a taskbar
        flowbox.set_column_spacing(20);
        flowbox.set_row_spacing(20);

        let apps = vec![
            ("🗑️", "Recycle Bin", ""),
            ("📁", "Files", "genesifiles"),
            ("⚙️", "Settings", "gnome-control-center"),
            ("📊", "Task Manager", "gnome-system-monitor"),
            ("🔵", "Chrome", "google-chrome"),
        ];

        for (icon, name, cmd) in apps {
            let item_box = Box::new(Orientation::Vertical, 8);
            item_box.set_halign(Align::Center);
            item_box.set_valign(Align::Center);
            item_box.set_size_request(80, 100);
            item_box.add_css_class("desktop-icon-item");

            let btn = Button::with_label(icon);
            btn.add_css_class("desktop-icon-btn");
            btn.set_size_request(60, 60);
            
            let cmd_string = cmd.to_string();
            let app_clone = app.clone();
            btn.connect_clicked(move |_| {
                if cmd_string == "genesifiles" {
                    FileExplorer::new(&app_clone);
                } else if !cmd_string.is_empty() {
                    launch_app(&cmd_string);
                }
            });

            let label = Label::new(Some(name));
            label.add_css_class("desktop-icon-label");
            label.set_wrap(true);
            label.set_justify(gtk4::Justification::Center);

            item_box.append(&btn);
            item_box.append(&label);

            flowbox.insert(&item_box, -1);
        }

        container.append(&flowbox);

        Self { container }
    }

    pub fn widget(&self) -> Box {
        self.container.clone()
    }
}
