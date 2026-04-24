# 🔥 Como Criar ISO do Genesi OS

## ⚠️ IMPORTANTE: WSL NÃO FUNCIONA!

O WSL tem limitações com `chroot` e `sudo` que impedem a criação da ISO:
```
❌ Erro: "unable to allocate pty: No such device"
```

**Soluções:**
1. ✅ **Usar VM Linux** (VirtualBox/VMware) - **RECOMENDADO**
2. ✅ Usar instância Linux na cloud (AWS EC2, Google Cloud)
3. ⚠️ Tentar Docker (pode ter mesmas limitações)

**Veja guias rápidos:**
- `GUIA-CRIAR-ISO-VM.md` - Passo a passo completo com VirtualBox
- `QUICK-ISO-GUIDE.md` - Guia visual rápido

---

## 📋 Pré-requisitos

- **Linux nativo** (Ubuntu/Debian 22.04 recomendado)
- Pelo menos 30GB de espaço livre
- 4GB RAM mínimo
- Conexão com internet

## 🎯 Estratégia

Vamos criar uma ISO baseada em Ubuntu/Debian com:
1. Sistema base mínimo
2. Genesi WM (Window Manager)
3. Genesi Desktop (Tauri)
4. Configuração para iniciar automaticamente

## 📦 Opção 1: ISO Customizada (Recomendado)

### Passo 1: Instalar ferramentas

```bash
sudo apt update
sudo apt install -y \
    debootstrap \
    squashfs-tools \
    xorriso \
    isolinux \
    syslinux-efi \
    grub-pc-bin \
    grub-efi-amd64-bin \
    mtools
```

### Passo 2: Criar estrutura base

```bash
# Cria diretório de trabalho
mkdir -p ~/genesi-iso
cd ~/genesi-iso

# Cria sistema base com debootstrap
sudo debootstrap --arch=amd64 jammy chroot http://archive.ubuntu.com/ubuntu/

# Monta sistemas necessários
sudo mount --bind /dev chroot/dev
sudo mount --bind /proc chroot/proc
sudo mount --bind /sys chroot/sys
```

### Passo 3: Configurar sistema base

```bash
# Entra no chroot
sudo chroot chroot

# Configura hostname
echo "genesi-os" > /etc/hostname

# Configura sources.list
cat > /etc/apt/sources.list << EOF
deb http://archive.ubuntu.com/ubuntu jammy main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu jammy-updates main restricted universe multiverse
deb http://archive.ubuntu.com/ubuntu jammy-security main restricted universe multiverse
EOF

# Atualiza e instala pacotes essenciais
apt update
apt install -y \
    linux-generic \
    systemd \
    network-manager \
    sudo \
    curl \
    wget \
    git \
    build-essential \
    libwebkit2gtk-4.1-dev \
    libgtk-3-dev \
    libayatana-appindicator3-dev \
    librsvg2-dev \
    patchelf \
    chromium-browser \
    firefox

# Instala Node.js
curl -fsSL https://deb.nodesource.com/setup_20.x | bash -
apt install -y nodejs

# Instala Rust
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
source $HOME/.cargo/env

# Cria usuário genesi
useradd -m -s /bin/bash -G sudo genesi
echo "genesi:genesi" | chpasswd

# Sai do chroot
exit
```

### Passo 4: Copiar Genesi OS para o sistema

```bash
# Copia o código do Genesi OS
sudo mkdir -p chroot/home/genesi/GenesiOS
sudo cp -r /caminho/para/seu/GenesiOS/* chroot/home/genesi/GenesiOS/
sudo chown -R 1000:1000 chroot/home/genesi/GenesiOS

# Compila o Genesi OS dentro do chroot
sudo chroot chroot /bin/bash << 'EOFCHROOT'
su - genesi
cd ~/GenesiOS/genesi-desktop/genesi-wm
cargo build --release
cd ~/GenesiOS/genesi-desktop
npm install
npm run tauri build
exit
EOFCHROOT
```

### Passo 5: Configurar autostart

```bash
# Cria script de inicialização
sudo tee chroot/usr/local/bin/start-genesi.sh > /dev/null << 'EOF'
#!/bin/bash
export DISPLAY=:0
export WAYLAND_DISPLAY=wayland-0

# Inicia Window Manager
/home/genesi/GenesiOS/genesi-desktop/genesi-wm/target/release/genesi-wm &

# Aguarda WM iniciar
sleep 2

# Inicia Desktop
cd /home/genesi/GenesiOS/genesi-desktop/src-tauri/target/release
./genesi-desktop
EOF

sudo chmod +x chroot/usr/local/bin/start-genesi.sh

# Configura systemd service
sudo tee chroot/etc/systemd/system/genesi.service > /dev/null << 'EOF'
[Unit]
Description=Genesi OS Desktop Environment
After=graphical.target

[Service]
Type=simple
User=genesi
ExecStart=/usr/local/bin/start-genesi.sh
Restart=always

[Install]
WantedBy=graphical.target
EOF

# Habilita o serviço
sudo chroot chroot systemctl enable genesi.service
```

### Passo 6: Criar ISO

```bash
# Desmonta sistemas
sudo umount chroot/dev
sudo umount chroot/proc
sudo umount chroot/sys

# Cria squashfs
sudo mksquashfs chroot filesystem.squashfs -comp xz

# Cria estrutura ISO
mkdir -p iso/boot/grub
mkdir -p iso/live

# Copia kernel e initrd
sudo cp chroot/boot/vmlinuz-* iso/boot/vmlinuz
sudo cp chroot/boot/initrd.img-* iso/boot/initrd.img
mv filesystem.squashfs iso/live/

# Cria grub.cfg
cat > iso/boot/grub/grub.cfg << 'EOF'
set timeout=10
set default=0

menuentry "Genesi OS" {
    linux /boot/vmlinuz boot=live quiet splash
    initrd /boot/initrd.img
}

menuentry "Genesi OS (Safe Mode)" {
    linux /boot/vmlinuz boot=live nomodeset
    initrd /boot/initrd.img
}
EOF

# Gera ISO
grub-mkrescue -o GenesiOS.iso iso/
```

### Passo 7: ISO criada!

```bash
ls -lh GenesiOS.iso
# Deve mostrar algo como: -rw-r--r-- 1 user user 1.5G GenesiOS.iso
```

## 📦 Opção 2: ISO Rápida (Baseada em Ubuntu Live)

### Método mais simples usando Cubic

```bash
# Instala Cubic (ferramenta GUI para criar ISOs customizadas)
sudo apt-add-repository ppa:cubic-wizard/release
sudo apt update
sudo apt install cubic

# Baixa Ubuntu Server ISO
wget https://releases.ubuntu.com/22.04/ubuntu-22.04.3-live-server-amd64.iso

# Abre Cubic
cubic

# No Cubic:
# 1. Seleciona a ISO do Ubuntu
# 2. Customiza o sistema (instala Genesi OS)
# 3. Gera nova ISO
```

## 🖥️ Testando na VM

### VirtualBox

```bash
# Instala VirtualBox
sudo apt install virtualbox

# Ou baixa do site: https://www.virtualbox.org/
```

**Criar VM:**
1. Abra VirtualBox
2. Clique em "Novo"
3. Nome: Genesi OS
4. Tipo: Linux
5. Versão: Ubuntu (64-bit)
6. Memória: 4096 MB (4GB)
7. Disco: 20 GB (VDI dinâmico)
8. Configurações → Armazenamento → Adiciona ISO
9. Configurações → Sistema → Habilita EFI
10. Configurações → Display → Memória de vídeo: 128 MB
11. Inicia a VM

### VMware Workstation

```bash
# Baixa VMware Workstation Player (grátis)
# https://www.vmware.com/products/workstation-player.html
```

**Criar VM:**
1. Abra VMware
2. Create a New Virtual Machine
3. Installer disc image file (iso): Seleciona GenesiOS.iso
4. Guest OS: Linux → Ubuntu 64-bit
5. Memória: 4 GB
6. Disco: 20 GB
7. Customize Hardware → Display → Accelerate 3D graphics
8. Finish → Power On

### QEMU/KVM (Linha de comando)

```bash
# Instala QEMU
sudo apt install qemu-kvm

# Cria disco virtual
qemu-img create -f qcow2 genesi-disk.qcow2 20G

# Roda a ISO
qemu-system-x86_64 \
    -enable-kvm \
    -m 4096 \
    -smp 2 \
    -cdrom GenesiOS.iso \
    -boot d \
    -hda genesi-disk.qcow2 \
    -vga virtio \
    -display gtk
```

## 🎯 Opção 3: Script Automatizado

Vou criar um script que faz tudo automaticamente:

```bash
# Salve como: build-iso.sh
chmod +x build-iso.sh
sudo ./build-iso.sh
```

## ⚠️ Notas Importantes

### Tamanho da ISO
- **Mínima**: ~800 MB (sem navegadores)
- **Completa**: ~1.5-2 GB (com Chrome/Firefox)

### Requisitos da VM
- **RAM**: Mínimo 2GB, recomendado 4GB
- **Disco**: Mínimo 10GB, recomendado 20GB
- **CPU**: 2 cores recomendado
- **Vídeo**: 128MB VRAM, aceleração 3D habilitada

### Boot
- **BIOS**: Funciona
- **UEFI**: Recomendado (melhor compatibilidade)

### Rede
- **NAT**: Funciona (padrão)
- **Bridge**: Recomendado (melhor performance)

## 🐛 Troubleshooting

### ISO não boota
- Verifique se criou com UEFI support
- Tente modo BIOS legacy
- Verifique se o grub.cfg está correto

### Tela preta após boot
- Adicione `nomodeset` nos parâmetros do kernel
- Tente `i915.modeset=0` para Intel
- Tente `nouveau.modeset=0` para NVIDIA

### Genesi OS não inicia
- Verifique logs: `journalctl -u genesi.service`
- Teste manualmente: `/usr/local/bin/start-genesi.sh`
- Verifique se compilou corretamente

## 📚 Próximos Passos

Depois de criar a ISO:

1. ✅ Teste na VM
2. ✅ Verifique se o navegador funciona (1 barra apenas)
3. ✅ Teste todas as funcionalidades
4. ✅ Ajuste conforme necessário
5. ✅ Crie versão final

## 🎉 Resultado Esperado

Quando bootar da ISO na VM:
- ✅ Sistema inicia automaticamente
- ✅ Genesi OS aparece em tela cheia
- ✅ Navegador funciona com 1 barra apenas
- ✅ Sobreposição de janelas funciona
- ✅ Tudo integrado perfeitamente

Quer que eu crie o script automatizado `build-iso.sh` para você?
