#!/bin/bash
# Script para rodar o Genesi OS completo no WSL/Linux
# Uso: bash run-genesi.sh

set -e

echo "🚀 Iniciando Genesi OS..."
echo ""

# Limpa processos antigos que podem estar travados
echo "🧹 Limpando processos antigos..."
pkill -9 genesi-wm 2>/dev/null || true
pkill -9 genesi-desktop 2>/dev/null || true
pkill -9 cargo 2>/dev/null || true
pkill -9 node 2>/dev/null || true
pkill -9 -f "vite" 2>/dev/null || true
pkill -9 -f "tauri" 2>/dev/null || true

# Mata processos na porta 1420 especificamente
if command -v lsof &> /dev/null; then
    PORT_PIDS=$(lsof -ti:1420 2>/dev/null)
    if [ ! -z "$PORT_PIDS" ]; then
        echo "   Liberando porta 1420..."
        echo "$PORT_PIDS" | xargs kill -9 2>/dev/null || true
    fi
fi

# Aguarda um pouco para garantir que os processos foram mortos
sleep 2
echo "✓ Processos limpos"
echo ""

# Carrega ambiente Rust se existir
if [ -f "$HOME/.cargo/env" ]; then
    source "$HOME/.cargo/env"
fi

# Configura display para WSLg
if [ -z "$DISPLAY" ]; then
    export DISPLAY=:0
    echo "✓ DISPLAY configurado para :0"
fi

# Verifica se tem WSLg/X11
if ! command -v xdpyinfo &> /dev/null; then
    echo "⚠ WSLg não detectado. Instalando dependências..."
    sudo apt update
    sudo apt install -y x11-apps libgtk-3-dev libwebkit2gtk-4.1-dev
fi

# Verifica se tem Rust/Cargo
if ! command -v cargo &> /dev/null; then
    echo "❌ Rust não encontrado!"
    echo "   Execute: bash setup-wsl.sh"
    exit 1
fi

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
    
    # Mata o WM se estiver rodando
    if [ ! -z "$WM_PID" ]; then
        kill -9 $WM_PID 2>/dev/null || true
    fi
    
    # Mata processos relacionados
    pkill -9 genesi-wm 2>/dev/null || true
    pkill -9 genesi-desktop 2>/dev/null || true
    pkill -9 cargo 2>/dev/null || true
    pkill -9 node 2>/dev/null || true
    
    echo "✓ Processos finalizados"
    exit 0
}

# Registra handler para Ctrl+C e outros sinais
trap cleanup SIGINT SIGTERM EXIT

# 1. Inicia o Window Manager em background
echo "🪟 Iniciando Window Manager (genesi-wm)..."
cd genesi-desktop/genesi-wm

# Verifica se precisa compilar
if [ ! -f "target/release/genesi-wm" ]; then
    echo "📦 Compilando Window Manager..."
    cargo build --release 2>&1 | grep -E "Compiling|Finished|error" || true
fi

cargo run --release &
WM_PID=$!
cd ../..

# Aguarda o WM inicializar
sleep 3

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

# Verifica se a porta 1420 está livre antes de iniciar
if command -v lsof &> /dev/null; then
    if lsof -ti:1420 &> /dev/null; then
        echo "❌ Erro: Porta 1420 ainda está em uso!"
        echo "   Executando limpeza forçada..."
        lsof -ti:1420 | xargs kill -9 2>/dev/null || true
        sleep 2
        
        # Verifica novamente
        if lsof -ti:1420 &> /dev/null; then
            echo "❌ Não foi possível liberar a porta 1420"
            echo "   Execute: wsl --shutdown (no PowerShell)"
            cleanup
            exit 1
        fi
    fi
fi

# Configura variáveis de ambiente para Tauri no WSL
export WEBKIT_DISABLE_COMPOSITING_MODE=1
export GDK_BACKEND=x11

echo ""
echo "✓ Genesi OS iniciado!"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Para parar: Pressione Ctrl+C"
echo "  Ou execute: bash stop-genesi.sh"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Roda o Tauri (bloqueia até fechar)
npm run tauri dev

# Quando o Tauri fechar, chama cleanup
cleanup
