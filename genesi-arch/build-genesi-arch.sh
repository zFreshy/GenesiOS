#!/bin/bash
set -e

# Genesi OS Arch Edition - Build Script
# Baseado em archiso e CachyOS

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORK_DIR="${SCRIPT_DIR}/work"
OUT_DIR="${SCRIPT_DIR}/out"
PROFILE_DIR="${SCRIPT_DIR}"

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Verificar se está rodando como root
if [ "$EUID" -ne 0 ]; then
    print_error "Este script precisa ser executado como root (use sudo)"
    exit 1
fi

# Verificar dependências
print_info "Verificando dependências..."
DEPS=("archiso" "mkinitcpio-archiso" "git" "squashfs-tools" "grub")
MISSING_DEPS=()

for dep in "${DEPS[@]}"; do
    if ! pacman -Qi "$dep" &> /dev/null; then
        MISSING_DEPS+=("$dep")
    fi
done

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    print_error "Dependências faltando: ${MISSING_DEPS[*]}"
    print_info "Instale com: sudo pacman -S ${MISSING_DEPS[*]}"
    exit 1
fi

print_success "Todas as dependências estão instaladas"

# Limpar diretórios anteriores
print_info "Limpando builds anteriores..."
rm -rf "${WORK_DIR}"
mkdir -p "${OUT_DIR}"

# Verificar se profiledef.sh existe
if [ ! -f "${PROFILE_DIR}/profiledef.sh" ]; then
    print_error "profiledef.sh não encontrado em ${PROFILE_DIR}"
    exit 1
fi

# Build da ISO usando mkarchiso
print_info "Iniciando build da ISO..."
print_info "Profile: ${PROFILE_DIR}"
print_info "Work dir: ${WORK_DIR}"
print_info "Output dir: ${OUT_DIR}"

mkarchiso -v -w "${WORK_DIR}" -o "${OUT_DIR}" "${PROFILE_DIR}"

if [ $? -eq 0 ]; then
    print_success "ISO construída com sucesso!"
    print_info "ISO localizada em: ${OUT_DIR}"
    ls -lh "${OUT_DIR}"/*.iso
else
    print_error "Falha ao construir a ISO"
    exit 1
fi

# Limpar work dir (opcional)
read -p "Deseja remover o diretório de trabalho? (s/N) " -n 1 -r
echo
if [[ $REPLY =~ ^[Ss]$ ]]; then
    print_info "Removendo diretório de trabalho..."
    rm -rf "${WORK_DIR}"
    print_success "Diretório de trabalho removido"
fi

print_success "Build completo!"
