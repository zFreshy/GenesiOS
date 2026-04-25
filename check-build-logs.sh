#!/bin/bash
# Script para verificar logs de build da ISO
# Uso: sudo bash check-build-logs.sh

WORK_DIR="$HOME/genesi-iso-build"

if [ ! -d "$WORK_DIR/chroot" ]; then
    echo "❌ Build não encontrado em $WORK_DIR"
    exit 1
fi

echo "📋 Logs de Build do Genesi OS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

echo "1️⃣  Window Manager Build Log:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -f "$WORK_DIR/chroot/tmp/wm-build.log" ]; then
    tail -50 "$WORK_DIR/chroot/tmp/wm-build.log"
else
    echo "❌ Log não encontrado"
fi
echo ""

echo "2️⃣  NPM Install Log:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -f "$WORK_DIR/chroot/tmp/npm-install.log" ]; then
    tail -30 "$WORK_DIR/chroot/tmp/npm-install.log"
else
    echo "❌ Log não encontrado"
fi
echo ""

echo "3️⃣  NPM Build Log:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -f "$WORK_DIR/chroot/tmp/npm-build.log" ]; then
    tail -30 "$WORK_DIR/chroot/tmp/npm-build.log"
else
    echo "❌ Log não encontrado"
fi
echo ""

echo "4️⃣  Tauri Build Log:"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
if [ -f "$WORK_DIR/chroot/tmp/tauri-build.log" ]; then
    tail -50 "$WORK_DIR/chroot/tmp/tauri-build.log"
else
    echo "❌ Log não encontrado"
fi
echo ""

echo "💡 Para ver log completo:"
echo "   cat $WORK_DIR/chroot/tmp/wm-build.log"
echo "   cat $WORK_DIR/chroot/tmp/tauri-build.log"
echo ""
