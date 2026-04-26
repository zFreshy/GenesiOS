#!/bin/bash
# Script de diagnóstico para Genesi OS com Sway
# Execute dentro da ISO bootada para verificar o estado do sistema

echo "🔍 Genesi OS - Diagnóstico Sway"
echo "================================"
echo ""

echo "1️⃣ Verificando processos..."
echo "----------------------------"
if pgrep -x sway > /dev/null; then
    echo "✅ Sway está rodando (PID: $(pgrep -x sway))"
else
    echo "❌ Sway NÃO está rodando"
fi

if pgrep -x genesi-desktop > /dev/null; then
    echo "✅ Genesi Desktop está rodando (PID: $(pgrep -x genesi-desktop))"
else
    echo "❌ Genesi Desktop NÃO está rodando"
fi

if pgrep -x genesi-wm > /dev/null; then
    echo "⚠️  Genesi WM está rodando (PID: $(pgrep -x genesi-wm)) - NÃO DEVERIA!"
else
    echo "✅ Genesi WM não está rodando (correto)"
fi

echo ""
echo "2️⃣ Verificando sockets Wayland..."
echo "----------------------------------"
if [ -n "$XDG_RUNTIME_DIR" ]; then
    echo "XDG_RUNTIME_DIR: $XDG_RUNTIME_DIR"
    if [ -d "$XDG_RUNTIME_DIR" ]; then
        echo "Conteúdo:"
        ls -la "$XDG_RUNTIME_DIR/" | grep wayland
        
        if [ -S "$XDG_RUNTIME_DIR/wayland-0" ]; then
            echo "✅ Socket wayland-0 existe"
        else
            echo "❌ Socket wayland-0 NÃO existe"
        fi
        
        if [ -S "$XDG_RUNTIME_DIR/wayland-1" ]; then
            echo "⚠️  Socket wayland-1 existe (não deveria)"
        else
            echo "✅ Socket wayland-1 não existe (correto)"
        fi
    else
        echo "❌ XDG_RUNTIME_DIR não existe!"
    fi
else
    echo "❌ XDG_RUNTIME_DIR não está definido!"
fi

echo ""
echo "3️⃣ Verificando variáveis de ambiente..."
echo "----------------------------------------"
echo "WAYLAND_DISPLAY: ${WAYLAND_DISPLAY:-não definido}"
echo "DISPLAY: ${DISPLAY:-não definido}"
echo "GDK_BACKEND: ${GDK_BACKEND:-não definido}"
echo "QT_QPA_PLATFORM: ${QT_QPA_PLATFORM:-não definido}"
echo "MOZ_ENABLE_WAYLAND: ${MOZ_ENABLE_WAYLAND:-não definido}"

echo ""
echo "4️⃣ Verificando logs..."
echo "-----------------------"
if [ -f /tmp/genesi-startup.log ]; then
    echo "📄 Últimas 20 linhas de /tmp/genesi-startup.log:"
    echo "------------------------------------------------"
    tail -n 20 /tmp/genesi-startup.log
else
    echo "❌ Log /tmp/genesi-startup.log não encontrado"
fi

echo ""
echo "5️⃣ Verificando binários..."
echo "---------------------------"
if [ -f /home/genesi/GenesiOS/genesi-desktop/src-tauri/target/release/genesi-desktop ]; then
    echo "✅ Binário genesi-desktop existe"
    ls -lh /home/genesi/GenesiOS/genesi-desktop/src-tauri/target/release/genesi-desktop
else
    echo "❌ Binário genesi-desktop NÃO encontrado"
fi

if command -v sway &> /dev/null; then
    echo "✅ Sway instalado: $(which sway)"
    sway --version
else
    echo "❌ Sway NÃO instalado"
fi

echo ""
echo "6️⃣ Verificando rede..."
echo "-----------------------"
if ping -c 1 8.8.8.8 &> /dev/null; then
    echo "✅ Internet funcionando"
else
    echo "❌ Sem internet"
    echo "   Execute: sudo ip link set enp0s3 up"
    echo "            sudo dhclient enp0s3"
fi

echo ""
echo "7️⃣ Verificando configuração Sway..."
echo "------------------------------------"
if [ -f ~/.config/sway/config ]; then
    echo "✅ Configuração Sway existe"
    echo "Conteúdo:"
    cat ~/.config/sway/config
else
    echo "❌ Configuração Sway não encontrada"
fi

echo ""
echo "================================"
echo "Diagnóstico completo!"
echo ""
echo "💡 Dicas:"
echo "   - Se Sway não está rodando: execute 'sway' manualmente"
echo "   - Se Desktop não está rodando: veja o log acima"
echo "   - Para reiniciar: Ctrl+Alt+F1 e execute 'start-genesi.sh'"
echo ""
