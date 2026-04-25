#!/bin/bash
# Script para testar Genesi OS localmente (sem criar ISO)

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Teste Local do Genesi OS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Verifica se está no Ubuntu
if ! grep -q "Ubuntu" /etc/os-release 2>/dev/null; then
    echo "❌ Este script deve rodar no Ubuntu (VM), não no WSL!"
    exit 1
fi

# Verifica dependências
echo "📦 Verificando dependências..."
MISSING_DEPS=()

if ! command -v weston &> /dev/null; then
    MISSING_DEPS+=("weston")
fi

if ! command -v cargo &> /dev/null; then
    MISSING_DEPS+=("cargo (Rust)")
fi

if ! command -v npm &> /dev/null; then
    MISSING_DEPS+=("npm (Node.js)")
fi

# Verifica bibliotecas de desenvolvimento
echo "📚 Verificando bibliotecas de desenvolvimento..."
MISSING_LIBS=()

if ! pkg-config --exists wayland-server; then
    MISSING_LIBS+=("libwayland-dev")
fi

if ! pkg-config --exists xkbcommon; then
    MISSING_LIBS+=("libxkbcommon-dev")
fi

if ! pkg-config --exists gbm; then
    MISSING_LIBS+=("libgbm-dev")
fi

if ! pkg-config --exists libinput; then
    MISSING_LIBS+=("libinput-dev")
fi

if ! pkg-config --exists libdrm; then
    MISSING_LIBS+=("libdrm-dev")
fi

if ! pkg-config --exists egl; then
    MISSING_LIBS+=("libegl1-mesa-dev")
fi

if ! pkg-config --exists libudev; then
    MISSING_LIBS+=("libudev-dev")
fi

if ! pkg-config --exists dbus-1; then
    MISSING_LIBS+=("libdbus-1-dev")
fi

if [ ${#MISSING_DEPS[@]} -gt 0 ] || [ ${#MISSING_LIBS[@]} -gt 0 ]; then
    echo "❌ Dependências faltando!"
    echo ""
    if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
        echo "Programas: ${MISSING_DEPS[*]}"
    fi
    if [ ${#MISSING_LIBS[@]} -gt 0 ]; then
        echo "Bibliotecas: ${MISSING_LIBS[*]}"
    fi
    echo ""
    echo "Instale com:"
    echo "  sudo apt update"
    if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
        echo "  sudo apt install -y weston"
    fi
    if [ ${#MISSING_LIBS[@]} -gt 0 ]; then
        echo "  sudo apt install -y ${MISSING_LIBS[*]}"
    fi
    echo ""
    echo "Para instalar tudo de uma vez:"
    echo "  bash setup-ubuntu-desktop.sh"
    exit 1
fi

echo "✅ Todas as dependências instaladas"
echo ""

# Compila o WM
echo "🔨 Compilando Genesi WM..."
cd genesi-desktop/genesi-wm
cargo build --release 2>&1 | tee /tmp/wm-build.log
if [ ${PIPESTATUS[0]} -ne 0 ]; then
    echo "❌ Falha ao compilar o WM"
    echo ""
    echo "Últimas linhas do erro:"
    tail -20 /tmp/wm-build.log
    exit 1
fi
echo "✅ WM compilado"
cd ../..

# Compila o Desktop
echo "🔨 Compilando Genesi Desktop..."
cd genesi-desktop

# Instala dependências npm se necessário
if [ ! -d "node_modules" ]; then
    echo "  → Instalando dependências npm..."
    npm install
fi

# Compila o Tauri
npm run tauri build 2>&1 | tee /tmp/desktop-build.log
if [ ${PIPESTATUS[0]} -ne 0 ]; then
    echo "❌ Falha ao compilar o Desktop"
    echo ""
    echo "Últimas linhas do erro:"
    tail -20 /tmp/desktop-build.log
    exit 1
fi
echo "✅ Desktop compilado"
cd ..

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Compilação Concluída!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Binários gerados:"
echo "  WM:      genesi-desktop/genesi-wm/target/release/genesi-wm"
echo "  Desktop: genesi-desktop/src-tauri/target/release/genesi-desktop"
echo ""
echo "Logs salvos em:"
echo "  WM:      /tmp/wm-build.log"
echo "  Desktop: /tmp/desktop-build.log"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Como Testar"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "OPÇÃO 1: Teste Rápido (com Weston em janela)"
echo "  bash test-genesi-windowed.sh"
echo ""
echo "OPÇÃO 2: Teste Completo (TTY, como na ISO)"
echo "  1. Saia do ambiente gráfico:"
echo "     sudo systemctl stop gdm3  # ou lightdm/sddm"
echo "  2. Vá para TTY1 (Ctrl+Alt+F1)"
echo "  3. Logue como seu usuário"
echo "  4. Execute:"
echo "     cd ~/GenesiOS"
echo "     bash test-genesi-tty.sh"
echo ""
