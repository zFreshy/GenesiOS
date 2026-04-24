#!/bin/bash
# Script para rodar o Genesi OS completo
# Uso: bash run-genesi.sh

set -e

echo "🚀 Iniciando Genesi OS..."
echo ""

# Compila o nocsd.so primeiro
echo "📦 Compilando nocsd.so..."
if [ -f "genesi-desktop/genesi-wm/nocsd.c" ]; then
    cc -shared -fPIC -ldl -o /tmp/genesi_nocsd.so genesi-desktop/genesi-wm/nocsd.c
    echo "/tmp/genesi_nocsd.so" > /tmp/genesi-nocsd-path.txt
    echo "✓ nocsd.so compilado"
else
    echo "⚠ nocsd.c não encontrado, continuando sem ele..."
fi

echo ""

# Função para cleanup ao sair
cleanup() {
    echo ""
    echo "🛑 Parando Genesi OS..."
    pkill -P $$ || true
    exit 0
}

trap cleanup SIGINT SIGTERM

# 1. Inicia o Window Manager em background
echo "🪟 Iniciando Window Manager (genesi-wm)..."
cd genesi-desktop/genesi-wm
cargo build --release 2>&1 | grep -E "Compiling|Finished|error" || true
cargo run --release &
WM_PID=$!
cd ../..

# Aguarda o WM inicializar
sleep 2

# Verifica se o WM está rodando
if ! kill -0 $WM_PID 2>/dev/null; then
    echo "❌ Erro: Window Manager falhou ao iniciar"
    exit 1
fi

echo "✓ Window Manager rodando (PID: $WM_PID)"
echo ""

# 2. Inicia o Desktop Environment
echo "🖥️  Iniciando Desktop Environment (genesi-desktop)..."
cd genesi-desktop

# Instala dependências se necessário
if [ ! -d "node_modules" ]; then
    echo "📦 Instalando dependências npm..."
    npm install
fi

# Roda o Tauri em modo dev
npm run tauri dev

# Quando o Tauri fechar, mata o WM também
cleanup
