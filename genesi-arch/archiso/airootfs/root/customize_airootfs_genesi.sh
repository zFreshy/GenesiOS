#!/usr/bin/env bash
# Install Genesi packages after mkarchiso

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🎨 Genesi OS package installation"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

echo "⏭️  Skipping local package installation (using repository packages)"
echo ""
echo "📋 Genesi packages will be installed from repositories during system installation"
echo ""

# Verify that genesi-settings is available in repositories
echo "🔍 Checking if genesi-settings is available..."
if pacman -Si genesi-settings &>/dev/null; then
    echo "✅ genesi-settings found in repositories"
else
    echo "⚠️  genesi-settings not found in repositories"
    echo "    It will be installed during system installation via pacstrap"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Package check complete!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
