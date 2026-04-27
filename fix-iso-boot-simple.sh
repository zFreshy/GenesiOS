#!/bin/bash
# Script simplificado para corrigir boot da ISO
# Uso: sudo ./fix-iso-boot-simple.sh

set -e

echo "🔧 Genesi OS - Fix ISO Boot (Simplified)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Verifica se está rodando como root
if [ "$EUID" -ne 0 ]; then 
    echo "❌ Este script precisa ser executado como root"
    echo "   Execute: sudo ./fix-iso-boot-simple.sh"
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
echo "  Recriando estrutura de boot"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Remove ISO antiga se existir
rm -f filesystem.squashfs
rm -rf iso

# Cria squashfs
echo "📦 Criando filesystem.squashfs..."
mksquashfs chroot filesystem.squashfs -comp xz -b 1M -noappend

# Estrutura ISO
mkdir -p iso/boot/grub iso/live

# Copia kernel e initrd
echo "📦 Copiando kernel e initrd..."
cp chroot/boot/vmlinuz-* iso/boot/vmlinuz
cp chroot/boot/initrd.img-* iso/boot/initrd.img
cp filesystem.squashfs iso/live/filesystem.squashfs

# GRUB config SIMPLIFICADO (só BIOS)
cat > iso/boot/grub/grub.cfg << 'GRUBEOF'
set timeout=10
set default=0

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

# Gera ISO com grub-mkrescue (só BIOS, mais simples)
echo "💿 Gerando ISO bootável..."

grub-mkrescue \
    --output="$ISO_NAME" \
    iso/ \
    -- \
    -volid "GENESI_OS" \
    -joliet \
    -rational-rock

if [ ! -f "$ISO_NAME" ]; then
    echo "❌ Erro ao gerar ISO!"
    exit 1
fi

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
echo "   ✅ GRUB configurado corretamente"
echo "   ✅ Modo Safe Mode com nomodeset"
echo "   ✅ Modo Text Mode para debug"
echo "   ✅ Timeout aumentado para 10 segundos"
echo ""
echo "🖥️  Configurações da VM:"
echo "   - Paravirtualization: Default ou None"
echo "   - 3D Acceleration: Desabilitado"
echo "   - Video Memory: 128 MB"
echo "   - EFI: Desabilitado"
echo ""
echo "💡 Se não bootar, tente 'Safe Mode' no menu GRUB"
echo ""
