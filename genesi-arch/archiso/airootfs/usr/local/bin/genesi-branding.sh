#!/bin/bash
# Genesi OS Branding Script
# Runs on live boot to replace CachyOS branding with Genesi OS

# Apply Genesi OS desktop config overrides to liveuser
OVERRIDE_DIR="/usr/share/genesi/skel-override"
LIVEUSER_HOME="/home/liveuser"
if [ -d "$OVERRIDE_DIR" ] && [ -d "$LIVEUSER_HOME" ]; then
    cp -rf "$OVERRIDE_DIR"/. "$LIVEUSER_HOME"/
    chown -R liveuser:liveuser "$LIVEUSER_HOME"
fi

# Also apply to /etc/skel for installed system
if [ -d "$OVERRIDE_DIR" ]; then
    cp -rf "$OVERRIDE_DIR"/. /etc/skel/
fi

# Install Genesi OS color scheme system-wide
if [ -f /usr/share/color-schemes/GenesiOS.colors ]; then
    chmod 644 /usr/share/color-schemes/GenesiOS.colors
fi

# Replace CachyOS Hello window title and text
if [ -f /usr/bin/cachyos-hello ]; then
    # Create a wrapper that patches the window title
    DESKTOP_FILE="/usr/share/applications/cachyos-hello.desktop"
    if [ -f "$DESKTOP_FILE" ]; then
        sed -i 's/CachyOS Hello/Genesi OS Welcome/g' "$DESKTOP_FILE"
        sed -i 's/CachyOS/Genesi OS/g' "$DESKTOP_FILE"
        sed -i 's/Name=Genesi OS Welcome/Name=Genesi OS Welcome/g' "$DESKTOP_FILE"
    fi
fi

# Replace CachyOS references in Calamares branding
CALA_BRANDING="/etc/calamares/branding"
if [ -d "$CALA_BRANDING" ]; then
    find "$CALA_BRANDING" -type f -name "*.conf" -exec sed -i \
        -e 's/CachyOS/Genesi OS/g' \
        -e 's/cachyos/genesi/g' \
        -e 's|https://cachyos.org|https://github.com/zFreshy/GenesiOS|g' \
        -e 's|https://discuss.cachyos.org|https://github.com/zFreshy/GenesiOS/issues|g' \
        {} +
    find "$CALA_BRANDING" -type f -name "*.qml" -exec sed -i \
        -e 's/CachyOS/Genesi OS/g' \
        -e 's/cachyos/genesi/g' \
        {} +
fi

# Replace in Calamares settings
CALA_SETTINGS="/etc/calamares/settings.conf"
if [ -f "$CALA_SETTINGS" ]; then
    sed -i 's/CachyOS/Genesi OS/g' "$CALA_SETTINGS"
fi

# Replace in Calamares modules
CALA_MODULES="/etc/calamares/modules"
if [ -d "$CALA_MODULES" ]; then
    find "$CALA_MODULES" -type f -name "*.conf" -exec sed -i \
        -e 's/CachyOS/Genesi OS/g' \
        -e 's/cachyos/genesi/g' \
        {} +
fi

# Replace in welcome app config
if [ -f /etc/cachyos-hello.conf ]; then
    sed -i 's/CachyOS/Genesi OS/g' /etc/cachyos-hello.conf
fi

# Update lsb-release
if [ -f /etc/lsb-release ]; then
    sed -i 's/CachyOS/Genesi OS/g' /etc/lsb-release
fi

echo "Genesi OS branding applied."
