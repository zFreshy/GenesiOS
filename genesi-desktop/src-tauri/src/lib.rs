use std::fs;
use std::path::Path;
use std::time::UNIX_EPOCH;
use sysinfo::Disks;

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

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![greet, get_drives, read_dir, read_file_bytes])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
