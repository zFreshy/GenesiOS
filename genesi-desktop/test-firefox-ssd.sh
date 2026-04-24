#!/bin/bash
# Script de teste para verificar se o Firefox respeita Server-Side Decoration (SSD)
# no Genesi OS

set -e

echo "=== Genesi OS - Teste de SSD no Firefox ==="
echo ""

# 1. Compila o nocsd.so se não existir
NOCSD_SO="/tmp/genesi_nocsd.so"
NOCSD_C="$(dirname "$0")/genesi-wm/nocsd.c"

if [ ! -f "$NOCSD_SO" ] || [ "$NOCSD_C" -nt "$NOCSD_SO" ]; then
    echo "📦 Compilando nocsd.so..."
    cc -shared -fPIC -ldl -o "$NOCSD_SO" "$NOCSD_C"
    echo "✓ nocsd.so compilado em $NOCSD_SO"
    echo "$NOCSD_SO" > /tmp/genesi-nocsd-path.txt
else
    echo "✓ nocsd.so já existe em $NOCSD_SO"
fi

echo ""

# 2. Mata qualquer instância do Firefox rodando
echo "🔪 Matando instâncias antigas do Firefox..."
pkill -9 firefox || true
sleep 1

# 3. Cria profile temporário limpo
PROFILE_DIR="/tmp/genesi-firefox-profile-$(date +%s)"
mkdir -p "$PROFILE_DIR"
echo "✓ Profile temporário criado em $PROFILE_DIR"

echo ""
echo "🚀 Lançando Firefox com SSD forçado..."
echo ""
echo "Variáveis de ambiente ativas:"
echo "  WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-wayland-1}"
echo "  MOZ_ENABLE_WAYLAND=1"
echo "  MOZ_GTK_TITLEBAR_DECORATION=system"
echo "  MOZ_DISABLE_CONTENT_SANDBOX=1"
echo "  GTK_CSD=0"
echo "  GDK_BACKEND=wayland"
echo "  LIBDECOR_PLUGIN_DIR=/dev/null"
echo "  LD_PRELOAD=$NOCSD_SO"
echo ""

# 4. Lança o Firefox com todas as proteções
env \
    WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-wayland-1}" \
    MOZ_ENABLE_WAYLAND=1 \
    MOZ_GTK_TITLEBAR_DECORATION=system \
    MOZ_DISABLE_CONTENT_SANDBOX=1 \
    MOZ_X11_EGL=0 \
    GTK_CSD=0 \
    GDK_BACKEND=wayland \
    LIBDECOR_PLUGIN_DIR=/dev/null \
    LD_PRELOAD="$NOCSD_SO" \
    firefox \
        --new-instance \
        --profile "$PROFILE_DIR" \
        --new-window \
        "about:support" \
    2>&1 | grep -i "decoration\|csd\|libdecor" || true

echo ""
echo "✓ Firefox lançado!"
echo ""
echo "VERIFICAÇÃO:"
echo "  1. O Firefox deve ter APENAS UMA topbar (a do Genesi OS)"
echo "  2. Não deve ter a topbar preta/escura do próprio Firefox"
echo "  3. Em about:support, procure por 'Window Protocol: wayland'"
echo ""
echo "Se ainda aparecer duas topbars, verifique:"
echo "  - Se o compositor Wayland está rodando (genesi-wm)"
echo "  - Se xdg-decoration está habilitado no compositor"
echo "  - Logs do stderr acima para mensagens de erro"
