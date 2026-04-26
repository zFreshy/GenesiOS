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

# Detecta qual display usar
if [ -z "$DISPLAY" ]; then
    echo "⚠️  DISPLAY não configurado, tentando :0"
    export DISPLAY=:0
fi

# Configura variáveis de ambiente para GTK4
export GDK_BACKEND=x11  # Força X11 (mais compatível que Wayland para teste)
export RUST_BACKTRACE=1
export GTK_A11Y=none  # Desabilita acessibilidade (evita warnings)

# Desabilita portal do Flatpak (não necessário para teste local)
export GTK_USE_PORTAL=0

echo "🔧 Configurações:"
echo "   DISPLAY=$DISPLAY"
echo "   GDK_BACKEND=$GDK_BACKEND"
echo ""

# Tenta rodar o desktop
if ! ./target/release/genesi-desktop-gtk 2>&1; then
    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "❌ Erro ao iniciar!"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "Possíveis soluções:"
    echo ""
    echo "1. Se estiver via SSH:"
    echo "   export DISPLAY=:0"
    echo "   xhost +local:"
    echo "   ./test-gtk-local.sh"
    echo ""
    echo "2. Se estiver no terminal do GNOME:"
    echo "   Abra o terminal gráfico (não SSH)"
    echo "   ./test-gtk-local.sh"
    echo ""
    echo "3. Verificar se X11 está rodando:"
    echo "   echo \$DISPLAY"
    echo "   ps aux | grep X"
    echo ""
fi
