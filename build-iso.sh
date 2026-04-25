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
    libegl1-mesa-dev \
    libgles2-mesa-dev \
    libgbm-dev \
    libinput-dev \
    libsystemd-dev \
    libdrm-dev

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
    desktop-file-utils

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

echo "Starting Genesi WM (Wayland Compositor)..." >> "$LOG_FILE"

# IMPORTANTE: Genesi WM é um compositor Wayland, precisa rodar direto no DRM
# Não usa DISPLAY ou WAYLAND_DISPLAY antes de iniciar
cd /home/genesi/GenesiOS/genesi-desktop/genesi-wm
./target/release/genesi-wm >> "$LOG_FILE" 2>&1 &
WM_PID=$!
echo "WM PID: $WM_PID" >> "$LOG_FILE"

# Aguarda WM criar o socket Wayland
sleep 5

# Verifica se WM está rodando
if ! kill -0 $WM_PID 2>/dev/null; then
    echo "ERROR: WM crashed!" >> "$LOG_FILE"
    cat "$LOG_FILE"
    echo ""
    echo "WM failed to start. Check log above."
    exec /bin/bash
    exit 1
fi

# Detecta o socket Wayland criado pelo WM
if [ -S "$XDG_RUNTIME_DIR/wayland-0" ]; then
    export WAYLAND_DISPLAY=wayland-0
elif [ -S "$XDG_RUNTIME_DIR/wayland-1" ]; then
    export WAYLAND_DISPLAY=wayland-1
else
    echo "ERROR: No Wayland socket found!" >> "$LOG_FILE"
    cat "$LOG_FILE"
    kill $WM_PID 2>/dev/null
    exec /bin/bash
    exit 1
fi

echo "WM started successfully on $WAYLAND_DISPLAY" >> "$LOG_FILE"

# Aguarda mais um pouco
sleep 2

# Inicia Desktop
echo "Starting Genesi Desktop..." >> "$LOG_FILE"
cd /home/genesi/GenesiOS/genesi-desktop/src-tauri/target/release
./genesi-desktop >> "$LOG_FILE" 2>&1

# Se o desktop fechar, mata o WM
echo "Desktop closed, killing WM..." >> "$LOG_FILE"
kill $WM_PID 2>/dev/null

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
