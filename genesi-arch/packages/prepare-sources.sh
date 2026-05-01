#!/usr/bin/env bash
# Prepare source files for genesi-kde-settings package
# This script creates symlinks to avoid duplicating files

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
KDE_PKG="$SCRIPT_DIR/genesi-kde-settings"

echo "=== Preparing genesi-kde-settings sources ==="

# Create symlinks for makepkg
cd "$KDE_PKG"

# Link etc directory
if [ ! -L "etc" ]; then
    ln -sf "$(pwd)/etc" etc
    echo "✓ Linked etc/"
fi

# Link usr directory
if [ ! -L "usr" ]; then
    ln -sf "$(pwd)/usr" usr
    echo "✓ Linked usr/"
fi

echo ""
echo "=== Sources prepared ==="
echo "You can now run: cd genesi-kde-settings && makepkg -sf"
