use gtk4::CssProvider;
use gdk4::Display;

pub fn load_css() {
    let provider = CssProvider::new();
    
    // Tenta vários caminhos possíveis para o CSS
    let possible_paths = vec![
        "resources/style.css",
        "../../../resources/style.css",
        "/home/genesi/GenesiOS/genesi-desktop/genesi-desktop-gtk/resources/style.css",
        "./style.css",
    ];
    
    let mut loaded = false;
    for path in possible_paths {
        if std::path::Path::new(path).exists() {
            tracing::info!("🎨 Carregando CSS de: {}", path);
            provider.load_from_path(path);
            loaded = true;
            break;
        }
    }
    
    if !loaded {
        tracing::error!("❌ CSS não encontrado! Tentei:");
        for path in possible_paths {
            tracing::error!("   - {}", path);
        }
        // Carrega CSS inline como fallback
        provider.load_from_data(include_str!("../resources/style.css"));
        tracing::warn!("⚠️  Usando CSS embutido (wallpaper pode não funcionar)");
    }

    gtk4::style_context_add_provider_for_display(
        &Display::default().expect("Could not connect to display"),
        &provider,
        gtk4::STYLE_PROVIDER_PRIORITY_APPLICATION,
    );

    tracing::info!("✅ CSS carregado com sucesso");
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
