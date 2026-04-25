#!/bin/bash
# Script para testar Genesi OS localmente (sem criar ISO)

set -e

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Teste Local do Genesi OS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Verifica se está no Ubuntu
if ! grep -q "Ubuntu" /etc/os-release 2>/dev/null; then
    echo "❌ Este script deve rodar no Ubuntu (VM), não no WSL!"
    exit 1
fi

# Verifica dependências
echo "📦 Verificando dependências..."
MISSING_DEPS=()

if ! command -v weston &> /dev/null; then
    MISSING_DEPS+=("weston")
fi

if ! command -v cargo &> /dev/null; then
    MISSING_DEPS+=("cargo (Rust)")
fi

if ! command -v npm &> /dev/null; then
    MISSING_DEPS+=("npm (Node.js)")
fi

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    echo "❌ Dependências faltando: ${MISSING_DEPS[*]}"
    echo ""
    echo "Instale com:"
    echo "  sudo apt update"
    echo "  sudo apt install -y weston"
    echo ""
    echo "Para Rust e Node.js, execute:"
    echo "  bash setup-ubuntu-desktop.sh"
    exit 1
fi

echo "✅ Todas as dependências instaladas"
echo ""

# Compila o WM
echo "🔨 Compilando Genesi WM..."
cd genesi-desktop/genesi-wm
cargo build --release
if [ $? -ne 0 ]; then
    echo "❌ Falha ao compilar o WM"
    exit 1
fi
echo "✅ WM compilado"
cd ../..

# Compila o Desktop
echo "🔨 Compilando Genesi Desktop..."
cd genesi-desktop

# Instala dependências npm se necessário
if [ ! -d "node_modules" ]; then
    echo "  → Instalando dependências npm..."
    npm install
fi

# Compila o Tauri
npm run tauri build
if [ $? -ne 0 ]; then
    echo "❌ Falha ao compilar o Desktop"
    exit 1
fi
echo "✅ Desktop compilado"
cd ..

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Compilação Concluída!"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "Binários gerados:"
echo "  WM:      genesi-desktop/genesi-wm/target/release/genesi-wm"
echo "  Desktop: genesi-desktop/src-tauri/target/release/genesi-desktop"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "  Como Testar"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo "OPÇÃO 1: Teste Rápido (com Weston em janela)"
echo "  bash test-genesi-windowed.sh"
echo ""
echo "OPÇÃO 2: Teste Completo (TTY, como na ISO)"
echo "  1. Saia do ambiente gráfico:"
echo "     sudo systemctl stop gdm3  # ou lightdm/sddm"
echo "  2. Vá para TTY1 (Ctrl+Alt+F1)"
echo "  3. Logue como seu usuário"
echo "  4. Execute:"
echo "     cd ~/GenesiOS"
echo "     bash test-genesi-tty.sh"
echo ""
