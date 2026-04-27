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
            ("web-browser", "Browser", "google-chrome", 0.0, 400.0),
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
        // Usamos GestureClick no lugar de Button para não conflitar com os eventos de arrastar
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

        // Drag com GestureDrag (a forma correta do GTK4 para arrastar sem tremer)
        let drag_gesture = gtk4::GestureDrag::new();
        drag_gesture.set_button(1);

        let widget_start_x = Rc::new(RefCell::new(init_x));
        let widget_start_y = Rc::new(RefCell::new(init_y));

        // Drag Update - Move livremente acompanhando o mouse
        let container_clone2 = container.clone();
        let item_box_clone2 = item_box.clone();
        let widget_start_x_clone2 = widget_start_x.clone();
        let widget_start_y_clone2 = widget_start_y.clone();

        drag_gesture.connect_drag_update(move |_, offset_x, offset_y| {
            let start_x = *widget_start_x_clone2.borrow();
            let start_y = *widget_start_y_clone2.borrow();
            
            let new_x = start_x + offset_x;
            let new_y = start_y + offset_y;
            
            container_clone2.move_(&item_box_clone2, new_x, new_y);
        });

        // Drag End - Snap na Grid
        let container_clone3 = container.clone();
        let item_box_clone3 = item_box.clone();
        let widget_start_x_clone3 = widget_start_x.clone();
        let widget_start_y_clone3 = widget_start_y.clone();

        drag_gesture.connect_drag_end(move |_, offset_x, offset_y| {
            let start_x = *widget_start_x_clone3.borrow();
            let start_y = *widget_start_y_clone3.borrow();
            
            let new_x = start_x + offset_x;
            let new_y = start_y + offset_y;
            
            // Snap to grid
            let snapped_x = (new_x / GRID_SIZE).round() * GRID_SIZE;
            let snapped_y = (new_y / GRID_SIZE).round() * GRID_SIZE;
            
            let final_x = snapped_x.max(0.0).min(1800.0);
            let final_y = snapped_y.max(0.0).min(1000.0);
            
            *widget_start_x_clone3.borrow_mut() = final_x;
            *widget_start_y_clone3.borrow_mut() = final_y;
            
            container_clone3.move_(&item_box_clone3, final_x, final_y);
        });

        item_box.add_controller(drag_gesture);

        item_box
    }

    pub fn widget(&self) -> Fixed {
        self.container.clone()
    }
}
