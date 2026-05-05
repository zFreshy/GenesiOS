#!/bin/bash
# Script para configurar o genesi-calamares-config como submodule

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🔧 Genesi OS - Setup Calamares Config Submodule"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Check if genesi-calamares-config-full exists
if [ ! -d "genesi-calamares-config-full" ]; then
    echo "❌ ERROR: genesi-calamares-config-full directory not found!"
    echo "   Please make sure you're in the Genesi project root directory."
    exit 1
fi

# Check if it's a git repository
if [ ! -d "genesi-calamares-config-full/.git" ]; then
    echo "❌ ERROR: genesi-calamares-config-full is not a git repository!"
    echo "   Please run the git init and commit commands first."
    exit 1
fi

echo "📋 Step 1: Checking if GitHub repository exists..."
echo ""
echo "⚠️  IMPORTANT: Before continuing, you need to:"
echo "   1. Go to https://github.com/zFreshy"
echo "   2. Create a new repository named: genesi-calamares-config"
echo "   3. Make it PUBLIC"
echo "   4. Do NOT initialize with README, .gitignore, or license"
echo ""
read -p "Have you created the repository? (y/n) " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "❌ Aborted. Please create the repository first."
    exit 1
fi

echo ""
echo "📤 Step 2: Pushing to GitHub..."
cd genesi-calamares-config-full

# Add remote if not exists
if ! git remote get-url origin &> /dev/null; then
    git remote add origin https://github.com/zFreshy/genesi-calamares-config.git
    echo "✅ Remote added"
else
    echo "✅ Remote already exists"
fi

# Rename branch to main
git branch -M main

# Push
echo "Pushing to GitHub..."
git push -u origin main

if [ $? -eq 0 ]; then
    echo "✅ Successfully pushed to GitHub!"
else
    echo "❌ Failed to push to GitHub"
    echo "   Please check your credentials and try again"
    exit 1
fi

cd ..

echo ""
echo "🔄 Step 3: Removing old CachyOS submodule..."

# Remove cachyos-calamares-config submodule if exists
if [ -d "cachyos-calamares-config" ]; then
    git submodule deinit -f cachyos-calamares-config
    git rm -f cachyos-calamares-config
    rm -rf .git/modules/cachyos-calamares-config
    echo "✅ Old submodule removed"
else
    echo "✅ Old submodule not found (already removed)"
fi

echo ""
echo "🔄 Step 4: Removing temporary directory..."
rm -rf genesi-calamares-config-full
echo "✅ Temporary directory removed"

echo ""
echo "➕ Step 5: Adding Genesi calamares-config as submodule..."
git submodule add https://github.com/zFreshy/genesi-calamares-config.git genesi-calamares-config-full

if [ $? -eq 0 ]; then
    echo "✅ Submodule added successfully!"
else
    echo "❌ Failed to add submodule"
    exit 1
fi

echo ""
echo "💾 Step 6: Committing changes..."
git add .gitmodules genesi-calamares-config-full
git commit -m "Replace CachyOS calamares-config with Genesi calamares-config"

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Setup complete!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "📝 Next steps:"
echo "   1. Push the changes: git push"
echo "   2. Build the ISO: cd genesi-arch && bash prepare-and-build.sh"
echo ""
echo "🎉 Your Genesi OS now has its own Calamares configuration!"
echo ""
