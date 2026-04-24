#!/bin/bash
# Script de diagnóstico do Genesi OS
# Uso: bash diagnostico.sh

echo "🔍 Diagnóstico do Genesi OS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# 1. Verifica processos rodando
echo "📊 Processos Genesi rodando:"
GENESI_PROCS=$(ps aux | grep -E "genesi|tauri|cargo.*genesi" | grep -v grep | grep -v diagnostico)
if [ -z "$GENESI_PROCS" ]; then
    echo "   ✓ Nenhum processo rodando"
else
    echo "$GENESI_PROCS"
fi
echo ""

# 2. Verifica porta 1420
echo "🔌 Porta 1420 (Tauri):"
PORT_1420=$(lsof -ti:1420 2>/dev/null)
if [ -z "$PORT_1420" ]; then
    echo "   ✓ Porta livre"
else
    echo "   ⚠ Porta em uso por PID: $PORT_1420"
    ps -p $PORT_1420 -o pid,cmd
fi
echo ""

# 3. Verifica Rust/Cargo
echo "🦀 Rust/Cargo:"
if command -v cargo &> /dev/null; then
    echo "   ✓ Cargo instalado: $(cargo --version)"
else
    echo "   ❌ Cargo não encontrado"
    echo "      Execute: bash setup-wsl.sh"
fi
echo ""

# 4. Verifica Node.js
echo "📦 Node.js:"
if command -v node &> /dev/null; then
    echo "   ✓ Node instalado: $(node --version)"
    echo "   ✓ npm instalado: $(npm --version)"
else
    echo "   ❌ Node não encontrado"
    echo "      Execute: bash setup-wsl.sh"
fi
echo ""

# 5. Verifica DISPLAY (WSLg)
echo "🖥️  Display (WSLg):"
if [ -z "$DISPLAY" ]; then
    echo "   ⚠ DISPLAY não configurado"
    echo "      Execute: export DISPLAY=:0"
else
    echo "   ✓ DISPLAY=$DISPLAY"
fi
echo ""

# 6. Verifica dependências GTK
echo "🎨 Dependências GTK:"
if dpkg -l | grep -q libwebkit2gtk-4.1-dev; then
    echo "   ✓ libwebkit2gtk-4.1-dev instalado"
else
    echo "   ⚠ libwebkit2gtk-4.1-dev não encontrado"
    echo "      Execute: bash setup-wsl.sh"
fi
echo ""

# 7. Verifica binários compilados
echo "🔨 Binários compilados:"
if [ -f "genesi-desktop/genesi-wm/target/release/genesi-wm" ]; then
    echo "   ✓ genesi-wm compilado"
else
    echo "   ⚠ genesi-wm não compilado"
    echo "      Será compilado na primeira execução"
fi
echo ""

# 8. Verifica node_modules
echo "📚 Dependências Node:"
if [ -d "genesi-desktop/node_modules" ]; then
    echo "   ✓ node_modules instalado"
else
    echo "   ⚠ node_modules não encontrado"
    echo "      Execute: cd genesi-desktop && npm install"
fi
echo ""

# 9. Verifica nocsd.so
echo "🔧 nocsd.so (Firefox CSD fix):"
if [ -f "/tmp/genesi_nocsd.so" ]; then
    echo "   ✓ nocsd.so compilado"
else
    echo "   ⚠ nocsd.so não encontrado"
    echo "      Será compilado automaticamente"
fi
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Recomendações
echo "💡 Recomendações:"
echo ""

if [ -z "$GENESI_PROCS" ] && [ -z "$PORT_1420" ]; then
    echo "   ✅ Sistema pronto para rodar!"
    echo "      Execute: bash run-genesi.sh"
else
    echo "   ⚠ Sistema já está rodando ou porta ocupada"
    echo "      Para parar: bash stop-genesi.sh"
    echo "      Para limpar: ./cleanup"
fi

echo ""
