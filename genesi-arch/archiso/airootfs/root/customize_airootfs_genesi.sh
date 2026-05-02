#!/usr/bin/env bash
# Install Genesi packages after mkarchiso

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🎨 Installing Genesi OS packages..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Packages are in /opt/genesi-packages/ (part of airootfs)
GENESI_PKG_DIR="/opt/genesi-packages"

if [ ! -d "$GENESI_PKG_DIR" ]; then
    echo "❌ Genesi packages directory not found: $GENESI_PKG_DIR"
    echo "❌ Skipping Genesi package installation"
    exit 0
fi

echo "📁 Found packages in: $GENESI_PKG_DIR"
ls -lh "$GENESI_PKG_DIR"/*.pkg.tar.zst 2>/dev/null || {
    echo "❌ No packages found in $GENESI_PKG_DIR"
    exit 0
}

# Install all packages
echo ""
echo "📦 Installing Genesi packages..."
pacman -U --noconfirm "$GENESI_PKG_DIR"/*.pkg.tar.zst || {
    echo "⚠️  Some packages failed to install, continuing..."
}

# Remove CachyOS branding packages that conflict
echo ""
echo "🗑️  Removing CachyOS branding packages..."
pacman -Rdd --noconfirm cachyos-calamares-next cachyos-settings cachyos-kde-settings cachyos-hello 2>/dev/null || true

echo ""
echo "✅ Genesi packages installation complete!"
echo ""
echo "Installed Genesi packages:"
pacman -Q | grep genesi || echo "⚠️  No Genesi packages found"
