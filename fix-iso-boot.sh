#!/bin/bash
# Script para corrigir problemas de boot na ISO
# Uso: sudo ./fix-iso-boot.sh

set -e

echo "🔧 Genesi OS - Fix ISO Boot"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Verifica se está rodando como root
if [ "$EUID" -ne 0 ]; then 
    echo "❌ Este script precisa ser executado como root"
    echo "   Execute: sudo ./fix-iso-boot.sh"
    exit 1
fi

WORK_DIR="$HOME/genesi-iso-build"
GENESI_SOURCE="$(pwd)"
ISO_NAME="GenesiOS-$(date +%Y%m%d-%H%M)-fixed.iso"

# Verifica se existe build anterior
if [ ! -d "$WORK_DIR/chroot" ]; then
    echo "❌ Não encontrei build anterior em $WORK_DIR"
    echo "   Execute primeiro: sudo ./build-iso.sh"
    exit 1
fi

echo "📁 Diretório de trabalho: $WORK_DIR"
echo "💿 ISO corrigida: $ISO_NAME"
echo ""

cd "$WORK_DIR"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Corrigindo estrutura de boot"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Remove ISO antiga se existir
rm -f filesystem.squashfs
rm -rf iso

# Cria squashfs
echo "📦 Criando filesystem.squashfs..."
mksquashfs chroot filesystem.squashfs -comp xz -b 1M -noappend

# Estrutura ISO corrigida
mkdir -p iso/boot/grub iso/live iso/isolinux

# Copia kernel e initrd
echo "📦 Copiando kernel e initrd..."
cp chroot/boot/vmlinuz-* iso/boot/vmlinuz
cp chroot/boot/initrd.img-* iso/boot/initrd.img
cp filesystem.squashfs iso/live/filesystem.squashfs

# Copia arquivos do GRUB
echo "📦 Configurando GRUB..."
cp -r /usr/lib/grub/i386-pc iso/boot/grub/ 2>/dev/null || true

# GRUB config CORRIGIDO
cat > iso/boot/grub/grub.cfg << 'GRUBEOF'
set timeout=10
set default=0

insmod all_video
insmod gfxterm
insmod png

set gfxmode=auto
set gfxpayload=keep

terminal_output gfxterm

menuentry "Genesi OS" {
    linux /boot/vmlinuz boot=live components quiet splash username=genesi hostname=genesi-os
    initrd /boot/initrd.img
}

menuentry "Genesi OS (Safe Mode - nomodeset)" {
    linux /boot/vmlinuz boot=live components nomodeset username=genesi hostname=genesi-os
    initrd /boot/initrd.img
}

menuentry "Genesi OS (Debug Mode)" {
    linux /boot/vmlinuz boot=live components debug verbose username=genesi hostname=genesi-os
    initrd /boot/initrd.img
}

menuentry "Genesi OS (Failsafe)" {
    linux /boot/vmlinuz boot=live components nomodeset noapic noacpi nosplash username=genesi hostname=genesi-os
    initrd /boot/initrd.img
}

menuentry "Genesi OS (Text Mode)" {
    linux /boot/vmlinuz boot=live components nomodeset noapic noacpi nosplash systemd.unit=multi-user.target username=genesi hostname=genesi-os
    initrd /boot/initrd.img
}
GRUBEOF

# Cria grub.cfg para EFI também
mkdir -p iso/EFI/BOOT
cp iso/boot/grub/grub.cfg iso/EFI/BOOT/grub.cfg

# Gera ISO com suporte BIOS e EFI
echo "💿 Gerando ISO bootável (BIOS + EFI)..."

grub-mkrescue \
    --output="$ISO_NAME" \
    --modules="linux normal iso9660 biosdisk memdisk search tar ls all_video gfxterm gfxmenu efi_gop efi_uga boot chain configfile echo reboot" \
    iso/ \
    -- \
    -volid "GENESI_OS" \
    -joliet \
    -joliet-long \
    -rational-rock

# Move para diretório original
mv "$ISO_NAME" "$GENESI_SOURCE/"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  ✅ ISO corrigida gerada com sucesso!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📍 Localização: $GENESI_SOURCE/$ISO_NAME"
echo "📊 Tamanho: $(du -h "$GENESI_SOURCE/$ISO_NAME" | cut -f1)"
echo ""
echo "🔧 Correções aplicadas:"
echo "   ✅ GRUB configurado para BIOS + EFI"
echo "   ✅ Modo Safe Mode com nomodeset"
echo "   ✅ Modo Text Mode para debug"
echo "   ✅ Timeout aumentado para 10 segundos"
echo ""
echo "🖥️  Teste na VM com as configurações:"
echo "   - Paravirtualization: Default ou None"
echo "   - 3D Acceleration: Desabilitado"
echo "   - Video Memory: 128 MB"
echo ""
echo "💡 Se não bootar, tente 'Safe Mode' no menu GRUB"
echo ""
