use gtk4::CssProvider;
use gdk4::Display;

pub fn load_css() {
    let provider = CssProvider::new();
    
    // CSS embutido no binário (mais confiável para ISO)
    let css_data = include_str!("../resources/style.css");
    provider.load_from_data(css_data);

    gtk4::style_context_add_provider_for_display(
        &Display::default().expect("Could not connect to display"),
        &provider,
        gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION,
    );

    tracing::info!("✅ CSS carregado com sucesso (embutido)");
}

// Config struct (não usado por enquanto, mas pode ser útil no futuro)
#[allow(dead_code)]
pub struct Config {
    pub panel_height: i32,
    pub dock_height: i32,
    pub icon_size: i32,
}

#[allow(dead_code)]
impl Config {
    pub fn default() -> Self {
        Self {
            panel_height: 40,
            dock_height: 64,
            icon_size: 48,
        }
    }
}
