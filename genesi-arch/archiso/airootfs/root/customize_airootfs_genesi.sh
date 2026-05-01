#!/usr/bin/env bash
# Install Genesi packages after mkarchiso

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🎨 Installing Genesi OS packages..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

GENESI_PACKAGES=(
    "/root/genesi-packages/genesi-settings-1.0.0-1-any.pkg.tar.zst"
    "/root/genesi-packages/genesi-kde-settings-1.0.0-2-any.pkg.tar.zst"
    "/root/genesi-packages/genesi-ai-mode-1.0.0-1-any.pkg.tar.zst"
    "/root/genesi-packages/genesi-updater-1.0.0-1-any.pkg.tar.zst"
)

# Install each package
for pkg in "${GENESI_PACKAGES[@]}"; do
    if [ -f "$pkg" ]; then
        echo "📦 Installing: $(basename $pkg)"
        pacman -U --noconfirm "$pkg"
    else
        echo "⚠️  Package not found: $pkg"
    fi
done

# Remove CachyOS branding packages that conflict
echo "🗑️  Removing CachyOS branding..."
pacman -Rdd --noconfirm cachyos-settings cachyos-kde-settings cachyos-hello 2>/dev/null || true

echo "✅ Genesi packages installed!"
