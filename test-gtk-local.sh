#!/bin/bash
# Script para testar Genesi Desktop GTK4 localmente (sem ISO)
# Uso: ./test-gtk-local.sh

set -e

echo "🔥 Genesi OS - Teste Local GTK4"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Verifica se está no diretório correto
if [ ! -d "genesi-desktop/genesi-desktop-gtk" ]; then
    echo "❌ Execute este script na raiz do projeto GenesiOS"
    exit 1
fi

cd genesi-desktop/genesi-desktop-gtk

echo "📦 Instalando dependências GTK4 (se necessário)..."
sudo apt update
sudo apt install -y libgtk-4-1 libgtk-4-dev pkg-config build-essential

echo ""
echo "🔨 Compilando Genesi Desktop GTK4..."
cargo build --release

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  ✅ Compilação concluída!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "🚀 Iniciando Genesi Desktop..."
echo ""
echo "IMPORTANTE:"
echo "  - Você precisa estar em um ambiente gráfico (GNOME/KDE/XFCE)"
echo "  - Se estiver via SSH, use: export DISPLAY=:0"
echo "  - Pressione Ctrl+C para fechar"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Configura variáveis de ambiente
export GDK_BACKEND=x11  # Usa X11 para teste local (mais compatível)
export RUST_BACKTRACE=1

# Roda o desktop
./target/release/genesi-desktop-gtk
