#!/bin/bash
# Prepare local repository and build ISO

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🚀 Genesi OS - Prepare and Build ISO"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    echo "❌ ERROR: Do not run this script with sudo!"
    echo ""
    echo "This script will:"
    echo "  1. Build packages as normal user (makepkg requirement)"
    echo "  2. Ask for sudo password only when building ISO"
    echo ""
    echo "Please run: bash prepare-and-build.sh"
    exit 1
fi

# Step 1: Build packages if not already built
# ENABLED: We MUST build local packages because our custom packages (genesi-welcome, etc) are not in CachyOS repos
echo "📦 Building Genesi packages..."
bash "$SCRIPT_DIR/build-local-packages.sh"

# Step 2: Copy Genesi Calamares config to airootfs
echo ""
echo "📋 Copying Genesi Calamares config to airootfs..."

# Copy the entire genesi-calamares-config-full submodule to airootfs/root
sudo mkdir -p "$SCRIPT_DIR/archiso/airootfs/root/"
sudo rm -rf "$SCRIPT_DIR/archiso/airootfs/root/genesi-calamares-config-full"
sudo cp -r "../genesi-calamares-config-full" "$SCRIPT_DIR/archiso/airootfs/root/"

echo "✅ Genesi Calamares config copied"
echo ""

# Step 3: Copy packages to airootfs (will be included in the ISO)
# ENABLED: We need to copy local packages so mkarchiso can install them
echo "📋 Copying packages to airootfs..."
sudo mkdir -p "$SCRIPT_DIR/archiso/airootfs/opt/genesi-packages/"
sudo chmod 755 "$SCRIPT_DIR/archiso/airootfs/opt/genesi-packages/"

# Copy packages with sudo
sudo cp "$SCRIPT_DIR/local-repo/"*.pkg.tar.zst "$SCRIPT_DIR/archiso/airootfs/opt/genesi-packages/" || true
sudo cp -a "$SCRIPT_DIR/local-repo/"genesi.db* "$SCRIPT_DIR/archiso/airootfs/opt/genesi-packages/" || true
sudo cp -a "$SCRIPT_DIR/local-repo/"genesi.files* "$SCRIPT_DIR/archiso/airootfs/opt/genesi-packages/" || true
echo ""

# Step 4: Build ISO (this will ask for sudo password)
echo "🔨 Building ISO (will ask for sudo password)..."
echo ""
cd "$SCRIPT_DIR"

# Create logs directory
mkdir -p "$SCRIPT_DIR/logs"
LOG_FILE="$SCRIPT_DIR/logs/build-$(date +%Y%m%d-%H%M%S).log"

echo "📝 Saving build output to: $LOG_FILE"
echo ""

# Run buildiso and capture ALL output (stdout + stderr)
sudo ./buildiso.sh -p desktop 2>&1 | tee "$LOG_FILE"

# Check if build succeeded
if [ ${PIPESTATUS[0]} -eq 0 ]; then
    BUILD_SUCCESS=true
else
    BUILD_SUCCESS=false
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

if [ "$BUILD_SUCCESS" = true ]; then
    echo "✅ ISO build complete!"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "ISO location: $SCRIPT_DIR/out/"
    ls -lh "$SCRIPT_DIR/out/"*.iso 2>/dev/null || echo "No ISO found"
else
    echo "❌ ISO build FAILED!"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo ""
    echo "❌ Build failed! Check the log for details:"
    echo "   $LOG_FILE"
    echo ""
    echo "Last 50 lines of log:"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    tail -50 "$LOG_FILE"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    exit 1
fi

echo ""
echo "📝 Full build log saved to: $LOG_FILE"
echo ""
