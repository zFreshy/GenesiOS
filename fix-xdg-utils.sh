#!/bin/bash
# Script para instalar xdg-utils no chroot existente
# Uso: sudo ./fix-xdg-utils.sh

set -e

WORK_DIR="$HOME/genesi-iso-build"

if [ "$EUID" -ne 0 ]; then 
    echo "❌ Este script precisa ser executado como root"
    exit 1
fi

if [ ! -d "$WORK_DIR/chroot" ]; then
    echo "❌ Chroot não encontrado em $WORK_DIR/chroot"
    echo "   Execute primeiro: sudo ./build-iso.sh"
    exit 1
fi

echo "🔧 Instalando xdg-utils no chroot..."

# Monta sistemas
mount --bind /dev "$WORK_DIR/chroot/dev" 2>/dev/null || true
mount --bind /dev/pts "$WORK_DIR/chroot/dev/pts" 2>/dev/null || true
mount --bind /proc "$WORK_DIR/chroot/proc" 2>/dev/null || true
mount --bind /sys "$WORK_DIR/chroot/sys" 2>/dev/null || true

# Instala xdg-utils
chroot "$WORK_DIR/chroot" /bin/bash << 'EOFCHROOT'
export DEBIAN_FRONTEND=noninteractive
apt update
apt install -y xdg-utils
EOFCHROOT

# Desmonta
umount "$WORK_DIR/chroot/dev/pts" 2>/dev/null || true
umount "$WORK_DIR/chroot/dev" 2>/dev/null || true
umount "$WORK_DIR/chroot/proc" 2>/dev/null || true
umount "$WORK_DIR/chroot/sys" 2>/dev/null || true

echo "✅ xdg-utils instalado com sucesso!"
echo ""
echo "Agora você pode rodar: sudo ./rebuild-iso.sh"
