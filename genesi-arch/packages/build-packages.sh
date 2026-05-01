#!/usr/bin/env bash
# Build all Genesi OS packages

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$SCRIPT_DIR/repo"

echo "=== Building Genesi OS Packages ==="
echo ""

# Create repo directory
mkdir -p "$REPO_DIR"

# List of packages to build
PACKAGES=(
    "genesi-settings"
    "genesi-ai-mode"
    "genesi-kde-settings"
)

# Build each package
for pkg in "${PACKAGES[@]}"; do
    echo ">>> Building $pkg..."
    cd "$SCRIPT_DIR/$pkg"
    
    # Clean previous builds
    rm -f *.pkg.tar.zst
    
    # Build package
    makepkg -sf --noconfirm
    
    # Move to repo
    mv *.pkg.tar.zst "$REPO_DIR/"
    
    echo "✓ $pkg built successfully"
    echo ""
done

# Create repository database
echo ">>> Creating repository database..."
cd "$REPO_DIR"
repo-add genesi.db.tar.gz *.pkg.tar.zst

echo ""
echo "=== Build Complete ==="
echo "Packages location: $REPO_DIR"
echo "Repository database: $REPO_DIR/genesi.db.tar.gz"
echo ""
echo "To use this repository, add to /etc/pacman.conf:"
echo ""
echo "[genesi]"
echo "SigLevel = Optional TrustAll"
echo "Server = file://$REPO_DIR"
echo ""
