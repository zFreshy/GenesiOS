#!/usr/bin/env bash
# Test script to verify Phase 2 (AI Mode) files are in place

echo "=== Genesi OS Phase 2 (AI Mode) - Pre-Build Verification ==="
echo ""

ERRORS=0

# Check daemon
if [ -f "archiso/airootfs/usr/local/bin/genesi-aid" ]; then
    echo "✅ genesi-aid daemon found"
else
    echo "❌ genesi-aid daemon NOT FOUND"
    ERRORS=$((ERRORS + 1))
fi

# Check systemd service
if [ -f "archiso/airootfs/usr/lib/systemd/system/genesi-aid.service" ]; then
    echo "✅ genesi-aid.service found"
else
    echo "❌ genesi-aid.service NOT FOUND"
    ERRORS=$((ERRORS + 1))
fi

# Check sysctl config
if [ -f "archiso/airootfs/etc/sysctl.d/99-genesi-ai.conf" ]; then
    echo "✅ sysctl AI config found"
else
    echo "❌ sysctl AI config NOT FOUND"
    ERRORS=$((ERRORS + 1))
fi

# Check Plasma widget
if [ -f "archiso/airootfs/usr/share/plasma/plasmoids/org.genesi.aimode/metadata.json" ]; then
    echo "✅ AI Mode widget metadata found"
else
    echo "❌ AI Mode widget metadata NOT FOUND"
    ERRORS=$((ERRORS + 1))
fi

if [ -f "archiso/airootfs/usr/share/plasma/plasmoids/org.genesi.aimode/contents/ui/main.qml" ]; then
    echo "✅ AI Mode widget QML found"
else
    echo "❌ AI Mode widget QML NOT FOUND"
    ERRORS=$((ERRORS + 1))
fi

# Check profiledef.sh has genesi-aid permission
if grep -q "genesi-aid" "archiso/profiledef.sh"; then
    echo "✅ genesi-aid permission in profiledef.sh"
else
    echo "❌ genesi-aid permission NOT in profiledef.sh"
    ERRORS=$((ERRORS + 1))
fi

# Check python-psutil in packages
if grep -q "python-psutil" "archiso/packages_desktop.x86_64"; then
    echo "✅ python-psutil in packages"
else
    echo "❌ python-psutil NOT in packages"
    ERRORS=$((ERRORS + 1))
fi

# Check customize_airootfs.sh enables the service
if grep -q "genesi-aid.service" "archiso/airootfs/root/customize_airootfs.sh"; then
    echo "✅ genesi-aid.service enabled in customize_airootfs.sh"
else
    echo "❌ genesi-aid.service NOT enabled in customize_airootfs.sh"
    ERRORS=$((ERRORS + 1))
fi

# Check documentation
if [ -f "docs/PHASE2-AI-MODE.md" ]; then
    echo "✅ Phase 2 documentation found"
else
    echo "❌ Phase 2 documentation NOT FOUND"
    ERRORS=$((ERRORS + 1))
fi

echo ""
echo "=== Summary ==="
if [ $ERRORS -eq 0 ]; then
    echo "✅ All Phase 2 files are in place! Ready to build."
    echo ""
    echo "Next steps:"
    echo "1. Build the ISO: sudo ./buildiso.sh -p desktop"
    echo "2. Boot in VirtualBox"
    echo "3. Check daemon: sudo systemctl status genesi-aid"
    echo "4. Install Ollama: curl -fsSL https://ollama.ai/install.sh | sh"
    echo "5. Run AI: ollama pull llama3.2 && ollama run llama3.2"
    echo "6. Watch logs: sudo journalctl -u genesi-aid -f"
    echo ""
    echo "See docs/PHASE2-AI-MODE.md for detailed testing guide."
    exit 0
else
    echo "❌ Found $ERRORS error(s). Please fix before building."
    exit 1
fi
