#!/usr/bin/env bash
# Install Genesi packages after mkarchiso

# DON'T use set -e - we want to continue even if some packages fail
# set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🎨 Installing Genesi OS packages..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# FIRST: Install libpamac-dummy to replace the broken libpamac-aur
echo "🔧 Installing libpamac-dummy to prevent install script errors..."
if [ -f /opt/genesi-packages/libpamac-dummy*.pkg.tar.zst ]; then
    pacman -U --noconfirm --overwrite '*' /opt/genesi-packages/libpamac-dummy*.pkg.tar.zst || echo "⚠️  Failed to install libpamac-dummy"
else
    echo "⚠️  libpamac-dummy not found"
fi

# SECOND: Remove problematic packages that have broken install scripts
echo "🗑️  Removing problematic packages..."
pacman -Rdd --noconfirm libpamac-aur libpamac pamac-aur 2>/dev/null || echo "Pamac packages not installed or already removed"

# CRITICAL: Disable signature verification for local packages
echo "🔓 Disabling GPG signature verification for local packages..."
sed -i 's/^LocalFileSigLevel.*/LocalFileSigLevel = Never/' /etc/pacman.conf
sed -i 's/^SigLevel.*/SigLevel = Never/' /etc/pacman.conf

# Verify the change
echo "📋 Current pacman.conf signature settings:"
grep -E "^(Local)?SigLevel" /etc/pacman.conf || echo "No SigLevel found"

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

# Install all packages at once (faster and cleaner)
echo ""
echo "📦 Installing all Genesi packages..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if pacman -U --noconfirm --needed --overwrite '*' "$GENESI_PKG_DIR"/*.pkg.tar.zst; then
    echo "✅ All Genesi packages installed successfully!"
else
    echo "⚠️  Some packages failed, trying one by one..."
    # Install packages one by one to see which ones fail
    for pkg in "$GENESI_PKG_DIR"/*.pkg.tar.zst; do
        echo ""
        echo "Installing: $(basename "$pkg")"
        if pacman -U --noconfirm --needed --overwrite '*' "$pkg"; then
            echo "✅ Installed: $(basename "$pkg")"
        else
            echo "❌ Failed: $(basename "$pkg")"
        fi
    done
fi

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
    "libpamac-aur"
    "libpamac"
    "pamac-aur"
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
