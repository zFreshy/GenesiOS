#!/bin/bash
# Script para rodar Genesi Desktop GTK4 forçando X11
# Use este se test-gtk-local.sh não funcionar

set -e

cd genesi-desktop/genesi-desktop-gtk

echo "🚀 Rodando Genesi Desktop GTK4 (X11 forçado)"
echo ""

# Detecta DISPLAY
if [ -z "$DISPLAY" ]; then
    echo "⚠️  DISPLAY não detectado, usando :0"
    export DISPLAY=:0
fi

# Permite conexões locais ao X11
xhost +local: 2>/dev/null || echo "⚠️  xhost não disponível (ignorando)"

# Configurações para GTK4 funcionar em X11
export GDK_BACKEND=x11
export RUST_BACKTRACE=1
export GTK_USE_PORTAL=0
export GTK_A11Y=none
export NO_AT_BRIDGE=1

echo "🔧 Configurações:"
echo "   DISPLAY: $DISPLAY"
echo "   GDK_BACKEND: $GDK_BACKEND"
echo ""
echo "🎨 Iniciando interface..."
echo ""

# Roda
./target/release/genesi-desktop-gtk
