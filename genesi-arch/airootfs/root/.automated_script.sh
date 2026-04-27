#!/bin/bash

# Genesi OS Arch Edition - Automated Setup Script
# Executado automaticamente no primeiro boot

# Criar usuário genesi
if ! id -u genesi &>/dev/null; then
    useradd -m -G wheel,audio,video,storage,optical,network -s /bin/bash genesi
    echo "genesi:genesi" | chpasswd
    echo "root:genesi" | chpasswd
fi

# Habilitar sudo sem senha para wheel
echo "%wheel ALL=(ALL:ALL) NOPASSWD: ALL" > /etc/sudoers.d/wheel

# Habilitar NetworkManager
systemctl enable NetworkManager
systemctl start NetworkManager

# Configurar locales
locale-gen

# Auto-login para usuário genesi
mkdir -p /etc/systemd/system/getty@tty1.service.d
cat > /etc/systemd/system/getty@tty1.service.d/autologin.conf << EOF
[Service]
ExecStart=
ExecStart=-/sbin/agetty -o '-p -f -- \\u' --noclear --autologin genesi %I \$TERM
EOF

# Iniciar Hyprland automaticamente
if [ ! -f /home/genesi/.bash_profile ]; then
    cat > /home/genesi/.bash_profile << 'EOF'
if [ -z "$DISPLAY" ] && [ "$XDG_VTNR" = 1 ]; then
    exec Hyprland
fi
EOF
    chown genesi:genesi /home/genesi/.bash_profile
fi

echo "Genesi OS Arch Edition setup complete!"
