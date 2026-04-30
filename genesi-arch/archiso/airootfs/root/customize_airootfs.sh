#!/usr/bin/env bash
# Genesi OS - customize_airootfs.sh
# Runs inside chroot AFTER packages are installed, BEFORE squashfs creation.
# This is the right place to override files installed by CachyOS packages.

set -e

echo ">>> Genesi OS: Applying branding..."

# Override hostname (cachyos-settings sets it to CachyOS)
echo "genesi" > /etc/hostname

# Override hosts
cat > /etc/hosts << 'HOSTS'
127.0.0.1   localhost
::1         localhost
127.0.1.1   genesi.localdomain genesi
185.199.108.133 raw.githubusercontent.com
HOSTS

# Override os-release (cachyos-settings overwrites this)
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

# Override lsb-release
cat > /etc/lsb-release << 'LSB'
LSB_VERSION=2.0
DISTRIB_ID=GenesiOS
DISTRIB_RELEASE=rolling
DISTRIB_DESCRIPTION="Genesi OS Linux"
LSB

# Apply Genesi OS desktop config overrides
# (cachyos-kde-settings installs its own configs, we override them)
if [ -d /usr/share/genesi/skel-override ]; then
    cp -rf /usr/share/genesi/skel-override/. /etc/skel/
    # Also apply to liveuser home if it exists
    if [ -d /home/liveuser ]; then
        cp -rf /usr/share/genesi/skel-override/. /home/liveuser/
        chown -R 1000:1000 /home/liveuser/
    fi
fi

# Rebrand CachyOS Hello app
if [ -f /usr/share/applications/cachyos-hello.desktop ]; then
    sed -i 's/CachyOS Hello/Genesi OS Welcome/g' /usr/share/applications/cachyos-hello.desktop
    sed -i 's/CachyOS/Genesi OS/g' /usr/share/applications/cachyos-hello.desktop
fi

# Rebrand CachyOS Hello config
if [ -f /etc/cachyos-hello.conf ]; then
    sed -i 's/CachyOS/Genesi OS/g' /etc/cachyos-hello.conf
fi

# Rebrand Calamares branding
if [ -d /etc/calamares/branding ]; then
    find /etc/calamares/branding -type f \( -name "*.conf" -o -name "*.qml" \) -exec sed -i \
        -e 's/CachyOS/Genesi OS/g' \
        -e 's|https://cachyos.org|https://github.com/zFreshy/GenesiOS|g' \
        -e 's|https://discuss.cachyos.org|https://github.com/zFreshy/GenesiOS/issues|g' \
        {} +
fi

# Rebrand Calamares settings
if [ -f /etc/calamares/settings.conf ]; then
    sed -i 's/CachyOS/Genesi OS/g' /etc/calamares/settings.conf
fi

# Rebrand Calamares modules
if [ -d /etc/calamares/modules ]; then
    find /etc/calamares/modules -type f -name "*.conf" -exec sed -i \
        -e 's/CachyOS/Genesi OS/g' \
        {} +
fi

# Enable genesi-branding service (creates real symlink)
ln -sf /etc/systemd/system/genesi-branding.service /etc/systemd/system/multi-user.target.wants/genesi-branding.service

# Generate locales
sed -i 's/#en_US.UTF-8/en_US.UTF-8/' /etc/locale.gen
sed -i 's/#pt_BR.UTF-8/pt_BR.UTF-8/' /etc/locale.gen
locale-gen

echo ">>> Genesi OS: Branding applied successfully!"
