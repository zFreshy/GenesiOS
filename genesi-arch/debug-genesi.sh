#!/bin/bash
# Genesi OS Debug Script
# Run this in the live ISO to check everything

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🔍 Genesi OS Debug Information"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

echo "📋 1. SYSTEM INFORMATION"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cat /etc/os-release
echo ""

echo "📋 2. PACMAN REPOSITORIES"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
grep -A 2 "^\[.*\]" /etc/pacman.conf | grep -E "^\[|^Server"
echo ""

echo "📋 3. GENESI PACKAGES INSTALLED"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
pacman -Q | grep genesi || echo "❌ No Genesi packages installed"
echo ""

echo "📋 4. CALAMARES PACKAGES INSTALLED"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
pacman -Q | grep calamares || echo "❌ No Calamares packages installed"
echo ""

echo "📋 5. GENESI PACKAGES IN REPOSITORIES"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
pacman -Ss genesi || echo "❌ No Genesi packages in repositories"
echo ""

echo "📋 6. /opt/genesi-packages DIRECTORY"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -d /opt/genesi-packages ]; then
    echo "✅ Directory exists"
    ls -lah /opt/genesi-packages/
    echo ""
    echo "Database files:"
    ls -lah /opt/genesi-packages/*.db* /opt/genesi-packages/*.files* 2>/dev/null || echo "❌ No database files"
else
    echo "❌ Directory does NOT exist"
fi
echo ""

echo "📋 7. GENESI SKEL-OVERRIDE"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -d /usr/share/genesi/skel-override ]; then
    echo "✅ Directory exists"
    echo "Files (first 20):"
    find /usr/share/genesi/skel-override -type f | head -20
else
    echo "❌ Directory does NOT exist"
fi
echo ""

echo "📋 8. LIVEUSER HOME DIRECTORY"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -d /home/liveuser ]; then
    echo "✅ Directory exists"
    echo "Owner: $(stat -c '%U:%G' /home/liveuser)"
    echo ""
    echo "KDE config files:"
    ls -lah /home/liveuser/.config/kwin* /home/liveuser/.config/kdeglobals /home/liveuser/.config/plasma* 2>/dev/null || echo "❌ No KDE config files"
else
    echo "❌ Directory does NOT exist"
fi
echo ""

echo "📋 9. CALAMARES CONFIGURATION"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -f /etc/calamares/settings.conf ]; then
    echo "✅ /etc/calamares/settings.conf exists"
    echo "Branding:"
    grep "branding:" /etc/calamares/settings.conf || echo "Not found"
else
    echo "❌ /etc/calamares/settings.conf does NOT exist"
fi
echo ""

if [ -d /etc/calamares/branding/genesi ]; then
    echo "✅ /etc/calamares/branding/genesi exists"
    ls -lah /etc/calamares/branding/genesi/
else
    echo "❌ /etc/calamares/branding/genesi does NOT exist"
fi
echo ""

echo "📋 10. CALAMARES MODULES"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -d /etc/calamares/modules ]; then
    echo "✅ /etc/calamares/modules exists"
    echo "Module count: $(ls /etc/calamares/modules/*.conf 2>/dev/null | wc -l)"
    echo ""
    echo "Key modules:"
    ls -lh /etc/calamares/modules/welcome*.conf /etc/calamares/modules/pacstrap.conf /etc/calamares/modules/bootloader.conf 2>/dev/null || echo "Some modules missing"
else
    echo "❌ /etc/calamares/modules does NOT exist"
fi
echo ""

echo "📋 11. CUSTOMIZATION LOG"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -f /var/log/genesi-customize.log ]; then
    echo "✅ Log exists"
    echo "Last 30 lines:"
    tail -30 /var/log/genesi-customize.log
else
    echo "❌ Log does NOT exist"
fi
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "✅ Debug information collected"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
