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

# Step 2: Copy packages to airootfs (will be included in the ISO)
echo ""
echo "📋 Copying packages to airootfs..."
cp "$SCRIPT_DIR/local-repo/"*.pkg.tar.zst "$SCRIPT_DIR/archiso/airootfs/opt/genesi-packages/"

echo "✅ Packages copied to airootfs/opt/genesi-packages/"
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
