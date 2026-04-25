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

# Atualiza .bashrc
cat > chroot/home/genesi/.bashrc << 'EOF'
# ~/.bashrc

# If not running interactively, don't do anything
case $- in
    *i*) ;;
      *) return;;
esac

# Inicia Genesi OS automaticamente no login do tty1
if [ "$(tty)" = "/dev/tty1" ]; then
    echo "🔥 Iniciando Genesi OS..."
    echo "   Para ver logs: cat /tmp/genesi-startup.log"
    echo ""
    # Inicia direto (sem startx, Wayland puro)
    exec /usr/local/bin/start-genesi.sh
fi
EOF

chown 1000:1000 chroot/home/genesi/.bashrc
cat > chroot/usr/local/bin/start-genesi.sh << 'EOF'
#!/bin/bash
export XDG_RUNTIME_DIR=/run/user/$(id -u)
export MOZ_ENABLE_WAYLAND=1
export GTK_CSD=0
export LIBDECOR_PLUGIN_DIR=/dev/null

# Cria XDG_RUNTIME_DIR se não existir
mkdir -p "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"

# Log file
LOG_FILE="/tmp/genesi-startup.log"
echo "=== Genesi OS Startup $(date) ===" > "$LOG_FILE"

echo "Starting Sway (Base Wayland Compositor)..." >> "$LOG_FILE"

# Inicia Sway (mais simples que Weston, não precisa de permissões especiais)
sway >> "$LOG_FILE" 2>&1 &
SWAY_PID=$!
echo "Sway PID: $SWAY_PID" >> "$LOG_FILE"

# Aguarda Sway criar o socket
sleep 5

# Verifica se Sway está rodando
if ! kill -0 $SWAY_PID 2>/dev/null; then
    echo "ERROR: Sway crashed!" >> "$LOG_FILE"
    cat "$LOG_FILE"
    echo ""
    echo "Sway failed to start. Check log above."
    exec /bin/bash
    exit 1
fi

# Sway cria wayland-0 ou wayland-1
if [ -S "$XDG_RUNTIME_DIR/wayland-0" ]; then
    export WAYLAND_DISPLAY=wayland-0
elif [ -S "$XDG_RUNTIME_DIR/wayland-1" ]; then
    export WAYLAND_DISPLAY=wayland-1
else
    echo "ERROR: Sway socket not found!" >> "$LOG_FILE"
    cat "$LOG_FILE"
    kill $SWAY_PID 2>/dev/null
    exec /bin/bash
    exit 1
fi

echo "Sway started successfully on $WAYLAND_DISPLAY" >> "$LOG_FILE"

echo "Starting Genesi WM (Window Manager)..." >> "$LOG_FILE"

# Genesi WM roda DENTRO do Weston e cria wayland-1 para os apps
cd /home/genesi/GenesiOS/genesi-desktop/genesi-wm
WAYLAND_DISPLAY=wayland-0 ./target/release/genesi-wm >> "$LOG_FILE" 2>&1 &
WM_PID=$!
echo "WM PID: $WM_PID" >> "$LOG_FILE"

# Aguarda WM criar seu socket
sleep 5

# Verifica se WM está rodando
if ! kill -0 $WM_PID 2>/dev/null; then
    echo "ERROR: WM crashed!" >> "$LOG_FILE"
    cat "$LOG_FILE"
    echo ""
    echo "WM failed to start. Check log above."
    kill $WESTON_PID 2>/dev/null
    exec /bin/bash
    exit 1
fi

# WM cria wayland-1
if [ ! -S "$XDG_RUNTIME_DIR/wayland-1" ]; then
    echo "ERROR: WM socket not found!" >> "$LOG_FILE"
    cat "$LOG_FILE"
    kill $WM_PID 2>/dev/null
    kill $WESTON_PID 2>/dev/null
    exec /bin/bash
    exit 1
fi

echo "WM started successfully on wayland-1" >> "$LOG_FILE"

# Aguarda mais um pouco
sleep 2

# Inicia Desktop conectando ao WM (wayland-1)
echo "Starting Genesi Desktop..." >> "$LOG_FILE"
echo "WAYLAND_DISPLAY=wayland-1" >> "$LOG_FILE"
echo "XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR" >> "$LOG_FILE"

cd /home/genesi/GenesiOS/genesi-desktop/src-tauri/target/release

# Verifica se o binário existe
if [ ! -f "./genesi-desktop" ]; then
    echo "ERROR: genesi-desktop binary not found!" >> "$LOG_FILE"
    cat "$LOG_FILE"
    kill $WM_PID 2>/dev/null
    kill $WESTON_PID 2>/dev/null
    exec /bin/bash
    exit 1
fi

# Roda o desktop conectando ao Genesi WM (wayland-1)
WAYLAND_DISPLAY=wayland-1 \
XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
GDK_BACKEND=wayland \
QT_QPA_PLATFORM=wayland \
SDL_VIDEODRIVER=wayland \
CLUTTER_BACKEND=wayland \
MOZ_ENABLE_WAYLAND=1 \
DISPLAY="" \
./genesi-desktop >> "$LOG_FILE" 2>&1

# Se o desktop fechar, mata tudo
echo "Desktop closed, cleaning up..." >> "$LOG_FILE"
kill $WM_PID 2>/dev/null
kill $SWAY_PID 2>/dev/null

# Mostra log e abre shell
cat "$LOG_FILE"
exec /bin/bash
EOF

chmod +x chroot/usr/local/bin/start-genesi.sh

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
