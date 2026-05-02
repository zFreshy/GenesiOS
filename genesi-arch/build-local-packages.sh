#!/bin/bash
# Build Genesi packages locally and add to local repository

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGES_DIR="$SCRIPT_DIR/packages"
REPO_DIR="$SCRIPT_DIR/local-repo"

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🔨 Building Genesi OS Packages Locally"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Create repo directory with proper permissions
mkdir -p "$REPO_DIR"
chmod 755 "$REPO_DIR"

# List of packages to build
PACKAGES=(
    "genesi-settings"
    "genesi-kde-settings"
    "genesi-ai-mode"
    "genesi-updater"
    "genesi-calamares-branding"
)

# Build each package
for pkg in "${PACKAGES[@]}"; do
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Building: $pkg"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    if [ ! -d "$PACKAGES_DIR/$pkg" ]; then
        echo "❌ Package directory not found: $pkg"
        exit 1
    fi
    
    cd "$PACKAGES_DIR/$pkg"
    
    # Clean previous builds
    rm -f *.pkg.tar.zst
    
    # Build package (makepkg cannot run as root)
    if [ "$EUID" -eq 0 ]; then
        echo "❌ ERROR: This script cannot be run as root (makepkg restriction)"
        echo "Please run without sudo: bash build-local-packages.sh"
        exit 1
    fi
    
    makepkg -sf --noconfirm
    
    if [ $? -eq 0 ]; then
        echo "✅ Built: $pkg"
        # Move to repo (copy first, then remove to avoid permission issues)
        cp *.pkg.tar.zst "$REPO_DIR/" || {
            echo "❌ Failed to copy package to repo"
            exit 1
        }
        rm -f *.pkg.tar.zst
    else
        echo "❌ Failed to build: $pkg"
        exit 1
    fi
    
    cd "$PACKAGES_DIR"
    echo ""
done

# Create repository database
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Creating repository database..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

cd "$REPO_DIR"

# Remove old database
rm -f genesi.db* genesi.files*

# Create new database
repo-add genesi.db.tar.gz *.pkg.tar.zst

# Create symlinks for pacman (it looks for .db not .db.tar.gz)
ln -sf genesi.db.tar.gz genesi.db
ln -sf genesi.files.tar.gz genesi.files

echo ""
echo "✅ Repository database created!"
echo ""
echo "=== Built Packages ==="
ls -lh "$REPO_DIR"/*.pkg.tar.zst
echo ""
echo "=== Repository Files ==="
ls -lh "$REPO_DIR"/genesi.*
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ All packages built successfully!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Local repository: $REPO_DIR"
echo ""
echo "Next step: Run buildiso.sh to build the ISO with these packages"
echo ""
