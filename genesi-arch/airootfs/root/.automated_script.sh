#!/usr/bin/env bash
#
# Genesi OS Arch Edition - Automated Setup Script
# Based on CachyOS automated script
# Executed automatically on first boot

script_cmdline() {
    local param
    for param in $(</proc/cmdline); do
        case "${param}" in
            script=*)
                echo "${param#*=}"
                return 0
                ;;
        esac
    done
}

automated_script() {
    local script rt
    script="$(script_cmdline)"
    if [[ -n "${script}" && ! -x /tmp/startup_script ]]; then
        if [[ "${script}" =~ ^((http|https|ftp|tftp)://) ]]; then
            printf '%s: downloading %s\n' "$0" "${script}"
            systemd-run --pty --quiet -p Wants=network-online.target -p After=network-online.target \
                curl "${script}" --location --retry-connrefused --retry 10 --fail -s -o /tmp/startup_script
            rt=$?
        else
            cp "${script}" /tmp/startup_script
            rt=$?
        fi
        if [[ ${rt} -eq 0 ]]; then
            chmod +x /tmp/startup_script
            printf '%s: executing automated script\n' "$0"
            /tmp/startup_script
        fi
    fi
}

# Setup Genesi OS
setup_genesi() {
    # Create genesi user if doesn't exist
    if ! id -u genesi &>/dev/null; then
        useradd -m -G wheel,audio,video,storage,optical,network -s /bin/bash genesi
        echo "genesi:genesi" | chpasswd
        echo "root:genesi" | chpasswd
    fi

    # Enable sudo without password for wheel group
    echo "%wheel ALL=(ALL:ALL) NOPASSWD: ALL" > /etc/sudoers.d/g_wheel
    chmod 440 /etc/sudoers.d/g_wheel

    # Enable NetworkManager
    systemctl enable NetworkManager
    systemctl start NetworkManager

    # Generate locales
    locale-gen

    # Setup auto-login for genesi user
    mkdir -p /etc/systemd/system/getty@tty1.service.d
    cat > /etc/systemd/system/getty@tty1.service.d/autologin.conf << 'EOF'
[Service]
ExecStart=
ExecStart=-/sbin/agetty -o '-p -f -- \\u' --noclear --autologin genesi %I $TERM
EOF

    # Auto-start Hyprland on login (disabled for VirtualBox compatibility)
    if [ ! -f /home/genesi/.bash_profile ]; then
        cat > /home/genesi/.bash_profile << 'EOF'
# Auto-start Hyprland on tty1 (disabled)
# if [ -z "$DISPLAY" ] && [ "$XDG_VTNR" = 1 ]; then
#     exec Hyprland
# fi
EOF
        chown genesi:genesi /home/genesi/.bash_profile
    fi

    echo "Genesi OS setup complete!"
}

# Run automated script if provided
if [[ $(tty) == "/dev/tty1" ]]; then
    automated_script
    setup_genesi
fi
