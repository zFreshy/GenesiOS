#!/bin/bash
# Script para rodar Genesi Desktop GTK4 no GNOME Terminal
# IMPORTANTE: Execute este script NO TERMINAL GRÁFICO do GNOME, não via SSH!

set -e

cd genesi-desktop/genesi-desktop-gtk

echo "🚀 Genesi Desktop GTK4 - Modo GNOME"
echo ""
echo "⚠️  IMPORTANTE:"
echo "   Este script deve ser executado NO TERMINAL GRÁFICO"
echo "   Se estiver via SSH, não vai funcionar!"
echo ""

# Detecta se está em sessão gráfica
if [ -z "$WAYLAND_DISPLAY" ] && [ -z "$DISPLAY" ]; then
    echo "❌ Nenhum display detectado!"
    echo ""
    echo "Você está via SSH? Tente:"
    echo "  1. Abra o terminal DENTRO da VM (não SSH)"
    echo "  2. Ou use VNC/RDP para acessar a interface gráfica"
    echo "  3. Ou pule o teste local e vá direto para ISO"
    exit 1
fi

# Tenta detectar o display correto
if [ -n "$WAYLAND_DISPLAY" ]; then
    echo "✅ Wayland detectado: $WAYLAND_DISPLAY"
    export GDK_BACKEND=wayland
elif [ -n "$DISPLAY" ]; then
    echo "✅ X11 detectado: $DISPLAY"
    export GDK_BACKEND=x11
fi

# Configurações
export RUST_BACKTRACE=1
export GTK_USE_PORTAL=0
export GTK_A11Y=none
export NO_AT_BRIDGE=1

echo ""
echo "🎨 Iniciando Genesi Desktop..."
echo "   (Pressione Ctrl+C para fechar)"
echo ""

# Roda
exec ./target/release/genesi-desktop-gtk
