#!/bin/bash
# Debug script to build packages one by one and see where it fails

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PACKAGES_DIR="$SCRIPT_DIR/packages"
REPO_DIR="$SCRIPT_DIR/local-repo"

echo "🔍 DEBUG: Building Genesi packages one by one"
echo ""

# Create repo directory
mkdir -p "$REPO_DIR"

# Test each package individually
PACKAGES=(
    "genesi-settings"
    "genesi-kde-settings"
    "genesi-ai-mode"
    "genesi-updater"
)

for pkg in "${PACKAGES[@]}"; do
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Testing: $pkg"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    cd "$PACKAGES_DIR/$pkg"
    
    echo "📁 Package directory: $(pwd)"
    echo "📄 Files in directory:"
    ls -la
    echo ""
    
    echo "📋 PKGBUILD content:"
    cat PKGBUILD
    echo ""
    
    echo "🔨 Attempting to build..."
    if makepkg -sf --noconfirm 2>&1 | tee /tmp/build-$pkg.log; then
        echo "✅ SUCCESS: $pkg built"
        mv *.pkg.tar.zst "$REPO_DIR/" 2>/dev/null || true
    else
        echo "❌ FAILED: $pkg"
        echo "📝 Build log saved to: /tmp/build-$pkg.log"
        echo ""
        echo "Last 20 lines of error:"
        tail -20 /tmp/build-$pkg.log
        exit 1
    fi
    
    cd "$PACKAGES_DIR"
    echo ""
done

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Creating repository database..."
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

cd "$REPO_DIR"
rm -f genesi.db* genesi.files*
repo-add genesi.db.tar.gz *.pkg.tar.zst

# Create symlinks for pacman (it looks for .db not .db.tar.gz)
ln -sf genesi.db.tar.gz genesi.db
ln -sf genesi.files.tar.gz genesi.files

echo ""
echo "✅ All packages built successfully!"
ls -lh "$REPO_DIR"
