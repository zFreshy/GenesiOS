#!/bin/bash
# Setup para Ubuntu Desktop (VM)

set -e

echo "🔧 Configurando Genesi OS no Ubuntu Desktop"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Atualiza sistema
echo "📦 Atualizando sistema..."
sudo apt update

# Instala dependências básicas
echo "📦 Instalando dependências..."
sudo apt install -y \
    curl \
    wget \
    git \
    build-essential \
    pkg-config \
    libssl-dev \
    libudev-dev \
    libdbus-1-dev \
    libseat-dev

# Instala dependências gráficas (sem libgl1-mesa-glx obsoleto)
echo "🎨 Instalando dependências gráficas..."
sudo apt install -y \
    libwebkit2gtk-4.1-dev \
    libgtk-3-dev \
    libayatana-appindicator3-dev \
    librsvg2-dev \
    libjavascriptcoregtk-4.1-dev \
    libsoup-3.0-dev \
    patchelf \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    mesa-utils

# Instala Rust
echo "🦀 Instalando Rust..."
if ! command -v cargo &> /dev/null; then
    curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
    source "$HOME/.cargo/env"
    echo 'source "$HOME/.cargo/env"' >> ~/.bashrc
else
    echo "✅ Rust já instalado"
fi

# Instala Node.js 20
echo "📦 Instalando Node.js 20..."
if ! command -v node &> /dev/null; then
    curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash -
    sudo apt install -y nodejs
else
    echo "✅ Node.js já instalado"
fi

# Verifica instalações
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  ✅ Verificando instalações"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

source "$HOME/.cargo/env"

echo "🦀 Rust: $(rustc --version)"
echo "📦 Cargo: $(cargo --version)"
echo "📦 Node.js: $(node --version)"
echo "📦 npm: $(npm --version)"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  🎉 Setup concluído!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Agora você pode rodar:"
echo "  ./run-genesi.sh"
echo ""
