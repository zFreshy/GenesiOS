#!/usr/bin/env bash
# Genesi OS - customize_airootfs.sh
# Runs inside chroot AFTER packages are installed, BEFORE squashfs creation.
# Overrides all CachyOS branding with Genesi OS.

# Don't use set -e - we want to continue even if some commands fail
echo ">>> Genesi OS: Applying branding..."

# ============================================================
# 0. Generate Plymouth progress bar images (if convert is available)
# ============================================================
if command -v convert &>/dev/null; then
    echo ">>> Generating Plymouth progress bar images..."
    convert -size 300x6 xc:'#0A1E1A' -fill '#0F6E56' -draw 'roundrectangle 0,0 299,5 3,3' \
        /usr/share/plymouth/themes/genesi/progress-bg.png 2>/dev/null || true
    convert -size 296x4 xc:'#1D9E75' -fill '#1D9E75' -draw 'roundrectangle 0,0 295,3 2,2' \
        /usr/share/plymouth/themes/genesi/progress-bar.png 2>/dev/null || true
fi

# Set Plymouth theme if plymouth is installed
if command -v plymouth-set-default-theme &>/dev/null; then
    echo ">>> Setting Plymouth theme to genesi..."
    plymouth-set-default-theme genesi 2>/dev/null || true
fi

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
    echo ">>> Copying skel-override to /etc/skel and /home/liveuser..."
    cp -rf /usr/share/genesi/skel-override/. /etc/skel/
    echo ">>> Copied to /etc/skel"
    if [ -d /home/liveuser ]; then
        cp -rf /usr/share/genesi/skel-override/. /home/liveuser/
        chmod +x /home/liveuser/Desktop/*.desktop 2>/dev/null || true
        chown -R 1000:1000 /home/liveuser/
        echo ">>> Copied to /home/liveuser"
    else
        echo ">>> WARNING: /home/liveuser does not exist yet"
        # Create liveuser home and copy
        mkdir -p /home/liveuser
        cp -rf /usr/share/genesi/skel-override/. /home/liveuser/
        chmod +x /home/liveuser/Desktop/*.desktop 2>/dev/null || true
        chown -R 1000:1000 /home/liveuser/ 2>/dev/null || true
        echo ">>> Created /home/liveuser and copied overrides"
    fi
    echo ">>> skel-override contents:"
    find /usr/share/genesi/skel-override -type f
else
    echo ">>> WARNING: /usr/share/genesi/skel-override NOT FOUND!"
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

# Remove cachyos-hello binary (it has hardcoded CachyOS text that can't be changed)
# We'll replace it with our own Genesi Welcome app later
if [ -f /usr/bin/cachyos-hello ]; then
    rm -f /usr/bin/cachyos-hello
fi
# Remove its autostart
rm -f /etc/xdg/autostart/cachyos-hello.desktop 2>/dev/null || true

# ============================================================
# 4. Rebrand Calamares (installed by cachyos-calamares-next)
# ============================================================

# Patch branding.desc and copy to genesi folder
if [ -f /usr/share/calamares/branding/cachyos/branding.desc ]; then
    # Copy the entire cachyos branding to genesi folder
    cp -rf /usr/share/calamares/branding/cachyos/* /usr/share/calamares/branding/genesi/ 2>/dev/null || true
    # Now patch the genesi copy
    sed -i \
        -e 's/productName:.*CachyOS/productName:       Genesi OS/' \
        -e 's/shortProductName:.*CachyOS/shortProductName:  Genesi OS/' \
        -e 's/versionedName:.*CachyOS/versionedName:     Genesi OS/' \
        -e 's/shortVersionedName:.*CachyOS/shortVersionedName: Genesi OS/' \
        -e 's/bootLoaderEntryName:.*CachyOS/bootLoaderEntryName: Genesi OS/' \
        -e 's/componentName:.*cachyos/componentName:     genesi/' \
        -e 's|https://cachyos.org|https://github.com/zFreshy/GenesiOS|g' \
        -e 's|https://discuss.cachyos.org|https://github.com/zFreshy/GenesiOS/issues|g' \
        -e 's|https://paste.cachyos.org|https://github.com/zFreshy/GenesiOS|g' \
        /usr/share/calamares/branding/genesi/branding.desc
fi

# Also copy to /etc/calamares/branding/genesi if that path is used
mkdir -p /etc/calamares/branding/genesi
if [ -f /usr/share/calamares/branding/genesi/branding.desc ]; then
    cp -rf /usr/share/calamares/branding/genesi/* /etc/calamares/branding/genesi/
fi

# Update Calamares settings to use genesi branding
find /usr/share/calamares /etc/calamares -type f -name "settings*.conf" -exec sed -i \
    -e 's/branding:.*cachyos/branding: genesi/' \
    -e 's/CachyOS/Genesi OS/g' \
    {} + 2>/dev/null || true

# Patch all other Calamares config files
if [ -d /etc/calamares ]; then
    find /etc/calamares -type f \( -name "*.conf" -o -name "*.qml" -o -name "*.yml" -o -name "*.yaml" -o -name "*.desc" \) -exec sed -i \
        -e 's/CachyOS/Genesi OS/g' \
        -e 's|https://cachyos.org|https://github.com/zFreshy/GenesiOS|g' \
        {} + 2>/dev/null || true
fi
if [ -d /usr/share/calamares ]; then
    find /usr/share/calamares -type f \( -name "*.conf" -o -name "*.qml" -o -name "*.yml" -o -name "*.yaml" -o -name "*.desc" \) -exec sed -i \
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

# ============================================================
# 8. Configure SDDM theme
# ============================================================

# Ensure SDDM uses the Genesi theme
mkdir -p /etc/sddm.conf.d
cat > /etc/sddm.conf.d/genesi-theme.conf << 'SDDMCONF'
[Theme]
Current=genesi
CursorTheme=breeze_cursors
Font=Noto Sans,10

[General]
Numlock=on
InputMethod=

[Users]
MaximumUid=60513
MinimumUid=1000
SDDMCONF

echo ">>> Genesi OS: Branding applied successfully!"
