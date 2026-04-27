use gtk4::prelude::*;
use gtk4::{Application, Box, Button, Label, Orientation, Fixed, GestureClick, Image};
use crate::utils::launch_app;
use crate::components::file_explorer::FileExplorer;
use std::cell::RefCell;
use std::rc::Rc;

const GRID_SIZE: f64 = 100.0; // Tamanho da grade (snap)

pub struct Desktop {
    container: Fixed,
}

impl Desktop {
    pub fn new(app: &Application) -> Self {
        let container = Fixed::new();
        container.add_css_class("desktop");
        container.set_hexpand(true);
        container.set_vexpand(true);

        let apps = vec![
            ("user-trash", "Recycle Bin", "", 0.0, 0.0),
            ("system-file-manager", "Files", "genesifiles", 0.0, 100.0),
            ("preferences-system", "Settings", "gnome-control-center", 0.0, 200.0),
            ("utilities-system-monitor", "Task Manager", "gnome-system-monitor", 0.0, 300.0),
            ("web-browser", "Browser", "firefox", 0.0, 400.0),
        ];

        for (icon, name, cmd, init_x, init_y) in apps {
            let icon_widget = Self::create_desktop_icon(
                app,
                icon,
                name,
                cmd,
                init_x,
                init_y,
                &container,
            );
            container.put(&icon_widget, init_x, init_y);
        }

        let drop_target = gtk4::DropTarget::new(gtk4::glib::Type::STRING, gdk4::DragAction::MOVE);
        let container_clone = container.clone();
        drop_target.connect_drop(move |_, value, x, y| {
            if let Ok(dragged_name) = value.get::<String>() {
                // Encontrar qual widget tem o nome que está sendo arrastado
                let mut target_widget = None;
                let mut child_opt = container_clone.first_child();
                
                while let Some(w) = child_opt {
                    if let Ok(box_w) = w.clone().downcast::<Box>() {
                        if let Some(lbl_w) = box_w.last_child() {
                            if let Ok(lbl) = lbl_w.downcast::<Label>() {
                                if lbl.text().as_str() == dragged_name {
                                    target_widget = Some(box_w);
                                    break;
                                }
                            }
                        }
                    }
                    child_opt = w.next_sibling();
                }

                if let Some(widget) = target_widget {
                    // Centralizar o ícone no mouse
                    let center_x = x - 40.0; // metade da largura do ícone (80/2)
                    let center_y = y - 50.0; // metade da altura do ícone (100/2)

                    // Snap to grid
                    let snapped_x = (center_x / GRID_SIZE).round() * GRID_SIZE;
                    let snapped_y = (center_y / GRID_SIZE).round() * GRID_SIZE;
                    
                    let final_x = snapped_x.max(0.0).min(1800.0);
                    let final_y = snapped_y.max(0.0).min(1000.0);
                    
                    container_clone.move_(&widget, final_x, final_y);
                }
            }
            true
        });
        container.add_controller(drop_target);

        Self { container }
    }

    fn create_desktop_icon(
        app: &Application,
        icon_name: &str,
        label_text: &str,
        command: &str,
        init_x: f64,
        init_y: f64,
        container: &Fixed,
    ) -> Box {
        let item_box = Box::new(Orientation::Vertical, 8);
        item_box.set_size_request(80, 100);
        item_box.add_css_class("desktop-icon");

        // Ícone
        let img = Image::from_icon_name(icon_name);
        img.set_pixel_size(48);
        img.add_css_class("desktop-icon-btn");
        img.set_size_request(60, 60);

        // Label
        let label = Label::new(Some(label_text));
        label.add_css_class("desktop-icon-label");
        label.set_wrap(true);
        label.set_justify(gtk4::Justification::Center);
        label.set_max_width_chars(10);

        item_box.append(&img);
        item_box.append(&label);

        // Double click para abrir
        let click = GestureClick::new();
        click.set_button(1);
        let cmd_string = command.to_string();
        let app_clone = app.clone();
        
        click.connect_pressed(move |_, n_press, _, _| {
            if n_press == 2 {
                if cmd_string == "genesifiles" {
                    FileExplorer::new(&app_clone);
                } else if !cmd_string.is_empty() {
                    launch_app(&cmd_string);
                }
            }
        });
        item_box.add_controller(click);

        // Drag and Drop (Arrastar perfeito sem tremer)
        let drag_source = gtk4::DragSource::new();
        drag_source.set_actions(gdk4::DragAction::MOVE);
        let name_str = label_text.to_string();
        
        drag_source.connect_prepare(move |_, _x, _y| {
            Some(gdk4::ContentProvider::for_value(&name_str.to_value()))
        });
        item_box.add_controller(drag_source);

        item_box
    }

    pub fn widget(&self) -> Fixed {
        self.container.clone()
    }
}
