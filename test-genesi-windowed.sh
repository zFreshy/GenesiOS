#!/bin/bash
# Teste rápido do Genesi OS em modo janela (dentro do ambiente gráfico atual)

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Teste Genesi OS (Modo Janela)"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

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
export XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR:-/run/user/$(id -u)}
export MOZ_ENABLE_WAYLAND=1
export GTK_CSD=0
export LIBDECOR_PLUGIN_DIR=/dev/null

mkdir -p "$XDG_RUNTIME_DIR"
chmod 700 "$XDG_RUNTIME_DIR"

LOG_FILE="/tmp/genesi-test-$(date +%s).log"
echo "=== Genesi OS Test $(date) ===" > "$LOG_FILE"
echo "Log: $LOG_FILE"
echo ""

# Limpa sockets antigos
rm -f "$XDG_RUNTIME_DIR/wayland-test-"*

# Inicia Weston em modo janela (roda dentro do ambiente gráfico atual)
echo "🚀 Iniciando Weston (modo janela)..."
WAYLAND_DISPLAY=wayland-test-0 weston --width=1280 --height=720 >> "$LOG_FILE" 2>&1 &
WESTON_PID=$!
echo "   PID: $WESTON_PID"

sleep 3

if ! kill -0 $WESTON_PID 2>/dev/null; then
    echo "❌ Weston falhou ao iniciar"
    cat "$LOG_FILE"
    exit 1
fi

# Verifica socket
if [ ! -S "$XDG_RUNTIME_DIR/wayland-test-0" ]; then
    echo "❌ Socket do Weston não encontrado"
    kill $WESTON_PID 2>/dev/null
    exit 1
fi

echo "✅ Weston iniciado"
echo ""

# Inicia Genesi WM dentro do Weston
echo "🚀 Iniciando Genesi WM..."
cd genesi-desktop/genesi-wm
WAYLAND_DISPLAY=wayland-test-0 ./target/release/genesi-wm >> "$LOG_FILE" 2>&1 &
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
if [ ! -S "$XDG_RUNTIME_DIR/wayland-test-1" ]; then
    echo "❌ Socket do WM não encontrado"
    echo ""
    echo "Sockets disponíveis:"
    ls -la "$XDG_RUNTIME_DIR"/wayland-* 2>/dev/null || echo "  Nenhum"
    echo ""
    kill $WM_PID 2>/dev/null
    kill $WESTON_PID 2>/dev/null
    exit 1
fi

echo "✅ Genesi WM iniciado"
echo ""

# Inicia Genesi Desktop
echo "🚀 Iniciando Genesi Desktop..."
cd genesi-desktop/src-tauri/target/release

WAYLAND_DISPLAY=wayland-test-1 \
XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
GDK_BACKEND=wayland \
QT_QPA_PLATFORM=wayland \
SDL_VIDEODRIVER=wayland \
CLUTTER_BACKEND=wayland \
MOZ_ENABLE_WAYLAND=1 \
DISPLAY="" \
./genesi-desktop >> "$LOG_FILE" 2>&1 &

DESKTOP_PID=$!
echo "   PID: $DESKTOP_PID"
cd ../../../..

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  ✅ Genesi OS Rodando!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Processos:"
echo "  Weston:  $WESTON_PID"
echo "  WM:      $WM_PID"
echo "  Desktop: $DESKTOP_PID"
echo ""
echo "Log: $LOG_FILE"
echo ""
echo "Para parar, pressione Ctrl+C ou execute:"
echo "  kill $DESKTOP_PID $WM_PID $WESTON_PID"
echo ""
echo "Aguardando Desktop fechar..."

# Aguarda o Desktop fechar
wait $DESKTOP_PID 2>/dev/null

echo ""
echo "Desktop fechado, limpando..."
kill $WM_PID 2>/dev/null
kill $WESTON_PID 2>/dev/null

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Log do Teste"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cat "$LOG_FILE"
