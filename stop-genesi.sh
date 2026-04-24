#!/bin/bash
# Script para parar completamente o Genesi OS
# Uso: bash stop-genesi.sh

echo ""
echo "🛑 Parando Genesi OS..."
echo ""

# 1. Mata processos do Desktop Environment
echo "[1/4] Parando Desktop Environment..."
pkill -9 genesi-desktop 2>/dev/null && echo "      ✓ Desktop parado" || echo "      ⚠ Desktop não estava rodando"

# 2. Mata processos do Window Manager
echo "[2/4] Parando Window Manager..."
pkill -9 genesi-wm 2>/dev/null && echo "      ✓ Window Manager parado" || echo "      ⚠ Window Manager não estava rodando"
pkill -9 cargo 2>/dev/null || true

# 3. Mata processos Node/NPM relacionados
echo "[3/4] Limpando processos Node..."
pkill -9 -f "tauri dev" 2>/dev/null || true
pkill -9 -f "vite" 2>/dev/null || true
pkill -9 node 2>/dev/null || true

# 4. Libera porta 1420 (porta do Tauri)
echo "[4/4] Liberando porta 1420..."
lsof -ti:1420 2>/dev/null | xargs kill -9 2>/dev/null && echo "      ✓ Porta 1420 liberada" || echo "      ⚠ Porta já estava livre"

echo ""
echo "✅ Genesi OS parado!"
echo ""

# Verifica se ainda tem processos rodando
REMAINING=$(ps aux | grep -E "genesi|tauri|cargo.*genesi" | grep -v grep | grep -v stop-genesi)

if [ ! -z "$REMAINING" ]; then
    echo "⚠ Alguns processos ainda estão rodando:"
    echo "$REMAINING"
    echo ""
    echo "💡 Para forçar o fechamento completo:"
    echo "   pkill -9 genesi-wm"
    echo "   pkill -9 genesi-desktop"
    echo "   lsof -ti:1420 | xargs kill -9"
else
    echo "✓ Nenhum processo relacionado rodando"
fi

echo ""
