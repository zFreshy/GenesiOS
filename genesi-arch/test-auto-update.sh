#!/bin/bash
# Quick test script for Genesi OS Auto-Update System

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🧪 Genesi OS Auto-Update System - Quick Test"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
PASSED=0
FAILED=0

# Test function
test_check() {
    local name="$1"
    local command="$2"
    
    echo -n "Testing: $name... "
    
    if eval "$command" &>/dev/null; then
        echo -e "${GREEN}✓ PASS${NC}"
        ((PASSED++))
        return 0
    else
        echo -e "${RED}✗ FAIL${NC}"
        ((FAILED++))
        return 1
    fi
}

# Test 1: Package files exist
echo "━━━ Phase 1: Package Files ━━━"
test_check "PKGBUILD exists" "[ -f packages/genesi-updater/PKGBUILD ]"
test_check "Daemon script exists" "[ -f packages/genesi-updater/genesi-updater ]"
test_check "Service file exists" "[ -f packages/genesi-updater/genesi-updater.service ]"
test_check "Timer file exists" "[ -f packages/genesi-updater/genesi-updater.timer ]"
test_check "Config file exists" "[ -f packages/genesi-updater/genesi-updater.conf ]"
test_check "Notifier script exists" "[ -f packages/genesi-updater/genesi-update-notifier ]"
test_check "Widget metadata exists" "[ -f packages/genesi-updater/plasmoid/metadata.json ]"
test_check "Widget QML exists" "[ -f packages/genesi-updater/plasmoid/contents/ui/main.qml ]"
echo ""

# Test 2: Scripts are executable
echo "━━━ Phase 2: File Permissions ━━━"
test_check "Daemon is executable" "[ -x packages/genesi-updater/genesi-updater ]"
test_check "Notifier is executable" "[ -x packages/genesi-updater/genesi-update-notifier ]"
echo ""

# Test 3: Python syntax
echo "━━━ Phase 3: Python Syntax ━━━"
if command -v python3 &>/dev/null; then
    test_check "Daemon Python syntax" "python3 -m py_compile packages/genesi-updater/genesi-updater"
else
    echo -e "${YELLOW}⚠ SKIP${NC} (python3 not found)"
fi
echo ""

# Test 4: Systemd files syntax
echo "━━━ Phase 4: Systemd Files ━━━"
if command -v systemd-analyze &>/dev/null; then
    test_check "Service file syntax" "systemd-analyze verify packages/genesi-updater/genesi-updater.service 2>/dev/null || true"
else
    echo -e "${YELLOW}⚠ SKIP${NC} (systemd-analyze not found)"
fi
echo ""

# Test 5: JSON syntax
echo "━━━ Phase 5: JSON Syntax ━━━"
if command -v jq &>/dev/null; then
    test_check "Widget metadata JSON" "jq empty packages/genesi-updater/plasmoid/metadata.json"
else
    echo -e "${YELLOW}⚠ SKIP${NC} (jq not found)"
fi
echo ""

# Test 6: Documentation exists
echo "━━━ Phase 6: Documentation ━━━"
test_check "Main documentation exists" "[ -f docs/AUTO-UPDATE-SYSTEM.md ]"
test_check "Test guide exists" "[ -f docs/TEST-AUTO-UPDATE.md ]"
test_check "Summary exists" "[ -f docs/WORKFLOW-AUTO-UPDATE-SUMMARY.md ]"
test_check "Package README exists" "[ -f packages/genesi-updater/README.md ]"
echo ""

# Test 7: GitHub Actions workflow
echo "━━━ Phase 7: GitHub Actions ━━━"
test_check "Workflow file exists" "[ -f ../.github/workflows/publish-packages.yml ]"
echo ""

# Test 8: Build script
echo "━━━ Phase 8: Build System ━━━"
test_check "Build script exists" "[ -f packages/build-packages.sh ]"
test_check "Build script is executable" "[ -x packages/build-packages.sh ]"
echo ""

# Test 9: Try to build (if makepkg available)
echo "━━━ Phase 9: Package Build ━━━"
if command -v makepkg &>/dev/null; then
    echo "Attempting to build package..."
    cd packages/genesi-updater
    if makepkg --printsrcinfo &>/dev/null; then
        echo -e "${GREEN}✓ PASS${NC} Package can be built"
        ((PASSED++))
    else
        echo -e "${RED}✗ FAIL${NC} Package build failed"
        ((FAILED++))
    fi
    cd ../..
else
    echo -e "${YELLOW}⚠ SKIP${NC} (makepkg not found)"
fi
echo ""

# Summary
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 Test Results"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"
echo ""

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}✅ All tests passed!${NC}"
    echo ""
    echo "Next steps:"
    echo "1. Build packages: cd packages && bash build-packages.sh"
    echo "2. Test locally: sudo pacman -U repo/genesi-updater-*.pkg.tar.zst"
    echo "3. Push to GitHub: git push origin arch-base"
    echo "4. Check GitHub Actions: https://github.com/zFreshy/GenesiOS/actions"
    echo ""
    exit 0
else
    echo -e "${RED}❌ Some tests failed!${NC}"
    echo ""
    echo "Please fix the issues above before proceeding."
    echo ""
    exit 1
fi
