# Genesi OS

Sistema operacional baseado em Linux com interface desktop moderna usando Wayland.

## 🚀 Como Rodar

### Windows (Recomendado)

**Opção 1: PowerShell Híbrido (Recomendado)**
```powershell
.\run-genesi-hybrid.ps1
```
Este script roda o WM no WSL e o Desktop no Windows (evita problemas de GTK).

**Opção 2: PowerShell Puro**
```powershell
.\run-genesi.ps1
```

**Opção 3: Batch/CMD**
```cmd
run-genesi.bat
```

**Opção 4: Apenas Desktop (sem WM)**
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

**Para parar:**
```bash
# Pressione Ctrl+C no terminal onde está rodando

# Ou em outro terminal:
bash stop-genesi.sh

# Ou comando rápido:
./cleanup
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

**Opção 1: Script de Parada (Recomendado)**
```powershell
# PowerShell
.\stop-genesi.ps1

# Ou Batch
stop-genesi.bat
```

**Opção 2: Fechar a janela**
- Clique no X da janela do Genesi OS

**Opção 3: Parar o WSL (mata o vmmemWSL)**
```powershell
wsl --shutdown
```

**Opção 4: Task Manager**
- Ctrl+Shift+Esc → Procure "genesi-desktop" → End Task
- **Nota**: O vmmemWSL (Window Manager no WSL) não pode ser fechado pelo Task Manager
  - Use `wsl --shutdown` para parar ele

**Opção 5: PowerShell Manual**
```powershell
# Para o Desktop
Get-Process | Where-Object {$_.ProcessName -like "*genesi*"} | Stop-Process -Force

# Para o WSL (Window Manager)
wsl --shutdown
```

### Linux/macOS
- **Ctrl+C no terminal**: Pressione Ctrl+C (agora funciona corretamente!)
- **Script dedicado**: `bash stop-genesi.sh`
- **Comando rápido**: `./cleanup`
- **Manual**: `pkill -9 genesi-wm genesi-desktop`

## 📋 Requisitos

### Windows
- Node.js e npm
- Rust (cargo) - Opcional, só se quiser rodar o WM
- WSL2 com Ubuntu - Opcional, só para o Window Manager
- WSLg habilitado - Se for rodar tudo no WSL

### Linux
- Rust (cargo)
- Node.js e npm
- GCC (build-essential)
- Dependências do Tauri: `sudo apt install libwebkit2gtk-4.1-dev libappindicator3-dev librsvg2-dev patchelf`

## 🐛 Problemas Conhecidos

### Erro "Failed to initialize GTK" no WSL

**Problema:** Ao rodar `bash run-genesi.sh` no WSL, aparece erro de GTK.

**Causa:** O Tauri precisa de interface gráfica, e o WSL sem WSLg não tem.

**Soluções:**
1. **Use o script híbrido** (Recomendado):
   ```powershell
   .\run-genesi-hybrid.ps1
   ```

2. **Rode direto no Windows**:
   ```powershell
   cd genesi-desktop
   npm run tauri dev
   ```

3. **Habilite WSLg no WSL**:
   ```powershell
   # No PowerShell como Admin
   wsl --update
   wsl --shutdown
   
   # No WSL, verifique:
   echo $DISPLAY  # Deve mostrar :0 ou :1
   ```

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
