#!/bin/bash
# Script para rebuild rápido da ISO (reutiliza sistema base)
# Uso: sudo ./rebuild-iso.sh

set -e

echo "🔄 Genesi OS - Rebuild ISO (Rápido)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Verifica se está rodando como root
if [ "$EUID" -ne 0 ]; then 
    echo "❌ Este script precisa ser executado como root"
    echo "   Execute: sudo ./rebuild-iso.sh"
    exit 1
fi

# Configurações
WORK_DIR="$HOME/genesi-iso-build"
GENESI_SOURCE="$(pwd)"
ISO_NAME="GenesiOS-$(date +%Y%m%d-%H%M).iso"

# Verifica se existe build anterior
if [ ! -d "$WORK_DIR/chroot" ]; then
    echo "❌ Não encontrei build anterior em $WORK_DIR"
    echo "   Execute primeiro: sudo ./build-iso.sh"
    exit 1
fi

echo "📁 Diretório de trabalho: $WORK_DIR"
echo "📦 Código fonte: $GENESI_SOURCE"
echo "💿 ISO final: $ISO_NAME"
echo ""
echo "✅ Build anterior encontrado! Vou reutilizar o sistema base."
echo ""

# Pergunta confirmação
read -p "Continuar? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

cd "$WORK_DIR"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 1/4: Montando sistemas"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Desmonta se já estiver montado
umount chroot/dev/pts 2>/dev/null || true
umount chroot/dev 2>/dev/null || true
umount chroot/proc 2>/dev/null || true
umount chroot/sys 2>/dev/null || true

# Monta novamente
mount --bind /dev chroot/dev
mount --bind /dev/pts chroot/dev/pts
mount --bind /proc chroot/proc
mount --bind /sys chroot/sys

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 2/4: Atualizando código fonte"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Remove código antigo
echo "🗑️  Removendo código antigo..."
rm -rf chroot/home/genesi/GenesiOS

# Copia código novo
echo "📦 Copiando código atualizado..."
mkdir -p chroot/home/genesi/GenesiOS
cp -r "$GENESI_SOURCE"/* chroot/home/genesi/GenesiOS/
chown -R 1000:1000 chroot/home/genesi/GenesiOS

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 3/4: Recompilando Genesi OS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Recompila
echo "🔨 Recompilando (isso é mais rápido, ~5 minutos)..."
chroot chroot su - genesi -c '
source ~/.cargo/env
cd ~/GenesiOS/genesi-desktop/genesi-wm
echo "  → Recompilando Window Manager..."
cargo build --release 2>&1 | tee /tmp/wm-rebuild.log
cd ~/GenesiOS/genesi-desktop
echo "  → Reinstalando dependências npm (se necessário)..."
npm install 2>&1 | tee /tmp/npm-reinstall.log
echo "  → Rebuildando frontend..."
npm run build 2>&1 | tee /tmp/npm-rebuild.log
cd src-tauri
echo "  → Recompilando Tauri (sem bundling)..."
cargo build --release 2>&1 | tee /tmp/tauri-rebuild.log
'

# Verifica se a compilação foi bem-sucedida
if [ ! -f "chroot/home/genesi/GenesiOS/genesi-desktop/src-tauri/target/release/genesi-desktop" ]; then
    echo "❌ ERRO: Falha na recompilação do Genesi Desktop"
    echo "   Verifique os logs em /tmp/*-rebuild.log"
    exit 1
fi

if [ ! -f "chroot/home/genesi/GenesiOS/genesi-desktop/genesi-wm/target/release/genesi-wm" ]; then
    echo "❌ ERRO: Falha na recompilação do Window Manager"
    echo "   Verifique os logs em /tmp/wm-rebuild.log"
    exit 1
fi

echo "✅ Recompilação concluída com sucesso!"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 4/4: Gerando nova ISO"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Desmonta
umount chroot/dev/pts
umount chroot/dev
umount chroot/proc
umount chroot/sys

# Remove ISO antiga se existir
rm -f filesystem.squashfs
rm -rf iso

# Cria squashfs
echo "📦 Criando filesystem.squashfs..."
mksquashfs chroot filesystem.squashfs -comp xz -b 1M

# Estrutura ISO
mkdir -p iso/boot/grub iso/live
cp chroot/boot/vmlinuz-* iso/boot/vmlinuz
cp chroot/boot/initrd.img-* iso/boot/initrd.img
mv filesystem.squashfs iso/live/

# GRUB config com parâmetros corretos para Live CD
cat > iso/boot/grub/grub.cfg << 'EOF'
set timeout=5
set default=0

menuentry "Genesi OS" {
    linux /boot/vmlinuz boot=live components quiet splash username=genesi hostname=genesi-os
    initrd /boot/initrd.img
}

menuentry "Genesi OS (Safe Mode)" {
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
EOF

# Gera ISO
echo "💿 Gerando ISO..."
grub-mkrescue -o "$ISO_NAME" iso/

# Move para diretório original
mv "$ISO_NAME" "$GENESI_SOURCE/"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  ✅ ISO recriada com sucesso!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📍 Localização: $GENESI_SOURCE/$ISO_NAME"
echo "📊 Tamanho: $(du -h "$GENESI_SOURCE/$ISO_NAME" | cut -f1)"
echo ""
echo "⚡ Rebuild completo em tempo recorde!"
echo ""
