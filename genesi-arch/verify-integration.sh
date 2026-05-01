#!/bin/bash
# Verificação rápida de integração do sistema de updates

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "🔍 Verificação de Integração - Genesi OS Auto-Update"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASS=0
FAIL=0

check() {
    local name="$1"
    local test="$2"
    
    echo -n "Verificando: $name... "
    
    if eval "$test" &>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        ((PASS++))
    else
        echo -e "${RED}✗${NC}"
        ((FAIL++))
    fi
}

echo "━━━ Arquivos de Pacote ━━━"
check "PKGBUILD existe" "[ -f packages/genesi-updater/PKGBUILD ]"
check "Daemon existe" "[ -f packages/genesi-updater/genesi-updater ]"
check "Widget existe" "[ -f packages/genesi-updater/plasmoid/metadata.json ]"
echo ""

echo "━━━ Integração na ISO ━━━"
check "genesi-updater em packages_desktop.x86_64" "grep -q 'genesi-updater' archiso/packages_desktop.x86_64"
check "Repositório configurado" "[ -f archiso/airootfs/etc/pacman.conf.d/genesi.conf ]"
check "GitHub Actions workflow existe" "[ -f ../.github/workflows/publish-packages.yml ]"
echo ""

echo "━━━ Verificação GitHub ━━━"
echo -n "Verificando release no GitHub... "
if curl -s -I "https://github.com/zFreshy/GenesiOS/releases/download/packages-latest/genesi.db.tar.gz" | grep -q "200 OK"; then
    echo -e "${GREEN}✓${NC} Release existe!"
    ((PASS++))
else
    echo -e "${YELLOW}⚠${NC} Release não encontrado (precisa criar)"
    echo "   Execute: git push origin arch-base"
    echo "   Ou crie manualmente em: https://github.com/zFreshy/GenesiOS/releases/new"
fi
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "📊 Resultado"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "Passou: ${GREEN}$PASS${NC}"
echo -e "Falhou: ${RED}$FAIL${NC}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}✅ Tudo pronto para buildar a ISO!${NC}"
    echo ""
    echo "Próximos passos:"
    echo "1. sudo ./buildiso.sh -p desktop"
    echo "2. Testar ISO em VM"
    echo "3. Verificar widgets e notificações"
    echo ""
else
    echo -e "${RED}❌ Alguns itens precisam de atenção${NC}"
    echo ""
    echo "Veja INTEGRATION-GUIDE.md para detalhes"
    echo ""
fi
