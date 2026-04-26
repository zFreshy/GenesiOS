#!/bin/bash
# Script para instalar bibliotecas GTK4 no chroot existente
# Uso: sudo ./install-gtk4-libs.sh

set -e

echo "🔥 Genesi OS - Instalar GTK4 Runtime"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Verifica se está rodando como root
if [ "$EUID" -ne 0 ]; then 
    echo "❌ Este script precisa ser executado como root"
    echo "   Execute: sudo ./install-gtk4-libs.sh"
    exit 1
fi

WORK_DIR="$HOME/genesi-iso-build"

# Verifica se o chroot existe
if [ ! -d "$WORK_DIR/chroot" ]; then
    echo "❌ Chroot não encontrado em $WORK_DIR/chroot"
    echo "   Execute primeiro: sudo ./build-iso.sh"
    exit 1
fi

echo "📦 Instalando bibliotecas GTK4 no chroot..."
echo ""

# Monta sistemas
mount --bind /dev "$WORK_DIR/chroot/dev" 2>/dev/null || true
mount --bind /dev/pts "$WORK_DIR/chroot/dev/pts" 2>/dev/null || true
mount --bind /proc "$WORK_DIR/chroot/proc" 2>/dev/null || true
mount --bind /sys "$WORK_DIR/chroot/sys" 2>/dev/null || true

# Instala GTK4 runtime
chroot "$WORK_DIR/chroot" /bin/bash << 'EOFCHROOT'
set -e
export DEBIAN_FRONTEND=noninteractive

echo "📦 Atualizando cache de pacotes..."
apt update

echo "📦 Instalando libgtk-4-1 e libgtk-4-dev..."
apt install -y libgtk-4-1 libgtk-4-dev

echo "✅ GTK4 runtime instalado com sucesso!"
EOFCHROOT

# Desmonta
umount "$WORK_DIR/chroot/dev/pts" 2>/dev/null || true
umount "$WORK_DIR/chroot/dev" 2>/dev/null || true
umount "$WORK_DIR/chroot/proc" 2>/dev/null || true
umount "$WORK_DIR/chroot/sys" 2>/dev/null || true

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  ✅ GTK4 runtime instalado!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "🚀 Agora você pode rodar:"
echo "   sudo ./rebuild-iso.sh"
echo ""
