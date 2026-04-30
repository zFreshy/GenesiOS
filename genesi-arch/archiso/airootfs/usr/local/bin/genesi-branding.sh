#!/bin/bash
# Genesi OS Runtime Branding Script
# Runs on every boot to ensure Genesi OS branding is applied.
# Handles files that may be regenerated or downloaded at runtime.

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

# Ensure hostname is correct
echo "genesi" > /etc/hostname

# Rebrand any CachyOS references that may have been regenerated
if [ -f /usr/share/applications/cachyos-hello.desktop ]; then
    sed -i 's/CachyOS Hello/Genesi OS Welcome/g' /usr/share/applications/cachyos-hello.desktop
    sed -i 's/CachyOS/Genesi OS/g' /usr/share/applications/cachyos-hello.desktop
fi

if [ -f /etc/cachyos-hello.conf ]; then
    sed -i 's/CachyOS/Genesi OS/g' /etc/cachyos-hello.conf
fi

# Rebrand Calamares if it was downloaded
if [ -d /etc/calamares ]; then
    find /etc/calamares -type f \( -name "*.conf" -o -name "*.qml" -o -name "*.yml" \) \
        -exec sed -i -e 's/CachyOS/Genesi OS/g' {} + 2>/dev/null || true
fi
if [ -d /usr/share/calamares ]; then
    find /usr/share/calamares -type f \( -name "*.conf" -o -name "*.qml" -o -name "*.yml" \) \
        -exec sed -i -e 's/CachyOS/Genesi OS/g' {} + 2>/dev/null || true
fi

echo "Genesi OS branding applied."
