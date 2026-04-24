#!/bin/bash
# Script para parar completamente o Genesi OS
# Uso: bash stop-genesi.sh

echo ""
echo "🛑 Parando Genesi OS..."
echo ""

# 1. Mata processos do Desktop Environment
echo "[1/3] Parando Desktop Environment..."
pkill -9 genesi-desktop 2>/dev/null && echo "      ✓ Desktop parado" || echo "      ⚠ Desktop não estava rodando"

# 2. Mata processos do Window Manager
echo "[2/3] Parando Window Manager..."
pkill -9 genesi-wm 2>/dev/null && echo "      ✓ Window Manager parado" || echo "      ⚠ Window Manager não estava rodando"
pkill -9 cargo 2>/dev/null || true

# 3. Mata processos Node/NPM relacionados
echo "[3/3] Limpando processos Node..."
pkill -9 -f "tauri dev" 2>/dev/null || true
pkill -9 -f "vite" 2>/dev/null || true

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
else
    echo "✓ Nenhum processo relacionado rodando"
fi

echo ""
