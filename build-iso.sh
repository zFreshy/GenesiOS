#!/bin/bash
# Script automatizado para criar ISO do Genesi OS
# Uso: sudo ./build-iso.sh

set -e

echo "🔥 Genesi OS - Build ISO"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Verifica se está rodando como root
if [ "$EUID" -ne 0 ]; then 
    echo "❌ Este script precisa ser executado como root"
    echo "   Execute: sudo ./build-iso.sh"
    exit 1
fi

# Configurações
WORK_DIR="$HOME/genesi-iso-build"
GENESI_SOURCE="$(pwd)"
ISO_NAME="GenesiOS-$(date +%Y%m%d).iso"

echo "📁 Diretório de trabalho: $WORK_DIR"
echo "📦 Código fonte: $GENESI_SOURCE"
echo "💿 ISO final: $ISO_NAME"
echo ""

# Pergunta confirmação
read -p "Continuar? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 1/7: Instalando dependências"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

apt update
apt install -y \
    debootstrap \
    squashfs-tools \
    xorriso \
    isolinux \
    syslinux-efi \
    grub-pc-bin \
    grub-efi-amd64-bin \
    mtools

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 2/7: Criando sistema base"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Limpa diretório anterior se existir
if [ -d "$WORK_DIR" ]; then
    echo "⚠️  Removendo build anterior..."
    umount "$WORK_DIR/chroot/dev/pts" 2>/dev/null || true
    umount "$WORK_DIR/chroot/dev" 2>/dev/null || true
    umount "$WORK_DIR/chroot/proc" 2>/dev/null || true
    umount "$WORK_DIR/chroot/sys" 2>/dev/null || true
    rm -rf "$WORK_DIR"
fi

mkdir -p "$WORK_DIR"
cd "$WORK_DIR"

echo "📦 Criando sistema base Ubuntu 22.04..."
debootstrap --arch=amd64 jammy chroot http://archive.ubuntu.com/ubuntu/

# Monta sistemas
mount --bind /dev chroot/dev
mount --bind /dev/pts chroot/dev/pts
mount --bind /proc chroot/proc
mount --bind /sys chroot/sys

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 3/7: Configurando sistema"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Configura sistema dentro do chroot
chroot chroot /bin/bash << 'EOFCHROOT'
set -e

# Hostname
echo "genesi-os" > /etc/hostname

# Sources.list
cat > /etc/apt/sources.list << EOF
deb http://archive.ubuntu.com/ubuntu jammy main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu jammy-updates main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu jammy-security main restricted universe multiverse
EOF

# Atualiza
export DEBIAN_FRONTEND=noninteractive
apt update

# Instala kernel e sistema base
apt install -y \
    linux-generic \
    systemd \
    systemd-sysv \
    network-manager \
    sudo \
    curl \
    wget \
    git \
    build-essential \
    gcc \
    g++ \
    make \
    cmake \
    pkg-config \
    libudev-dev \
    libdbus-1-dev \
    libseat-dev

# Instala pacotes para Live CD
apt install -y \
    live-boot \
    live-boot-initramfs-tools \
    live-config \
    live-config-systemd \
    casper

# Instala dependências do Wayland/Smithay
apt install -y \
    libwayland-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    libegl1-mesa-dev \
    libgles2-mesa-dev \
    libgbm-dev \
    libinput-dev \
    libsystemd-dev \
    libdrm-dev \
    sway \
    xwayland \
    xdg-desktop-portal \
    xdg-desktop-portal-gtk \
    xdg-desktop-portal-wlr \
    dbus-x11 \
    mesa-utils \
    mesa-utils-extra

# Instala dependências do Tauri
apt install -y \
    libwebkit2gtk-4.1-dev \
    libgtk-3-dev \
    libayatana-appindicator3-dev \
    librsvg2-dev \
    libjavascriptcoregtk-4.1-dev \
    libsoup-3.0-dev \
    patchelf \
    libssl-dev \
    xdg-utils \
    desktop-file-utils \
    gtk2-engines \
    gtk2-engines-murrine \
    gtk2-engines-pixbuf \
    adwaita-icon-theme \
    gnome-themes-extra

# Instala navegadores
apt install -y chromium-browser firefox

# Instala servidor gráfico mínimo
apt install -y \
    xserver-xorg-core \
    xserver-xorg-video-all \
    xinit

# Instala Node.js 20
curl -fsSL https://deb.nodesource.com/setup_20.x | bash -
apt install -y nodejs

# Cria usuário genesi
useradd -m -s /bin/bash -G sudo genesi
echo "genesi:genesi" | chpasswd

# Permite que genesi rode weston como root sem senha
echo "genesi ALL=(ALL) NOPASSWD: /usr/bin/weston" >> /etc/sudoers.d/genesi-weston
chmod 440 /etc/sudoers.d/genesi-weston

# Ativa NetworkManager para ter internet automática
systemctl enable NetworkManager

# Configura autologin
mkdir -p /etc/systemd/system/getty@tty1.service.d
cat > /etc/systemd/system/getty@tty1.service.d/autologin.conf << EOF
[Service]
ExecStart=
ExecStart=-/sbin/agetty --autologin genesi --noclear %I \$TERM
EOF

EOFCHROOT

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 4/7: Instalando Rust"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Instala Rust para o usuário genesi
chroot chroot su - genesi -c "curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 5/7: Copiando e compilando Genesi OS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Copia código fonte
echo "📦 Copiando código fonte..."
mkdir -p chroot/home/genesi/GenesiOS
cp -r "$GENESI_SOURCE"/* chroot/home/genesi/GenesiOS/
chown -R 1000:1000 chroot/home/genesi/GenesiOS

# Compila dentro do chroot
echo "🔨 Compilando Genesi OS (isso pode demorar 10-15 minutos)..."
chroot chroot su - genesi -c '
set -e
source ~/.cargo/env
cd ~/GenesiOS/genesi-desktop/genesi-wm
echo "  → Compilando Window Manager..."
cargo build --release 2>&1 | tee /tmp/wm-build.log || { echo "ERRO WM"; exit 1; }
cd ~/GenesiOS/genesi-desktop
echo "  → Instalando dependências npm..."
npm install 2>&1 | tee /tmp/npm-install.log || { echo "ERRO NPM INSTALL"; exit 1; }
echo "  → Buildando frontend..."
npm run build 2>&1 | tee /tmp/npm-build.log || { echo "ERRO NPM BUILD"; exit 1; }
cd src-tauri
echo "  → Compilando Tauri (sem bundling)..."
cargo build --release 2>&1 | tee /tmp/tauri-build.log || { echo "ERRO TAURI"; exit 1; }
' || {
    echo ""
    echo "❌ ERRO na compilação!"
    echo ""
    echo "Logs disponíveis no chroot:"
    echo "  - /tmp/wm-build.log"
    echo "  - /tmp/npm-install.log"
    echo "  - /tmp/npm-build.log"
    echo "  - /tmp/tauri-build.log"
    echo ""
    echo "Para ver os logs:"
    echo "  sudo chroot $WORK_DIR/chroot cat /tmp/wm-build.log"
    echo ""
    exit 1
}

# Verifica se a compilação foi bem-sucedida
if [ ! -f "chroot/home/genesi/GenesiOS/genesi-desktop/src-tauri/target/release/genesi-desktop" ]; then
    echo "❌ ERRO: Falha na compilação do Genesi Desktop"
    echo "   Verifique: sudo chroot $WORK_DIR/chroot cat /tmp/tauri-build.log"
    exit 1
fi

if [ ! -f "chroot/home/genesi/GenesiOS/genesi-desktop/genesi-wm/target/release/genesi-wm" ]; then
    echo "❌ ERRO: Falha na compilação do Window Manager"
    echo "   Verifique: sudo chroot $WORK_DIR/chroot cat /tmp/wm-build.log"
    exit 1
fi

echo "✅ Compilação concluída com sucesso!"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 6/7: Configurando autostart"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Script de inicialização
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

# Inicia DBus se não estiver rodando
if [ -z "$DBUS_SESSION_BUS_ADDRESS" ]; then
    echo "Starting DBus session..." >> "$LOG_FILE"
    eval $(dbus-launch --sh-syntax)
    echo "DBus session started: $DBUS_SESSION_BUS_ADDRESS" >> "$LOG_FILE"
fi

echo "Starting Sway (Wayland Compositor)..." >> "$LOG_FILE"

# Configura Sway para não mostrar barra de status
mkdir -p ~/.config/sway
cat > ~/.config/sway/config << 'SWAYEOF'
# Genesi OS - Sway Configuration
# Remove barra de status e decorações
bar {
    mode invisible
}

# Remove bordas das janelas
default_border none
default_floating_border none

# Sem gaps
gaps inner 0
gaps outer 0

# Foco segue o mouse
focus_follows_mouse yes
SWAYEOF

# Inicia Sway
sway >> "$LOG_FILE" 2>&1 &
SWAY_PID=$!
echo "Sway PID: $SWAY_PID" >> "$LOG_FILE"

# Aguarda Sway criar o socket
sleep 3

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
    ls -la "$XDG_RUNTIME_DIR/" >> "$LOG_FILE"
    cat "$LOG_FILE"
    kill $SWAY_PID 2>/dev/null
    exec /bin/bash
    exit 1
fi

echo "Sway started successfully on $WAYLAND_DISPLAY" >> "$LOG_FILE"

# Aguarda mais um pouco para Sway estabilizar
sleep 2

# Inicia xdg-desktop-portal para Tauri/GTK
echo "Starting xdg-desktop-portal..." >> "$LOG_FILE"
/usr/libexec/xdg-desktop-portal >> "$LOG_FILE" 2>&1 &
XDG_PORTAL_PID=$!
echo "xdg-desktop-portal PID: $XDG_PORTAL_PID" >> "$LOG_FILE"
sleep 2

# Configura GTK para Wayland
export GDK_BACKEND=wayland
export GTK_THEME=Adwaita:dark

# Inicia Desktop diretamente no Sway
echo "Starting Genesi Desktop on Sway..." >> "$LOG_FILE"
echo "WAYLAND_DISPLAY=$WAYLAND_DISPLAY" >> "$LOG_FILE"
echo "XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR" >> "$LOG_FILE"

cd /home/genesi/GenesiOS/genesi-desktop/src-tauri/target/release

# Verifica se o binário existe
if [ ! -f "./genesi-desktop" ]; then
    echo "ERROR: genesi-desktop binary not found!" >> "$LOG_FILE"
    cat "$LOG_FILE"
    kill $SWAY_PID 2>/dev/null
    exec /bin/bash
    exit 1
fi

# Roda o desktop diretamente no Sway
WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
DBUS_SESSION_BUS_ADDRESS=$DBUS_SESSION_BUS_ADDRESS \
GDK_BACKEND=wayland \
QT_QPA_PLATFORM=wayland \
SDL_VIDEODRIVER=wayland \
CLUTTER_BACKEND=wayland \
MOZ_ENABLE_WAYLAND=1 \
WEBKIT_DISABLE_COMPOSITING_MODE=1 \
WEBKIT_DISABLE_DMABUF_RENDERER=1 \
NO_AT_BRIDGE=1 \
GTK_THEME=Adwaita:dark \
DISPLAY="" \
RUST_BACKTRACE=1 \
./genesi-desktop >> "$LOG_FILE" 2>&1

# Se o desktop fechar, mata tudo
echo "Desktop closed, cleaning up..." >> "$LOG_FILE"
kill $SWAY_PID 2>/dev/null

# Mostra log e abre shell
cat "$LOG_FILE"
exec /bin/bash
EOF

chmod +x chroot/usr/local/bin/start-genesi.sh

# Cria .xinitrc que NÃO usa startx, roda direto
cat > chroot/home/genesi/.xinitrc << 'EOF'
#!/bin/bash
# Genesi OS não usa X11, roda Wayland puro
exec /usr/local/bin/start-genesi.sh
EOF

chmod +x chroot/home/genesi/.xinitrc
chown 1000:1000 chroot/home/genesi/.xinitrc chroot/home/genesi/.bashrc

# Configura autologin no Live CD via live-config
mkdir -p chroot/etc/live/config.conf.d
cat > chroot/etc/live/config.conf.d/genesi.conf << 'EOF'
LIVE_USERNAME="genesi"
LIVE_HOSTNAME="genesi-os"
LIVE_USER_FULLNAME="Genesi User"
EOF

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 7/7: Gerando ISO"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Desmonta
umount chroot/dev/pts
umount chroot/dev
umount chroot/proc
umount chroot/sys

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
echo "  ✅ ISO criada com sucesso!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📍 Localização: $GENESI_SOURCE/$ISO_NAME"
echo "📊 Tamanho: $(du -h "$GENESI_SOURCE/$ISO_NAME" | cut -f1)"
echo ""
echo "🖥️  Para testar na VM:"
echo "   VirtualBox: Crie VM com 4GB RAM, 20GB disco, adicione a ISO"
echo "   VMware: Crie VM Linux/Ubuntu 64-bit, adicione a ISO"
echo "   QEMU: qemu-system-x86_64 -m 4096 -cdrom $ISO_NAME -boot d"
echo ""
echo "🎉 Pronto para bootar!"
echo ""
