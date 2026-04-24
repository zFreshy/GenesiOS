#!/bin/bash
# Script que limpa e inicia o Genesi OS
# Uso: bash start.sh

echo "🔄 Preparando Genesi OS..."
echo ""

# 1. Para processos antigos
bash stop-genesi.sh

# 2. Aguarda um pouco
sleep 1

# 3. Inicia o sistema
bash run-genesi.sh
