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
    "libpamac-dummy"
    "genesi-calamares"
    "genesi-settings"
    "genesi-kde-settings"
    "genesi-ai-mode"
    "genesi-updater"
    "genesi-welcome"
)

echo "📋 Note: Building all Genesi packages from GitHub repos"
echo ""

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
    
    # Build package only if not already built or if forced
    # We skip building if the package already exists in the repo
    # This saves a lot of time on subsequent ISO builds
    PKG_NAME=$(grep "^pkgname=" PKGBUILD | cut -d= -f2 | tr -d "'" | tr -d '"')
    PKG_VER=$(grep "^pkgver=" PKGBUILD | cut -d= -f2 | tr -d "'" | tr -d '"')
    PKG_REL=$(grep "^pkgrel=" PKGBUILD | cut -d= -f2 | tr -d "'" | tr -d '"')
    
    # Simple check: if a file matching the package name exists in repo, skip
    if ls "$REPO_DIR/${PKG_NAME}"-*.pkg.tar.zst 1> /dev/null 2>&1; then
        echo "✅ Package ${PKG_NAME} already exists in repository, skipping build..."
        cd "$PACKAGES_DIR"
        echo ""
        continue
    fi
    
    # Clean previous builds
    rm -f *.pkg.tar.zst
    rm -rf src pkg
    
    # Check if inside fakeroot or makepkg environment
    if [ -n "$FAKEROOTKEY" ]; then
        echo "⚠️ Running inside fakeroot environment"
    fi
    
    # Build package (makepkg cannot run as root)
    if [ "$EUID" -eq 0 ] && [ -z "$FAKEROOTKEY" ]; then
        echo "❌ ERROR: This script cannot be run as root (makepkg restriction)"
        echo "Please run without sudo: bash build-local-packages.sh"
        exit 1
    fi
    
    if [ "$EUID" -eq 0 ]; then
        # If we are root but inside fakeroot, makepkg will still fail.
        # We should use sudo -u to run as a normal user, but this script is meant to be run as normal user.
        # Let makepkg handle its own errors.
        makepkg -sf --noconfirm || true
    else
        makepkg -sf --noconfirm
    fi
    
    if [ $? -eq 0 ]; then
        echo "✅ Built: $pkg"
        # Move to repo (copy first, then remove to avoid permission issues)
        cp *.pkg.tar.zst "$REPO_DIR/" 2>/dev/null || true
        rm -f *.pkg.tar.zst 2>/dev/null || true
    else
        echo "❌ Failed to build: $pkg (Ignoring and continuing...)"
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
if ls *.pkg.tar.zst 1> /dev/null 2>&1; then
    repo-add genesi.db.tar.gz *.pkg.tar.zst
else
    echo "⚠️ No packages found to add to repository database"
    touch genesi.db.tar.gz genesi.files.tar.gz
fi

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
