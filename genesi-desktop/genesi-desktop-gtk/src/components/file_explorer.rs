use gtk4::prelude::*;
use gtk4::{Application, Window, Box, Orientation, Label, Button, ScrolledWindow, FlowBox, Align, SelectionMode, Separator, SearchEntry, Image};

pub struct FileExplorer;

impl FileExplorer {
    pub fn new(app: &Application) {
        let window = Window::builder()
            .application(app)
            .title("Genesi Files")
            .default_width(900)
            .default_height(600)
            .hide_on_close(true)
            .build();

        window.add_css_class("file-explorer-window");

        let main_box = Box::new(Orientation::Horizontal, 0);

        // ================= Sidebar =================
        let sidebar = Box::new(Orientation::Vertical, 8);
        sidebar.add_css_class("file-explorer-sidebar");
        sidebar.set_size_request(220, -1);
        
        let sidebar_title = Label::new(Some("Quick Access"));
        sidebar_title.add_css_class("sidebar-title");
        sidebar_title.set_halign(Align::Start);
        sidebar.append(&sidebar_title);

        let quick_access = vec![
            ("go-home", "Home"),
            ("user-desktop", "Desktop"),
            ("folder-download", "Downloads"),
            ("folder-documents", "Documents"),
            ("folder-pictures", "Pictures"),
            ("folder-music", "Music"),
            ("folder-videos", "Videos"),
        ];

        for (icon, name) in quick_access {
            let btn_box = Box::new(Orientation::Horizontal, 12);
            let img = Image::from_icon_name(icon);
            img.set_pixel_size(16);
            let name_lbl = Label::new(Some(name));
            
            btn_box.append(&img);
            btn_box.append(&name_lbl);
            
            let btn = Button::builder().child(&btn_box).build();
            btn.add_css_class("sidebar-btn");
            sidebar.append(&btn);
        }

        // ================= Main Content Area =================
        let content_box = Box::new(Orientation::Vertical, 0);
        content_box.set_hexpand(true);
        content_box.set_vexpand(true);
        content_box.add_css_class("file-explorer-content");

        // Topbar
        let topbar = Box::new(Orientation::Horizontal, 12);
        topbar.add_css_class("file-explorer-topbar");
        
        let back_btn = Button::with_label("◀");
        back_btn.add_css_class("topbar-btn");
        let forward_btn = Button::with_label("▶");
        forward_btn.add_css_class("topbar-btn");
        
        let path_label = Label::new(Some("/home/mathe/Desktop"));
        path_label.add_css_class("path-label");
        path_label.set_hexpand(true);
        path_label.set_halign(Align::Start);

        let search = SearchEntry::new();
        search.set_placeholder_text(Some("Search files..."));
        
        topbar.append(&back_btn);
        topbar.append(&forward_btn);
        topbar.append(&path_label);
        topbar.append(&search);

        // Separator
        let sep = Separator::new(Orientation::Horizontal);
        
        // Files Grid
        let scroll = ScrolledWindow::new();
        scroll.set_hexpand(true);
        scroll.set_vexpand(true);

        let flowbox = FlowBox::new();
        flowbox.set_valign(Align::Start);
        flowbox.set_halign(Align::Start);
        flowbox.set_selection_mode(SelectionMode::Single);
        flowbox.set_max_children_per_line(10);
        flowbox.set_column_spacing(16);
        flowbox.set_row_spacing(16);
        flowbox.set_margin_top(20);
        flowbox.set_margin_start(20);
        flowbox.set_margin_end(20);
        flowbox.set_margin_bottom(20);

        // Mock files
        let files = vec![
            ("folder", "Projects"),
            ("folder", "Wallpapers"),
            ("text-x-generic", "notes.txt"),
            ("image-x-generic", "photo.png"),
            ("audio-x-generic", "audio.mp3"),
            ("package-x-generic", "archive.zip"),
        ];

        for (icon, name) in files {
            let item_box = Box::new(Orientation::Vertical, 8);
            item_box.set_halign(Align::Center);
            item_box.set_valign(Align::Center);
            item_box.set_size_request(100, 100);
            item_box.add_css_class("file-item");

            let img = Image::from_icon_name(icon);
            img.set_pixel_size(64);
            
            let name_lbl = Label::new(Some(name));
            name_lbl.add_css_class("file-name");
            name_lbl.set_wrap(true);
            name_lbl.set_justify(gtk4::Justification::Center);

            item_box.append(&img);
            item_box.append(&name_lbl);

            flowbox.insert(&item_box, -1);
        }

        scroll.set_child(Some(&flowbox));

        content_box.append(&topbar);
        content_box.append(&sep);
        content_box.append(&scroll);

        main_box.append(&sidebar);
        
        let vsep = Separator::new(Orientation::Vertical);
        main_box.append(&vsep);
        
        main_box.append(&content_box);

        window.set_child(Some(&main_box));
        window.present();
    }
}
