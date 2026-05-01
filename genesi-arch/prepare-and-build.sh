#!/bin/bash
# Prepare local repository and build ISO

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🚀 Genesi OS - Prepare and Build ISO"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Step 1: Build packages if not already built
if [ ! -d "$SCRIPT_DIR/local-repo" ] || [ ! -f "$SCRIPT_DIR/local-repo/genesi.db" ]; then
    echo "📦 Building Genesi packages..."
    bash "$SCRIPT_DIR/build-local-packages.sh"
else
    echo "✅ Packages already built"
fi

# Step 2: Copy packages to airootfs for installation during build
echo ""
echo "📋 Copying packages to airootfs..."
sudo mkdir -p "$SCRIPT_DIR/archiso/airootfs/root/genesi-packages"
sudo cp "$SCRIPT_DIR/local-repo/"*.pkg.tar.zst "$SCRIPT_DIR/archiso/airootfs/root/genesi-packages/"
sudo chmod +x "$SCRIPT_DIR/archiso/airootfs/root/customize_airootfs_genesi.sh"

echo "✅ Packages copied"
echo ""

# Step 3: Build ISO
echo "🔨 Building ISO..."
echo ""
cd "$SCRIPT_DIR"
sudo ./buildiso.sh -p desktop

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ ISO build complete!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "ISO location: $SCRIPT_DIR/out/"
ls -lh "$SCRIPT_DIR/out/"*.iso 2>/dev/null || echo "No ISO found"
echo ""
