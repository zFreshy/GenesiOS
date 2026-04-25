#!/bin/bash
# Script SIMPLIFICADO para criar ISO bootável do Genesi OS
# Usa abordagem mais direta sem Live CD

set -e

echo "🔥 Genesi OS - Build ISO Simplificado"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

if [ "$EUID" -ne 0 ]; then 
    echo "❌ Execute como root: sudo ./build-iso-simple.sh"
    exit 1
fi

WORK_DIR="$HOME/genesi-iso-simple"
GENESI_SOURCE="$(pwd)"
ISO_NAME="GenesiOS-Simple-$(date +%Y%m%d).iso"

echo "📁 Diretório: $WORK_DIR"
echo "💿 ISO: $ISO_NAME"
echo ""

# Limpa
rm -rf "$WORK_DIR"
mkdir -p "$WORK_DIR/iso/boot/grub"
cd "$WORK_DIR"

echo "📦 Copiando binários compilados..."

# Copia os binários já compilados do chroot anterior
CHROOT_PATH="$HOME/genesi-iso-build/chroot"

if [ ! -d "$CHROOT_PATH" ]; then
    echo "❌ Chroot não encontrado. Execute build-iso.sh primeiro!"
    exit 1
fi

# Cria estrutura mínima
mkdir -p iso/genesi/{wm,desktop,lib}

# Copia binários
cp "$CHROOT_PATH/home/genesi/GenesiOS/genesi-desktop/genesi-wm/target/release/genesi-wm" iso/genesi/wm/
cp "$CHROOT_PATH/home/genesi/GenesiOS/genesi-desktop/src-tauri/target/release/genesi-desktop" iso/genesi/desktop/

# Copia kernel e initrd
cp "$CHROOT_PATH/boot/vmlinuz-"* iso/boot/vmlinuz
cp "$CHROOT_PATH/boot/initrd.img-"* iso/boot/initrd

# Cria script de init personalizado
cat > iso/init << 'EOFINIT'
#!/bin/sh
# Init script minimalista para Genesi OS

mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t devtmpfs devtmpfs /dev

# Inicia Genesi WM
/genesi/wm/genesi-wm &

# Aguarda
sleep 2

# Inicia Genesi Desktop
cd /genesi/desktop
./genesi-desktop

# Shell de emergência se falhar
exec /bin/sh
EOFINIT

chmod +x iso/init

# GRUB config CORRETO
cat > iso/boot/grub/grub.cfg << 'EOF'
set timeout=5
set default=0

menuentry "Genesi OS" {
    linux /boot/vmlinuz init=/init quiet
    initrd /boot/initrd
}

menuentry "Genesi OS (Debug)" {
    linux /boot/vmlinuz init=/init debug
    initrd /boot/initrd
}

menuentry "Genesi OS (Shell)" {
    linux /boot/vmlinuz init=/bin/sh
    initrd /boot/initrd
}
EOF

echo "💿 Gerando ISO..."
grub-mkrescue -o "$ISO_NAME" iso/

mv "$ISO_NAME" "$GENESI_SOURCE/"

echo ""
echo "✅ ISO Simplificada criada!"
echo "📍 $GENESI_SOURCE/$ISO_NAME"
echo "📊 $(du -h "$GENESI_SOURCE/$ISO_NAME" | cut -f1)"
echo ""
