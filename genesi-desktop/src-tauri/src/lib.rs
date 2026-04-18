use std::fs;
use std::path::Path;
use std::time::UNIX_EPOCH;
use std::sync::Mutex;
use sysinfo::{Disks, System};
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
fn connect_wifi(ssid: String, _password: Option<String>) -> Result<bool, String> {
    #[cfg(target_os = "windows")]
    {
        // On Windows, you typically need to create a profile XML first, 
        // but for simplicity we will just call netsh connect (which works if profile exists)
        // A full implementation would create the XML and add it using `netsh wlan add profile`
        
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
fn rename_file(old_path: &str, new_path: &str) -> Result<(), String> {
    std::fs::rename(old_path, new_path).map_err(|e| e.to_string())
}

#[derive(serde::Serialize, Clone)]
pub struct ProcessInfo {
    pid: u32,
    name: String,
    memory: u64, // bytes
    cpu: f32, // percentage
    parent_id: Option<u32>,
}

#[derive(serde::Serialize)]
pub struct SystemInfoPayload {
    processes: Vec<ProcessInfo>,
    total_memory: u64,
    used_memory: u64,
    global_cpu: f32,
    genesi_cpu: f32,
    genesi_memory: u64,
}

lazy_static::lazy_static! {
    static ref SYSTEM: Mutex<System> = Mutex::new(System::new_all());
}

#[tauri::command]
fn get_system_processes() -> Result<SystemInfoPayload, String> {
    let mut sys = SYSTEM.lock().map_err(|e| e.to_string())?;
    sys.refresh_all();
    
    let cpu_count = sys.cpus().len() as f32;
    let mut processes = Vec::new();
    
    let current_pid = std::process::id();
    let mut genesi_pids = std::collections::HashSet::new();
    genesi_pids.insert(current_pid);
    
    // Localiza todos os filhos e subprocessos (ex: WebView2) do nosso PID atual
    loop {
        let mut added = false;
        for (pid, process) in sys.processes() {
            if let Some(parent) = process.parent() {
                if genesi_pids.contains(&parent.as_u32()) && !genesi_pids.contains(&pid.as_u32()) {
                    genesi_pids.insert(pid.as_u32());
                    added = true;
                }
            }
        }
        if !added { break; }
    }

    let mut genesi_total_memory = 0;
    let mut genesi_total_cpu = 0.0;
    
    for (pid, process) in sys.processes() {
        let p_cpu = if cpu_count > 0.0 { process.cpu_usage() / cpu_count } else { process.cpu_usage() };
        let p_mem = process.memory();
        
        if genesi_pids.contains(&pid.as_u32()) {
            // Em sistemas Linux, processos filhos do WebKit compartilham o RSS, somar gera GBs falsos.
            // Para ter a memória base do Genesi OS, consideramos o RSS apenas do processo pai (current_pid).
            if pid.as_u32() == current_pid {
                genesi_total_memory = p_mem;
            }
            genesi_total_cpu += p_cpu;
        }

        processes.push(ProcessInfo {
            pid: pid.as_u32(),
            name: process.name().to_string_lossy().to_string(),
            memory: p_mem,
            cpu: p_cpu,
            parent_id: process.parent().map(|p| p.as_u32()),
        });
    }
    
    Ok(SystemInfoPayload {
        processes,
        total_memory: sys.total_memory(),
        used_memory: sys.used_memory(),
        global_cpu: sys.global_cpu_usage(),
        genesi_cpu: genesi_total_cpu,
        genesi_memory: genesi_total_memory,
    })
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
    #[cfg(not(windows))]
    {
        if let Ok(home) = std::env::var("HOME") {
            paths.insert("desktop".to_string(), format!("{}/Desktop", home));
            paths.insert("downloads".to_string(), format!("{}/Downloads", home));
            paths.insert("documents".to_string(), format!("{}/Documents", home));
            paths.insert("pictures".to_string(), format!("{}/Pictures", home));
            paths.insert("music".to_string(), format!("{}/Music", home));
            paths.insert("videos".to_string(), format!("{}/Videos", home));
        }
    }
    Ok(paths)
}

#[tauri::command]
fn launch_browser_wayland() -> Result<(), String> {
    #[cfg(target_os = "linux")]
    {
        let display = std::env::var("WAYLAND_DISPLAY").unwrap_or_else(|_| "wayland-1".to_string());
        
        // Em vez de lutar contra o AppArmor e os perfis do Firefox Snap, vamos apenas abrir uma
        // janela limpa usando o perfil default (--new-window), ou o modo private se preferir.
        // O `--new-instance` tenta forçar o Firefox a não grudar na janela do host Windows.
        let _child = Command::new("firefox")
            .arg("--new-instance")
            .arg("--new-window")
            .env("WAYLAND_DISPLAY", &display)
            .env("MOZ_ENABLE_WAYLAND", "1")
            .spawn()
            .or_else(|_| {
                Command::new("epiphany")
                    .env("WAYLAND_DISPLAY", &display)
                    .spawn()
            })
            .map_err(|e| format!("Falha ao iniciar o Navegador: {}", e))?;

        Ok(())
    }
    
    #[cfg(not(target_os = "linux"))]
    {
        // Tenta ler o socket do Genesi WM de um arquivo compartilhado
        // O WM escreve o socket em /tmp/genesi-wayland-socket.txt via WSL
        let socket_path = "\\\\wsl.localhost\\Ubuntu\\tmp\\genesi-wayland-socket.txt";
        
        // Tenta ler o arquivo do socket via path do WSL
        let wayland_display = std::fs::read_to_string(socket_path)
            .unwrap_or_default()
            .trim()
            .to_string();
        
        if !wayland_display.is_empty() {
            // Executar firefox dentro do WSL com o display correto e sem decorações nativas
            Command::new("wsl")
                .args(&["-e", "bash", "-lc", &format!("WAYLAND_DISPLAY={} MOZ_ENABLE_WAYLAND=1 firefox ", wayland_display)])
                .spawn()
                .map_err(|e| format!("Falha ao iniciar Firefox no WSL: {}", e))?;
            return Ok(());
        }
        
        // Fallback para Windows nativo
        Command::new("cmd")
            .args(&["/c", "start", "msedge"])
            .spawn()
            .map_err(|e| format!("Falha ao iniciar Navegador no Windows: {}", e))?;
        Ok(())
    }
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    // Força o WebView2 (Linux/WebKitGTK) a usar renderização via software no VM
    // E DESTRÓI qualquer menção de X11, forçando ele a conversar 100% via Wayland nativo!
    std::env::set_var("WEBKIT_DISABLE_DMABUF_RENDERER", "1");
    std::env::set_var("WEBKIT_DISABLE_COMPOSITING_MODE", "1");
    std::env::set_var("LIBGL_ALWAYS_SOFTWARE", "1");
    std::env::set_var("GDK_BACKEND", "wayland");
    
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
            get_default_paths,
            rename_file,
            get_system_processes,
            launch_browser_wayland
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
