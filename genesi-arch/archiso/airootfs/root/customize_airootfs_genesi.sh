#!/usr/bin/env bash
# Install Genesi packages after mkarchiso

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🎨 Installing Genesi OS packages..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Packages are in /opt/genesi-packages/ (part of airootfs)
GENESI_PKG_DIR="/opt/genesi-packages"

echo "🔍 Checking for Genesi packages directory..."
if [ ! -d "$GENESI_PKG_DIR" ]; then
    echo "❌ Genesi packages directory not found: $GENESI_PKG_DIR"
    echo "❌ Skipping Genesi package installation"
    echo ""
    echo "📂 Available directories in /opt:"
    ls -la /opt/ || true
    exit 0
fi

echo "✅ Found packages directory: $GENESI_PKG_DIR"
echo ""
echo "📦 Package files:"
ls -lh "$GENESI_PKG_DIR"/*.pkg.tar.zst 2>/dev/null || {
    echo "❌ No .pkg.tar.zst files found in $GENESI_PKG_DIR"
    echo ""
    echo "📂 Contents of $GENESI_PKG_DIR:"
    ls -la "$GENESI_PKG_DIR" || true
    exit 0
}

# Install all packages
echo ""
echo "📦 Installing Genesi packages..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Install packages one by one to see which ones fail
for pkg in "$GENESI_PKG_DIR"/*.pkg.tar.zst; do
    echo ""
    echo "Installing: $(basename "$pkg")"
    # Use --overwrite to force installation even if files conflict
    # Use --needed to skip if already installed
    if pacman -U --noconfirm --needed --overwrite '*' "$pkg"; then
        echo "✅ Installed: $(basename "$pkg")"
    else
        echo "⚠️  Failed to install: $(basename "$pkg")"
        echo "Trying with --nodeps and --force..."
        if pacman -U --noconfirm --nodeps --overwrite '*' "$pkg"; then
            echo "✅ Installed with --nodeps: $(basename "$pkg")"
        else
            echo "❌ Failed completely: $(basename "$pkg")"
        fi
    fi
done

# Remove CachyOS branding packages that conflict
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🗑️  Removing CachyOS branding packages..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

CACHYOS_PACKAGES=(
    "cachyos-calamares-next"
    "cachyos-settings"
    "cachyos-kde-settings"
    "cachyos-hello"
)

for pkg in "${CACHYOS_PACKAGES[@]}"; do
    if pacman -Q "$pkg" &>/dev/null; then
        echo "Removing: $pkg"
        pacman -Rdd --noconfirm "$pkg" 2>/dev/null || echo "⚠️  Could not remove $pkg"
    else
        echo "Not installed: $pkg (skipping)"
    fi
done

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Genesi packages installation complete!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📋 Installed Genesi packages:"
pacman -Q | grep genesi || echo "⚠️  No Genesi packages found in pacman database"
echo ""
