use std::fs;
use std::path::Path;
use std::time::UNIX_EPOCH;
use sysinfo::Disks;
use btleplug::api::{Central, Manager as _, Peripheral, ScanFilter};
use btleplug::platform::Manager;
use tokio::time;
use std::time::Duration;
use std::process::Command;

// Learn more about Tauri commands at https://tauri.app/develop/calling-rust/
#[tauri::command]
fn greet(name: &str) -> String {
    format!("Hello, {}! You've been greeted from Rust!", name)
}

#[derive(serde::Serialize)]
pub struct DiskInfo {
    name: String,
    mount_point: String,
    total_space: u64,
    available_space: u64,
}

#[tauri::command]
fn get_drives() -> Vec<DiskInfo> {
    let mut drives = Vec::new();
    let disks = Disks::new_with_refreshed_list();
    
    for disk in disks.list() {
        drives.push(DiskInfo {
            name: disk.name().to_string_lossy().into_owned(),
            mount_point: disk.mount_point().to_string_lossy().into_owned(),
            total_space: disk.total_space(),
            available_space: disk.available_space(),
        });
    }
    
    // If no disks found (can happen if sysinfo fails on some systems), fallback
    if drives.is_empty() {
        #[cfg(windows)]
        {
            for byte in b'A'..=b'Z' {
                let drive_path = format!("{}:\\", byte as char);
                if Path::new(&drive_path).exists() {
                    drives.push(DiskInfo {
                        name: format!("Local Disk ({}:)", byte as char),
                        mount_point: drive_path,
                        total_space: 0,
                        available_space: 0,
                    });
                }
            }
        }
        #[cfg(unix)]
        {
            drives.push(DiskInfo {
                name: "Root".to_string(),
                mount_point: "/".to_string(),
                total_space: 0,
                available_space: 0,
            });
        }
    }
    
    drives
}

#[derive(serde::Serialize)]
pub struct FileInfo {
    name: String,
    path: String,
    is_dir: bool,
    size: u64,
    modified_at: u64,
}

#[tauri::command]
fn read_dir(path: &str) -> Result<Vec<FileInfo>, String> {
    let mut entries = Vec::new();
    match fs::read_dir(path) {
        Ok(dir) => {
            for entry in dir.flatten() {
                let metadata = entry.metadata().map_err(|e| e.to_string())?;
                let modified_at = metadata.modified()
                    .unwrap_or(UNIX_EPOCH)
                    .duration_since(UNIX_EPOCH)
                    .unwrap_or_default()
                    .as_secs();

                entries.push(FileInfo {
                    name: entry.file_name().into_string().unwrap_or_default(),
                    path: entry.path().to_string_lossy().to_string(),
                    is_dir: metadata.is_dir(),
                    size: metadata.len(),
                    modified_at,
                });
            }
            // Sort: directories first, then alphabetically
            entries.sort_by(|a, b| {
                b.is_dir.cmp(&a.is_dir).then(a.name.to_lowercase().cmp(&b.name.to_lowercase()))
            });
            Ok(entries)
        }
        Err(e) => Err(e.to_string()),
    }
}

#[tauri::command]
fn read_file_bytes(path: &str) -> Result<Vec<u8>, String> {
    fs::read(path).map_err(|e| e.to_string())
}

#[tauri::command]
fn create_desktop_shortcut(target_path: &str, file_name: &str) -> Result<(), String> {
    #[cfg(windows)]
    {
        let ps_script = format!(
            "$WshShell = New-Object -comObject WScript.Shell; \
             $Shortcut = $WshShell.CreateShortcut([Environment]::GetFolderPath('Desktop') + '\\{}.lnk'); \
             $Shortcut.TargetPath = '{}'; \
             $Shortcut.Save()",
            file_name, target_path
        );
        
        let output = std::process::Command::new("powershell")
            .args(&["-NoProfile", "-Command", &ps_script])
            .output()
            .map_err(|e| e.to_string())?;
            
        if !output.status.success() {
            return Err(String::from_utf8_lossy(&output.stderr).to_string());
        }
        Ok(())
    }
    #[cfg(not(windows))]
    {
        // For non-Windows platforms, just return OK or implement symbolic link
        Err("Not implemented for this OS".into())
    }
}

#[derive(serde::Serialize)]
pub struct WifiNetwork {
    ssid: String,
    signal_level: i32,
    security: String,
}

#[tauri::command]
fn get_wifi_networks() -> Result<Vec<WifiNetwork>, String> {
    #[cfg(target_os = "windows")]
    {
        let output = Command::new("netsh")
            .args(&["wlan", "show", "networks", "mode=Bssid"])
            .output()
            .map_err(|e| format!("Failed to execute netsh: {}", e))?;

        let stdout = String::from_utf8_lossy(&output.stdout);
        let mut networks = Vec::new();
        let mut current_ssid = String::new();
        let mut current_security = String::new();
        let mut current_signal = 0;

        for line in stdout.lines() {
            let line = line.trim();
            if line.starts_with("SSID") {
                if !current_ssid.is_empty() {
                    networks.push(WifiNetwork {
                        ssid: current_ssid.clone(),
                        signal_level: current_signal,
                        security: current_security.clone(),
                    });
                }
                let parts: Vec<&str> = line.split(':').collect();
                if parts.len() > 1 {
                    current_ssid = parts[1].trim().to_string();
                }
                current_signal = 0;
                current_security = "Open".to_string();
            } else if line.starts_with("Authentication") || line.starts_with("Autenticação") {
                let parts: Vec<&str> = line.split(':').collect();
                if parts.len() > 1 {
                    current_security = parts[1].trim().to_string();
                }
            } else if line.starts_with("Signal") || line.starts_with("Sinal") {
                let parts: Vec<&str> = line.split(':').collect();
                if parts.len() > 1 {
                    let sig_str = parts[1].trim().trim_end_matches('%');
                    current_signal = sig_str.parse().unwrap_or(0);
                }
            }
        }

        if !current_ssid.is_empty() {
            networks.push(WifiNetwork {
                ssid: current_ssid,
                signal_level: current_signal,
                security: current_security,
            });
        }

        // Deduplicate networks by SSID, keeping the strongest signal
        networks.sort_by(|a, b| b.signal_level.cmp(&a.signal_level));
        networks.dedup_by(|a, b| a.ssid == b.ssid);

        Ok(networks)
    }

    #[cfg(not(target_os = "windows"))]
    {
        // Mock data for other OS for now
        Ok(vec![
            WifiNetwork { ssid: "Genesi_5G".to_string(), signal_level: 95, security: "WPA2".to_string() },
            WifiNetwork { ssid: "Guest_Wifi".to_string(), signal_level: 60, security: "Open".to_string() },
        ])
    }
}

#[tauri::command]
fn connect_wifi(ssid: String, password: Option<String>) -> Result<bool, String> {
    #[cfg(target_os = "windows")]
    {
        // On Windows, you typically need to create a profile XML first, 
        // but for simplicity we will just call netsh connect (which works if profile exists)
        // A full implementation would create the XML and add it using `netsh wlan add profile`
        
        let mut args = vec!["wlan", "connect", "name=", &ssid];
        
        let output = Command::new("netsh")
            .args(&["wlan", "connect", &format!("name={}", ssid)])
            .output()
            .map_err(|e| format!("Failed to connect: {}", e))?;

        if String::from_utf8_lossy(&output.stdout).contains("successfully") || String::from_utf8_lossy(&output.stdout).contains("com êxito") {
            Ok(true)
        } else {
            Err("Failed to connect to the network. Note: For new networks, a profile must be created first.".to_string())
        }
    }

    #[cfg(not(target_os = "windows"))]
    {
        Ok(true)
    }
}

#[derive(serde::Serialize)]
pub struct BluetoothDevice {
    id: String,
    name: String,
    is_connected: bool,
}

#[tauri::command]
async fn get_bluetooth_devices() -> Result<Vec<BluetoothDevice>, String> {
    let manager = Manager::new().await.map_err(|e| e.to_string())?;
    
    let adapters = manager.adapters().await.map_err(|e| e.to_string())?;
    if adapters.is_empty() {
        return Err("No Bluetooth adapters found".to_string());
    }
    
    let central = &adapters[0];
    central.start_scan(ScanFilter::default()).await.map_err(|e| e.to_string())?;
    
    // Wait for 3 seconds to discover devices
    time::sleep(Duration::from_secs(3)).await;
    
    let mut devices = Vec::new();
    for p in central.peripherals().await.map_err(|e| e.to_string())? {
        let properties = p.properties().await.map_err(|e| e.to_string())?;
        if let Some(props) = properties {
            let name = props.local_name.unwrap_or_else(|| "Unknown Device".to_string());
            let is_connected = p.is_connected().await.unwrap_or(false);
            
            devices.push(BluetoothDevice {
                id: p.id().to_string(),
                name,
                is_connected,
            });
        }
    }
    
    let _ = central.stop_scan().await;
    
    // Sort so connected and named devices are first
    devices.sort_by(|a, b| {
        b.is_connected.cmp(&a.is_connected).then(
            (b.name != "Unknown Device").cmp(&(a.name != "Unknown Device"))
        ).then(a.name.cmp(&b.name))
    });
    
    Ok(devices)
}

#[tauri::command]
async fn connect_bluetooth(id: String) -> Result<bool, String> {
    let manager = Manager::new().await.map_err(|e| e.to_string())?;
    let adapters = manager.adapters().await.map_err(|e| e.to_string())?;
    if adapters.is_empty() {
        return Err("No Bluetooth adapters found".to_string());
    }
    
    let central = &adapters[0];
    for p in central.peripherals().await.map_err(|e| e.to_string())? {
        if p.id().to_string() == id {
            p.connect().await.map_err(|e| e.to_string())?;
            return Ok(true);
        }
    }
    
    Err("Device not found".to_string())
}

#[tauri::command]
fn get_default_paths() -> Result<std::collections::HashMap<String, String>, String> {
    let mut paths = std::collections::HashMap::new();
    #[cfg(windows)]
    {
        if let Ok(user_profile) = std::env::var("USERPROFILE") {
            paths.insert("desktop".to_string(), format!("{}\\Desktop", user_profile));
            paths.insert("downloads".to_string(), format!("{}\\Downloads", user_profile));
            paths.insert("documents".to_string(), format!("{}\\Documents", user_profile));
            paths.insert("pictures".to_string(), format!("{}\\Pictures", user_profile));
            paths.insert("music".to_string(), format!("{}\\Music", user_profile));
            paths.insert("videos".to_string(), format!("{}\\Videos", user_profile));
        }
    }
    Ok(paths)
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_shell::init())
        .plugin(tauri_plugin_store::Builder::new().build())
        .plugin(tauri_plugin_opener::init())
        .plugin(tauri_plugin_dialog::init())
        .invoke_handler(tauri::generate_handler![
            greet, 
            get_drives, 
            read_dir, 
            read_file_bytes,
            create_desktop_shortcut,
            get_wifi_networks,
            connect_wifi,
            get_bluetooth_devices,
            connect_bluetooth,
            get_default_paths
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
