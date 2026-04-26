#!/bin/bash
# Script de build do Genesi Desktop GTK4

set -e

echo "🔥 Genesi OS Desktop - Build Script"
echo "===================================="
echo ""

# Verifica dependências
echo "📦 Verificando dependências..."
if ! command -v cargo &> /dev/null; then
    echo "❌ Rust não instalado!"
    echo "   Instale: curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh"
    exit 1
fi

if ! pkg-config --exists gtk4; then
    echo "❌ GTK4 não instalado!"
    echo "   Instale: sudo apt install libgtk-4-dev"
    exit 1
fi

echo "✅ Dependências OK"
echo ""

# Build
echo "🔨 Compilando..."
cargo build --release

echo ""
echo "✅ Build concluído!"
echo ""
echo "📍 Binário: target/release/genesi-desktop-gtk"
echo ""
echo "🚀 Para rodar:"
echo "   ./target/release/genesi-desktop-gtk"
echo ""
