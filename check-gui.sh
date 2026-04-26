#!/bin/bash
# Script para verificar se o sistema tem interface gráfica

echo "🔍 Verificando ambiente gráfico..."
echo ""

# Verifica se está rodando interface gráfica
if [ -n "$DISPLAY" ]; then
    echo "✅ DISPLAY detectado: $DISPLAY"
    echo "   Você está em um ambiente gráfico!"
else
    echo "❌ DISPLAY não detectado"
    echo "   Você pode estar em:"
    echo "   - Ubuntu Server (sem GUI)"
    echo "   - SSH sem X11 forwarding"
fi

echo ""

# Verifica qual desktop environment está instalado
echo "🖥️  Desktop Environments instalados:"
echo ""

if command -v gnome-shell &> /dev/null; then
    echo "✅ GNOME detectado"
    gnome-shell --version 2>/dev/null || echo "   (versão não disponível)"
fi

if command -v plasmashell &> /dev/null; then
    echo "✅ KDE Plasma detectado"
    plasmashell --version 2>/dev/null || echo "   (versão não disponível)"
fi

if command -v xfce4-session &> /dev/null; then
    echo "✅ XFCE detectado"
    xfce4-session --version 2>/dev/null || echo "   (versão não disponível)"
fi

if command -v mate-session &> /dev/null; then
    echo "✅ MATE detectado"
fi

if command -v cinnamon &> /dev/null; then
    echo "✅ Cinnamon detectado"
fi

# Verifica se nenhum foi encontrado
if ! command -v gnome-shell &> /dev/null && \
   ! command -v plasmashell &> /dev/null && \
   ! command -v xfce4-session &> /dev/null && \
   ! command -v mate-session &> /dev/null && \
   ! command -v cinnamon &> /dev/null; then
    echo "❌ Nenhum desktop environment detectado"
    echo ""
    echo "📦 Para instalar interface gráfica:"
    echo ""
    echo "   GNOME (mais popular):"
    echo "   sudo apt install ubuntu-desktop"
    echo ""
    echo "   XFCE (mais leve):"
    echo "   sudo apt install xubuntu-desktop"
    echo ""
    echo "   KDE Plasma:"
    echo "   sudo apt install kubuntu-desktop"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Verifica se GTK4 está instalado
echo "📦 Verificando GTK4..."
if dpkg -l | grep -q libgtk-4-1; then
    echo "✅ GTK4 instalado"
else
    echo "❌ GTK4 não instalado"
    echo "   Instale com: sudo apt install libgtk-4-1 libgtk-4-dev"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Conclusão
if [ -n "$DISPLAY" ]; then
    echo "✅ PODE TESTAR LOCALMENTE!"
    echo "   Execute: ./test-gtk-local.sh"
else
    echo "⚠️  NÃO PODE TESTAR LOCALMENTE"
    echo "   Opções:"
    echo "   1. Instale interface gráfica (ubuntu-desktop)"
    echo "   2. Ou crie a ISO direto (./rebuild-iso.sh)"
fi

echo ""
