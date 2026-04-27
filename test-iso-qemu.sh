#!/bin/bash
# Testa a ISO com QEMU para ver erros mais claros

set -e

echo "🔍 Testando ISO com QEMU"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Encontra a ISO mais recente
ISO=$(ls -t GenesiOS-*.iso 2>/dev/null | head -1)

if [ -z "$ISO" ]; then
    echo "❌ Nenhuma ISO encontrada!"
    echo "   Execute: sudo ./rebuild-iso.sh"
    exit 1
fi

echo "📦 ISO encontrada: $ISO"
echo "📊 Tamanho: $(du -h "$ISO" | cut -f1)"
echo ""

# Verifica se QEMU está instalado
if ! command -v qemu-system-x86_64 &> /dev/null; then
    echo "⚠️  QEMU não está instalado"
    echo ""
    echo "Instale com:"
    echo "  sudo apt install qemu-system-x86"
    echo ""
    exit 1
fi

echo "🚀 Iniciando VM com QEMU..."
echo "   (Pressione Ctrl+C para sair)"
echo ""

# Roda QEMU com configurações seguras
qemu-system-x86_64 \
    -m 4096 \
    -smp 4 \
    -cdrom "$ISO" \
    -boot d \
    -vga std \
    -serial stdio \
    -enable-kvm 2>/dev/null || \
qemu-system-x86_64 \
    -m 4096 \
    -smp 4 \
    -cdrom "$ISO" \
    -boot d \
    -vga std \
    -serial stdio

echo ""
echo "VM encerrada."
