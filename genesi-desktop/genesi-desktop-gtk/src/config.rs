use gtk4::{CssProvider, StyleContext};
use gdk4::Display;

pub fn load_css() {
    let provider = CssProvider::new();
    provider.load_from_data(include_str!("../resources/style.css"));

    gtk4::style_context_add_provider_for_display(
        &Display::default().expect("Could not connect to display"),
        &provider,
        gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION,
    );

    tracing::info!("🎨 CSS carregado com sucesso");
}

pub struct Config {
    pub panel_height: i32,
    pub dock_height: i32,
    pub icon_size: i32,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            panel_height: 40,
            dock_height: 64,
            icon_size: 48,
        }
    }
}
