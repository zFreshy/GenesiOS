# Genesi OS

Sistema operacional baseado em Linux com interface desktop moderna usando Wayland.

## 🚀 Como Rodar

### WSL (Ubuntu no Windows)

**IMPORTANTE**: Se você está no WSL, primeiro execute o setup:

```bash
# No terminal WSL (Ubuntu)
cd /mnt/d/Desenvolvimento/Genesi  # Ajuste o caminho conforme necessário
bash setup-wsl.sh
```

Depois de instalar as dependências, você tem 3 opções para rodar:

#### Opção 1: Script Automático (Recomendado)
```bash
bash start.sh  # Limpa e inicia automaticamente
```

#### Opção 2: Script Manual (2 passos)
```bash
bash stop-genesi.sh  # Limpa processos antigos
bash run-genesi.sh   # Inicia o sistema
```

#### Opção 3: Manual Completo (2 terminais - Mais controle)

**Terminal 1 - Window Manager:**
```bash
cd genesi-desktop/genesi-wm
cargo build --release
cargo run --release
```

**Terminal 2 - Desktop Environment:**
```bash
cd genesi-desktop
npm run tauri dev
```

**Vantagens da Opção 3:**
- ✅ Mais controle sobre cada componente
- ✅ Logs separados (fácil de debugar)
- ✅ Pode reiniciar um componente sem afetar o outro
- ✅ Melhor para desenvolvimento

**Para parar:**
```bash
# Pressione Ctrl+C em cada terminal

# Ou em outro terminal:
bash stop-genesi.sh

# Ou comando rápido:
./cleanup
```

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

**Solução:** Execute o script de setup primeiro:

```bash
# No terminal WSL
bash setup-wsl.sh
```

Este script irá:
- Instalar todas as dependências do Tauri (WebKit2GTK, GTK3, etc.)
- Instalar Node.js e npm
- Instalar Rust e Cargo
- Configurar o DISPLAY para WSLg
- Testar se a interface gráfica está funcionando

Depois do setup, rode:
```bash
bash run-genesi.sh
```

**Se ainda der erro:**
1. Atualize o WSL no PowerShell (como Admin):
   ```powershell
   wsl --update
   wsl --shutdown
   ```

2. Verifique se o DISPLAY está configurado:
   ```bash
   echo $DISPLAY  # Deve mostrar :0 ou :1
   ```

3. Teste se o X11 funciona:
   ```bash
   xeyes  # Deve abrir uma janela com olhos
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
