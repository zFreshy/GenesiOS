#!/bin/bash
# Script para configurar o WSL para rodar o Genesi OS
# Uso: bash setup-wsl.sh

set -e

echo "🔧 Configurando WSL para Genesi OS..."
echo ""

# Verifica se está no WSL
if ! grep -qi microsoft /proc/version 2>/dev/null; then
    echo "⚠ Este script é para WSL (Windows Subsystem for Linux)"
    echo "   Se você está no Linux nativo, as dependências podem ser diferentes"
    read -p "Continuar mesmo assim? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo "📦 Atualizando lista de pacotes..."
sudo apt update

# Instala dependências do Tauri
echo "📦 Instalando dependências do Tauri..."
sudo apt install -y \
    libwebkit2gtk-4.1-dev \
    build-essential \
    curl \
    wget \
    file \
    libssl-dev \
    libgtk-3-dev \
    libayatana-appindicator3-dev \
    librsvg2-dev \
    libjavascriptcoregtk-4.1-dev \
    libsoup-3.0-dev \
    patchelf \
    pkg-config \
    libudev-dev \
    libdbus-1-dev

# Instala dependências do WSLg (interface gráfica)
echo "📦 Instalando dependências gráficas..."
sudo apt install -y \
    x11-apps \
    mesa-utils \
    libgl1-mesa-glx

# Instala Node.js se não tiver
if ! command -v node &> /dev/null; then
    echo "📦 Instalando Node.js 20..."
    curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
    sudo apt install -y nodejs
    echo "✓ Node.js $(node --version) instalado"
else
    echo "✓ Node.js $(node --version) já instalado"
fi

# Instala Rust se não tiver
if ! command -v cargo &> /dev/null; then
    echo "📦 Instalando Rust..."
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
    source "$HOME/.cargo/env"
    echo "✓ Rust $(rustc --version) instalado"
else
    source "$HOME/.cargo/env" 2>/dev/null || true
    echo "✓ Rust $(rustc --version) já instalado"
fi

# Configura DISPLAY
echo ""
echo "🔧 Configurando DISPLAY..."
if ! grep -q "export DISPLAY=:0" ~/.bashrc; then
    echo 'export DISPLAY=:0' >> ~/.bashrc
    echo "✓ DISPLAY adicionado ao ~/.bashrc"
fi

export DISPLAY=:0

# Testa se o display funciona
echo ""
echo "🧪 Testando display gráfico..."
if command -v xeyes &> /dev/null; then
    timeout 2 xeyes 2>/dev/null && echo "✓ Display funcionando!" || echo "⚠ Display pode não estar funcionando"
fi

echo ""
echo "✅ Setup concluído!"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  📝 Próximos passos:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "  1. Feche e reabra o terminal WSL (para carregar o Rust)"
echo "  2. Execute: bash run-genesi.sh"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "💡 Dica: Se der erro de GTK, tente:"
echo "   - No PowerShell (Admin): wsl --update"
echo "   - Depois: wsl --shutdown"
echo "   - Reabra o WSL e tente novamente"
echo ""
