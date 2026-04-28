#!/bin/bash
# ============================================================
# Genesi OS - Teste Local (sem buildar ISO)
# ============================================================
# Este script instala os pacotes e configurações do Genesi OS
# diretamente na sua VM para testar sem precisar buildar a ISO.
#
# USO: sudo ./test-local.sh
# ============================================================

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

if [ "$EUID" -ne 0 ]; then
    echo -e "${RED}Rode com sudo: sudo ./test-local.sh${NC}"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Genesi OS - Teste Local${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

# ---- 1. Copiar configs do skel para o usuário atual ----
echo -e "${YELLOW}[1/4] Copiando configurações do desktop...${NC}"
REAL_USER="${SUDO_USER:-$USER}"
REAL_HOME=$(eval echo "~$REAL_USER")

if [ -d "$SCRIPT_DIR/airootfs/etc/skel/.config" ]; then
    cp -r "$SCRIPT_DIR/airootfs/etc/skel/.config/"* "$REAL_HOME/.config/" 2>/dev/null || true
    chown -R "$REAL_USER:$REAL_USER" "$REAL_HOME/.config/"
    echo -e "  ${GREEN}✓ Configs copiadas para $REAL_HOME/.config/${NC}"
else
    echo -e "  ${YELLOW}⚠ Nenhuma config de desktop encontrada em airootfs/etc/skel/.config${NC}"
fi

# ---- 2. Copiar configs do sistema ----
echo -e "${YELLOW}[2/4] Aplicando configurações do sistema...${NC}"

# sysctl
if [ -d "$SCRIPT_DIR/airootfs/etc/sysctl.d" ]; then
    cp "$SCRIPT_DIR/airootfs/etc/sysctl.d/"* /etc/sysctl.d/ 2>/dev/null || true
    sysctl --system > /dev/null 2>&1
    echo -e "  ${GREEN}✓ sysctl configs aplicadas${NC}"
fi

# modprobe
if [ -d "$SCRIPT_DIR/airootfs/etc/modprobe.d" ]; then
    cp "$SCRIPT_DIR/airootfs/etc/modprobe.d/"* /etc/modprobe.d/ 2>/dev/null || true
    echo -e "  ${GREEN}✓ modprobe configs copiadas${NC}"
fi

echo -e "  ${GREEN}✓ Configurações do sistema aplicadas${NC}"

# ---- 3. Instalar pacotes ----
echo -e "${YELLOW}[3/4] Instalando pacotes...${NC}"
echo -e "  ${YELLOW}Isso pode demorar alguns minutos...${NC}"

# Filtrar comentários e linhas vazias
PACKAGES=$(grep -v '^#' "$SCRIPT_DIR/packages.x86_64" | grep -v '^$' | tr '\n' ' ')

# Remover pacotes que são específicos de ISO/live e não fazem sentido instalar localmente
SKIP_PACKAGES="mkinitcpio-archiso archinstall clonezilla cloud-init darkhttpd ddrescue partclone partimage syslinux memtest86+ memtest86+-efi edk2-shell"

INSTALL_PACKAGES=""
for pkg in $PACKAGES; do
    skip=false
    for skip_pkg in $SKIP_PACKAGES; do
        if [ "$pkg" == "$skip_pkg" ]; then
            skip=true
            break
        fi
    done
    if [ "$skip" == "false" ]; then
        INSTALL_PACKAGES="$INSTALL_PACKAGES $pkg"
    fi
done

pacman -S --needed --noconfirm $INSTALL_PACKAGES 2>&1 | tail -5
echo -e "  ${GREEN}✓ Pacotes instalados${NC}"

# ---- 4. Habilitar serviços ----
echo -e "${YELLOW}[4/4] Habilitando serviços...${NC}"
systemctl enable sddm.service 2>/dev/null || true
systemctl enable NetworkManager.service 2>/dev/null || true
systemctl enable bluetooth.service 2>/dev/null || true
systemctl set-default graphical.target 2>/dev/null || true
echo -e "  ${GREEN}✓ Serviços habilitados${NC}"

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Teste local pronto!${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "Para testar o desktop, reinicie a VM:"
echo -e "  ${YELLOW}sudo reboot${NC}"
echo ""
echo -e "Ou inicie o SDDM agora:"
echo -e "  ${YELLOW}sudo systemctl start sddm${NC}"
echo ""
