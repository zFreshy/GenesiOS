#!/bin/bash
# Teste completo do Genesi OS no TTY (simula ambiente da ISO)

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Teste Genesi OS (Modo TTY - Como ISO)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Verifica se está em TTY
if [ -n "$DISPLAY" ] || [ -n "$WAYLAND_DISPLAY" ]; then
    echo "⚠️  AVISO: Você está em um ambiente gráfico!"
    echo ""
    echo "Para teste completo (como na ISO):"
    echo "  1. Saia do ambiente gráfico:"
    echo "     sudo systemctl stop gdm3  # ou lightdm/sddm"
    echo "  2. Vá para TTY1 (Ctrl+Alt+F1)"
    echo "  3. Execute este script novamente"
    echo ""
    read -p "Continuar mesmo assim? (s/N) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Ss]$ ]]; then
        exit 1
    fi
fi

# Verifica se os binários existem
if [ ! -f "genesi-desktop/genesi-wm/target/release/genesi-wm" ]; then
    echo "❌ WM não compilado. Execute primeiro:"
    echo "   bash test-genesi-local.sh"
    exit 1
fi

if [ ! -f "genesi-desktop/src-tauri/target/release/genesi-desktop" ]; then
    echo "❌ Desktop não compilado. Execute primeiro:"
    echo "   bash test-genesi-local.sh"
    exit 1
fi

# Configura variáveis
export XDG_RUNTIME_DIR=/run/user/$(id -u)
export MOZ_ENABLE_WAYLAND=1
export GTK_CSD=0
export LIBDECOR_PLUGIN_DIR=/dev/null
export DISPLAY=""
export WAYLAND_DISPLAY=""

mkdir -p "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"

LOG_FILE="/tmp/genesi-tty-test.log"
echo "=== Genesi OS TTY Test $(date) ===" > "$LOG_FILE"
echo "Log: $LOG_FILE"
echo ""

# Limpa sockets antigos
rm -f "$XDG_RUNTIME_DIR/wayland-"*

# Inicia Weston no DRM (acesso direto ao hardware)
echo "🚀 Iniciando Weston (DRM backend)..."
weston --backend=drm-backend.so --tty=1 >> "$LOG_FILE" 2>&1 &
WESTON_PID=$!
echo "   PID: $WESTON_PID"

sleep 5

if ! kill -0 $WESTON_PID 2>/dev/null; then
    echo "❌ Weston falhou ao iniciar"
    echo ""
    echo "Possíveis causas:"
    echo "  - Não está em TTY (precisa sair do ambiente gráfico)"
    echo "  - Sem permissão para acessar DRM"
    echo "  - Outro compositor já rodando"
    echo ""
    cat "$LOG_FILE"
    exit 1
fi

# Verifica socket
if [ ! -S "$XDG_RUNTIME_DIR/wayland-0" ]; then
    echo "❌ Socket do Weston não encontrado"
    kill $WESTON_PID 2>/dev/null
    cat "$LOG_FILE"
    exit 1
fi

echo "✅ Weston iniciado em wayland-0"
echo ""

# Inicia Genesi WM dentro do Weston
echo "🚀 Iniciando Genesi WM..."
cd genesi-desktop/genesi-wm
WAYLAND_DISPLAY=wayland-0 ./target/release/genesi-wm >> "$LOG_FILE" 2>&1 &
WM_PID=$!
echo "   PID: $WM_PID"
cd ../..

sleep 5

if ! kill -0 $WM_PID 2>/dev/null; then
    echo "❌ WM falhou ao iniciar"
    cat "$LOG_FILE"
    kill $WESTON_PID 2>/dev/null
    exit 1
fi

# Verifica socket do WM
if [ ! -S "$XDG_RUNTIME_DIR/wayland-1" ]; then
    echo "❌ Socket do WM não encontrado"
    echo ""
    echo "Sockets disponíveis:"
    ls -la "$XDG_RUNTIME_DIR"/wayland-* 2>/dev/null || echo "  Nenhum"
    echo ""
    kill $WM_PID 2>/dev/null
    kill $WESTON_PID 2>/dev/null
    cat "$LOG_FILE"
    exit 1
fi

echo "✅ Genesi WM iniciado em wayland-1"
echo ""

# Inicia Genesi Desktop
echo "🚀 Iniciando Genesi Desktop..."
cd genesi-desktop/src-tauri/target/release

WAYLAND_DISPLAY=wayland-1 \
XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
GDK_BACKEND=wayland \
QT_QPA_PLATFORM=wayland \
SDL_VIDEODRIVER=wayland \
CLUTTER_BACKEND=wayland \
MOZ_ENABLE_WAYLAND=1 \
DISPLAY="" \
./genesi-desktop >> "$LOG_FILE" 2>&1

# Desktop fechou
echo ""
echo "Desktop fechou, limpando..."
kill $WM_PID 2>/dev/null
kill $WESTON_PID 2>/dev/null

cd ../../../..

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Log do Teste"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cat "$LOG_FILE"
echo ""
echo "Teste concluído!"
