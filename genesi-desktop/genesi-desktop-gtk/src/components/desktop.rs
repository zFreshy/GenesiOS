use gtk4::prelude::*;
use gtk4::{Application, Box, Button, FlowBox, Label, Orientation, Align, SelectionMode, Image, DragSource, DropTarget};
use crate::utils::launch_app;
use crate::components::file_explorer::FileExplorer;
use glib::Type;
use gdk4::ContentProvider;

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
            ("user-trash", "Recycle Bin", ""),
            ("system-file-manager", "Files", "genesifiles"),
            ("preferences-system", "Settings", "gnome-control-center"),
            ("utilities-system-monitor", "Task Manager", "gnome-system-monitor"),
            ("web-browser", "Browser", "google-chrome"),
        ];

        for (icon, name, cmd) in apps {
            let item_box = Box::new(Orientation::Vertical, 8);
            item_box.set_halign(Align::Center);
            item_box.set_valign(Align::Center);
            item_box.set_size_request(80, 100);
            item_box.add_css_class("desktop-icon-item");

            let img = Image::from_icon_name(icon);
            img.set_pixel_size(48);

            let btn = Button::builder().child(&img).build();
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

            // Drag and Drop (Source)
            let drag_source = DragSource::new();
            let name_str = name.to_string();
            drag_source.connect_prepare(move |_, _, _| {
                Some(ContentProvider::for_value(&name_str.to_value()))
            });
            item_box.add_controller(drag_source);

            // Drag and Drop (Target)
            let drop_target = DropTarget::new(Type::STRING, gdk4::DragAction::MOVE);
            drop_target.connect_drop(move |target, value, _x, _y| {
                if let Ok(dragged_name) = value.get::<String>() {
                    let parent = target.widget().parent().and_then(|p| p.parent());
                    if let Some(flowbox_parent) = parent.and_then(|p| p.downcast::<FlowBox>().ok()) {
                        // Find indices
                        let mut dragged_idx = -1;
                        let mut target_idx = -1;
                        let mut current_idx = 0;
                        
                        while let Some(child) = flowbox_parent.child_at_index(current_idx) {
                            if let Some(box_child) = child.child().and_then(|c| c.downcast::<Box>().ok()) {
                                if let Some(lbl) = box_child.last_child().and_then(|c| c.downcast::<Label>().ok()) {
                                    let text = lbl.text().to_string();
                                    if text == dragged_name { dragged_idx = current_idx; }
                                    if text == name { target_idx = current_idx; }
                                }
                            }
                            current_idx += 1;
                        }

                        if dragged_idx != -1 && target_idx != -1 && dragged_idx != target_idx {
                            if let Some(dragged_child) = flowbox_parent.child_at_index(dragged_idx) {
                                // We can change the sort order dynamically, or just remove and insert.
                                // FlowBox has no insert_child, but insert() wraps the widget in a child.
                                // To move it, we can extract the inner widget, remove the child, and re-insert the inner widget.
                                if let Some(inner_box) = dragged_child.child() {
                                    // Remove the FlowBoxChild wrapper
                                    flowbox_parent.remove(&dragged_child);
                                    // Re-insert the inner box at the target index
                                    flowbox_parent.insert(&inner_box, target_idx);
                                }
                            }
                        }
                    }
                }
                true
            });
            item_box.add_controller(drop_target);

            flowbox.insert(&item_box, -1);
        }

        container.append(&flowbox);

        Self { container }
    }

    pub fn widget(&self) -> Box {
        self.container.clone()
    }
}
