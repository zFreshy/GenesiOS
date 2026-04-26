use gtk4::CssProvider;
use gdk4::Display;

pub fn load_css() {
    let provider = CssProvider::new();
    // Em vez de embutir o arquivo, carregamos ele do disco no momento da execução
    // Isso garante que os caminhos relativos de imagens, como `url('wallpaper1.png')`, funcionem!
    provider.load_from_path("resources/style.css");

    gtk4::style_context_add_provider_for_display(
        &Display::default().expect("Could not connect to display"),
        &provider,
        gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION,
    );

    tracing::info!("🎨 CSS carregado com sucesso");
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
