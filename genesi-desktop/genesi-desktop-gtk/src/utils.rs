use std::process::Command;

/// Lança um aplicativo externo
pub fn launch_app(command: &str) {
    tracing::info!("🚀 Lançando app: {}", command);
    
    match Command::new(command).spawn() {
        Ok(_) => tracing::info!("✅ App {} lançado com sucesso", command),
        Err(e) => tracing::error!("❌ Erro ao lançar {}: {}", command, e),
    }
}

/// Retorna lista de apps instalados
pub fn get_installed_apps() -> Vec<AppInfo> {
    vec![
        AppInfo {
            name: "Firefox".to_string(),
            command: "firefox".to_string(),
            icon: "firefox".to_string(),
        },
        AppInfo {
            name: "Terminal".to_string(),
            command: "gnome-terminal".to_string(),
            icon: "terminal".to_string(),
        },
        AppInfo {
            name: "Files".to_string(),
            command: "nautilus".to_string(),
            icon: "folder".to_string(),
        },
    ]
}

#[derive(Clone, Debug)]
pub struct AppInfo {
    pub name: String,
    pub command: String,
    pub icon: String,
}
