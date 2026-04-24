# Genesi OS

Sistema operacional baseado em Linux com interface desktop moderna usando Wayland.

## 🚀 Como Rodar

### Windows

**Opção 1: PowerShell (Recomendado)**
```powershell
.\run-genesi.ps1
```

**Opção 2: Batch/CMD**
```cmd
run-genesi.bat
```

**Opção 3: Manual**
```powershell
cd genesi-desktop
npm install  # primeira vez apenas
npm run tauri dev
```

### Linux/macOS

**Opção 1: Script Automático**
```bash
bash run-genesi.sh
```

**Opção 2: Manual (Dois Terminais)**

**Terminal 1 - Window Manager:**
```bash
cd genesi-desktop/genesi-wm
cargo build --release
cargo run --release
```

**Terminal 2 - Desktop Environment:**
```bash
cd genesi-desktop
npm install  # primeira vez apenas
npm run tauri dev
```

## 🛑 Como Parar

### Windows
- **Fechar a janela**: Clique no X da janela do Genesi OS
- **Ctrl+C no terminal**: Pressione Ctrl+C (pode precisar pressionar 2x)
- **Task Manager**: Ctrl+Shift+Esc → Procure "genesi-desktop" → End Task
- **PowerShell**: `Get-Process | Where-Object {$_.ProcessName -like "*genesi*"} | Stop-Process -Force`

### Linux/macOS
- **Ctrl+C no terminal**: Pressione Ctrl+C
- **Matar processo**: `pkill -9 genesi-desktop`

## 📋 Requisitos

- Rust (cargo)
- Node.js e npm
- GCC (build-essential)
- Dependências do Tauri: `sudo apt install libwebkit2gtk-4.1-dev libappindicator3-dev librsvg2-dev patchelf`

## 🐛 Problemas Conhecidos

### Firefox com Duas Topbars

Se o Firefox aparecer com duas topbars, veja a documentação completa em:
- `genesi-desktop/FIREFOX-SSD-FIX.md`

Ou rode o script de teste:
```bash
cd genesi-desktop
bash test-firefox-ssd.sh
```

## 📁 Estrutura

```
genesi-os/
├── genesi-desktop/          # Desktop Environment (Tauri + React)
│   ├── src/                 # Frontend React
│   ├── src-tauri/           # Backend Rust
│   └── genesi-wm/           # Window Manager (Wayland compositor)
├── kernel_legacy/           # Kernel antigo (não usado atualmente)
└── run-genesi.sh           # Script para rodar tudo
```

## 🎯 Navegadores

- **Chromium** (Recomendado): Suporte nativo a Server-Side Decoration
- **Firefox**: Requer correções de CSD (aplicadas automaticamente)

## 📝 Desenvolvimento

Para desenvolvimento ativo:

```bash
# Terminal 1: WM com hot reload
cd genesi-desktop/genesi-wm
cargo watch -x run

# Terminal 2: Desktop com hot reload
cd genesi-desktop
npm run tauri dev
```
