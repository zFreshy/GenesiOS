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
    pkg-config

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
    xdg-utils

# Instala navegadores
apt install -y chromium-browser firefox

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
source ~/.cargo/env
cd ~/GenesiOS/genesi-desktop/genesi-wm
echo "  → Compilando Window Manager..."
cargo build --release
cd ~/GenesiOS/genesi-desktop
echo "  → Instalando dependências npm..."
npm install
echo "  → Buildando frontend..."
npm run build
cd src-tauri
echo "  → Compilando Tauri (sem bundling)..."
cargo build --release
'

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Passo 6/7: Configurando autostart"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Script de inicialização
cat > chroot/usr/local/bin/start-genesi.sh << 'EOF'
#!/bin/bash
export DISPLAY=:0
export WAYLAND_DISPLAY=wayland-0
export XDG_RUNTIME_DIR=/run/user/$(id -u)

# Aguarda sistema gráfico
sleep 2

# Inicia Window Manager
/home/genesi/GenesiOS/genesi-desktop/genesi-wm/target/release/genesi-wm &
WM_PID=$!

# Aguarda WM iniciar
sleep 3

# Inicia Desktop
cd /home/genesi/GenesiOS/genesi-desktop/src-tauri/target/release
./genesi-desktop

# Se o desktop fechar, mata o WM
kill $WM_PID 2>/dev/null
EOF

chmod +x chroot/usr/local/bin/start-genesi.sh

# Systemd service
cat > chroot/etc/systemd/system/genesi.service << 'EOF'
[Unit]
Description=Genesi OS Desktop Environment
After=graphical.target

[Service]
Type=simple
User=genesi
Environment="DISPLAY=:0"
Environment="WAYLAND_DISPLAY=wayland-0"
ExecStart=/usr/local/bin/start-genesi.sh
Restart=on-failure

[Install]
WantedBy=graphical.target
EOF

# Habilita serviço
chroot chroot systemctl enable genesi.service

# Configura .bashrc para iniciar automaticamente
cat >> chroot/home/genesi/.bashrc << 'EOF'

# Inicia Genesi OS automaticamente no login
if [ -z "$DISPLAY" ] && [ "$(tty)" = "/dev/tty1" ]; then
    exec startx
fi
EOF

# Cria .xinitrc
cat > chroot/home/genesi/.xinitrc << 'EOF'
#!/bin/bash
exec /usr/local/bin/start-genesi.sh
EOF

chmod +x chroot/home/genesi/.xinitrc
chown 1000:1000 chroot/home/genesi/.xinitrc

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

# GRUB config
cat > iso/boot/grub/grub.cfg << 'EOF'
set timeout=5
set default=0

menuentry "Genesi OS" {
    linux /boot/vmlinuz boot=live quiet splash
    initrd /boot/initrd.img
}

menuentry "Genesi OS (Safe Mode)" {
    linux /boot/vmlinuz boot=live nomodeset
    initrd /boot/initrd.img
}

menuentry "Genesi OS (Debug)" {
    linux /boot/vmlinuz boot=live debug
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
