#!/usr/bin/env bash
# Genesi OS - customize_airootfs.sh
# Runs inside chroot AFTER packages are installed, BEFORE squashfs creation.
# Overrides all CachyOS branding with Genesi OS.

set -e

echo ">>> Genesi OS: Applying branding..."

# ============================================================
# 1. System identity files
# ============================================================

echo "genesi" > /etc/hostname

cat > /etc/hosts << 'HOSTS'
127.0.0.1   localhost
::1         localhost
127.0.1.1   genesi.localdomain genesi
185.199.108.133 raw.githubusercontent.com
HOSTS

cat > /etc/os-release << 'OSRELEASE'
NAME="Genesi OS"
PRETTY_NAME="Genesi OS"
ID=genesi
ID_LIKE=arch
BUILD_ID=rolling
ANSI_COLOR="38;2;29;158;117"
HOME_URL="https://github.com/zFreshy/GenesiOS"
SUPPORT_URL="https://github.com/zFreshy/GenesiOS/issues"
BUG_REPORT_URL="https://github.com/zFreshy/GenesiOS/issues"
LOGO=genesi
OSRELEASE

cat > /etc/lsb-release << 'LSB'
LSB_VERSION=2.0
DISTRIB_ID=GenesiOS
DISTRIB_RELEASE=rolling
DISTRIB_DESCRIPTION="Genesi OS Linux"
LSB

# ============================================================
# 2. Apply Genesi OS desktop config overrides
# ============================================================

if [ -d /usr/share/genesi/skel-override ]; then
    cp -rf /usr/share/genesi/skel-override/. /etc/skel/
    if [ -d /home/liveuser ]; then
        cp -rf /usr/share/genesi/skel-override/. /home/liveuser/
        chown -R 1000:1000 /home/liveuser/
    fi
fi

# ============================================================
# 3. Rebrand CachyOS Hello (if still installed as dependency)
# ============================================================

# Patch the desktop file
if [ -f /usr/share/applications/cachyos-hello.desktop ]; then
    sed -i 's/CachyOS Hello/Genesi OS Welcome/g' /usr/share/applications/cachyos-hello.desktop
    sed -i 's/Name=CachyOS/Name=Genesi OS/g' /usr/share/applications/cachyos-hello.desktop
    sed -i 's/Comment=.*CachyOS.*/Comment=Welcome to Genesi OS/g' /usr/share/applications/cachyos-hello.desktop
fi

# Patch the cachyos-hello config if it exists
if [ -f /etc/cachyos-hello.conf ]; then
    sed -i 's/CachyOS/Genesi OS/g' /etc/cachyos-hello.conf
    sed -i 's/cachyos\.org/github.com\/zFreshy\/GenesiOS/g' /etc/cachyos-hello.conf
    sed -i 's/discuss\.cachyos\.org/github.com\/zFreshy\/GenesiOS\/issues/g' /etc/cachyos-hello.conf
fi

# Patch the cachyos-hello binary strings (replace in-place)
if [ -f /usr/bin/cachyos-hello ]; then
    # Replace visible strings in the binary
    sed -i 's/Welcome to CachyOS/Welcome to GenesiOS/g' /usr/bin/cachyos-hello 2>/dev/null || true
    sed -i 's/CachyOS Hello/Genesi OS Hello/g' /usr/bin/cachyos-hello 2>/dev/null || true
    sed -i 's/CachyOS rolling/Genesi OS rolling/g' /usr/bin/cachyos-hello 2>/dev/null || true
    sed -i 's/CachyOS Developers/Genesi OS Developers/g' /usr/bin/cachyos-hello 2>/dev/null || true
    sed -i 's/using CachyOS/using Genesi OS/g' /usr/bin/cachyos-hello 2>/dev/null || true
fi

# ============================================================
# 4. Rebrand Calamares (installed by cachyos-calamares-next)
# ============================================================

# Patch all Calamares branding files
if [ -d /etc/calamares ]; then
    find /etc/calamares -type f \( -name "*.conf" -o -name "*.qml" -o -name "*.yml" -o -name "*.yaml" \) -exec sed -i \
        -e 's/CachyOS/Genesi OS/g' \
        -e 's|https://cachyos.org|https://github.com/zFreshy/GenesiOS|g' \
        -e 's|https://discuss.cachyos.org|https://github.com/zFreshy/GenesiOS/issues|g' \
        {} + 2>/dev/null || true
fi

# Patch Calamares shared data
if [ -d /usr/share/calamares ]; then
    find /usr/share/calamares -type f \( -name "*.conf" -o -name "*.qml" -o -name "*.yml" -o -name "*.yaml" \) -exec sed -i \
        -e 's/CachyOS/Genesi OS/g' \
        -e 's|https://cachyos.org|https://github.com/zFreshy/GenesiOS|g' \
        {} + 2>/dev/null || true
fi

# ============================================================
# 5. Enable services with REAL symlinks
# ============================================================

mkdir -p /etc/systemd/system/multi-user.target.wants

# NetworkManager (fix: was not starting because symlink was a text file)
ln -sf /usr/lib/systemd/system/NetworkManager.service /etc/systemd/system/multi-user.target.wants/NetworkManager.service
ln -sf /usr/lib/systemd/system/NetworkManager-dispatcher.service /etc/systemd/system/dbus-org.freedesktop.nm-dispatcher.service

# Genesi branding service
if [ -f /etc/systemd/system/genesi-branding.service ]; then
    ln -sf /etc/systemd/system/genesi-branding.service /etc/systemd/system/multi-user.target.wants/genesi-branding.service
fi

# Bluetooth
mkdir -p /etc/systemd/system/bluetooth.target.wants
ln -sf /usr/lib/systemd/system/bluetooth.service /etc/systemd/system/bluetooth.target.wants/bluetooth.service

# SSH
ln -sf /usr/lib/systemd/system/sshd.service /etc/systemd/system/multi-user.target.wants/sshd.service

# VirtualBox guest
ln -sf /usr/lib/systemd/system/vboxservice.service /etc/systemd/system/multi-user.target.wants/vboxservice.service 2>/dev/null || true

# ============================================================
# 6. Generate locales
# ============================================================

sed -i 's/#en_US.UTF-8/en_US.UTF-8/' /etc/locale.gen
sed -i 's/#pt_BR.UTF-8/pt_BR.UTF-8/' /etc/locale.gen
locale-gen

# ============================================================
# 7. Global text replacement for any remaining CachyOS references
# ============================================================

# Replace in all autostart desktop files
find /etc/xdg/autostart -name "*.desktop" -exec sed -i 's/CachyOS/Genesi OS/g' {} + 2>/dev/null || true

# Replace in SDDM/login manager configs
find /etc -name "*.conf" -path "*/sddm*" -exec sed -i 's/CachyOS/Genesi OS/g' {} + 2>/dev/null || true

echo ">>> Genesi OS: Branding applied successfully!"
