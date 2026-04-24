# 🖥️ Guia: Criar ISO do Genesi OS em VM Linux

## ⚠️ Por que não funciona no WSL?

O WSL tem limitações com:
- `chroot` (usado para criar o sistema base)
- `sudo` com alocação de pty
- Montagem de sistemas de arquivos especiais

**Solução:** Usar Linux nativo em VM ou cloud.

---

## 🎯 Método 1: VirtualBox (Recomendado)

### Passo 1: Instalar VirtualBox

1. Baixe: https://www.virtualbox.org/wiki/Downloads
2. Instale no Windows
3. Baixe Ubuntu 22.04 ISO: https://ubuntu.com/download/desktop

### Passo 2: Criar VM Ubuntu

1. Abra VirtualBox → **Novo**
2. Configure:
   - **Nome:** Ubuntu-Build
   - **Tipo:** Linux
   - **Versão:** Ubuntu (64-bit)
   - **Memória:** 4096 MB (4GB)
   - **Disco:** 30 GB (VDI dinâmico)
3. **Configurações** → **Sistema:**
   - Processador: 2 CPUs
   - Habilitar PAE/NX
4. **Configurações** → **Armazenamento:**
   - Adicionar Ubuntu ISO no drive óptico
5. **Configurações** → **Rede:**
   - Adaptador 1: NAT (ou Bridge para melhor performance)
6. **Iniciar** a VM e instalar Ubuntu

### Passo 3: Configurar Ubuntu na VM

```bash
# Atualizar sistema
sudo apt update && sudo apt upgrade -y

# Instalar Git
sudo apt install -y git

# Clonar seu projeto (ou transferir via pasta compartilhada)
git clone https://github.com/seu-usuario/GenesiOS.git
cd GenesiOS
```

### Passo 4: Criar a ISO

```bash
# Tornar script executável
chmod +x build-iso.sh

# Executar (vai demorar 15-20 minutos)
sudo ./build-iso.sh
```

**O que vai acontecer:**
1. ✅ Instala dependências (debootstrap, squashfs-tools, etc)
2. ✅ Cria sistema base Ubuntu 22.04
3. ✅ Instala Rust, Node.js, navegadores
4. ✅ Compila Genesi WM e Desktop
5. ✅ Configura autostart
6. ✅ Gera ISO bootável

### Passo 5: Pegar a ISO

A ISO estará em: `~/GenesiOS/GenesiOS-YYYYMMDD.iso`

**Transferir para Windows:**
- Opção 1: Pasta compartilhada do VirtualBox
- Opção 2: Upload para Google Drive/Dropbox
- Opção 3: Servidor HTTP simples:
  ```bash
  cd ~/GenesiOS
  python3 -m http.server 8000
  # No Windows: http://IP-DA-VM:8000
  ```

---

## 🎯 Método 2: VMware Workstation Player

### Passo 1: Instalar VMware

1. Baixe: https://www.vmware.com/products/workstation-player.html (grátis)
2. Instale no Windows
3. Baixe Ubuntu 22.04 ISO

### Passo 2: Criar VM

1. **Create a New Virtual Machine**
2. **Installer disc image (iso):** Selecione Ubuntu ISO
3. Configure:
   - **Full name:** genesi
   - **Username:** genesi
   - **Password:** genesi
4. **Disk size:** 30 GB
5. **Customize Hardware:**
   - Memory: 4 GB
   - Processors: 2
6. **Finish** → Aguarde instalação automática

### Passo 3: Seguir passos 3-5 do método VirtualBox

---

## 🎯 Método 3: AWS EC2 (Cloud)

### Passo 1: Criar Instância

1. Acesse: https://console.aws.amazon.com/ec2/
2. **Launch Instance**
3. Configure:
   - **AMI:** Ubuntu Server 22.04 LTS
   - **Instance type:** t2.medium (4GB RAM)
   - **Storage:** 30 GB
4. **Launch** e baixe a chave SSH

### Passo 2: Conectar

```bash
# No Windows (PowerShell ou WSL)
ssh -i sua-chave.pem ubuntu@IP-DA-INSTANCIA
```

### Passo 3: Transferir Código

```bash
# Na sua máquina local (WSL)
cd /caminho/para/GenesiOS
tar czf genesi.tar.gz .

# Transferir
scp -i sua-chave.pem genesi.tar.gz ubuntu@IP-DA-INSTANCIA:~

# Na instância EC2
tar xzf genesi.tar.gz
cd GenesiOS
```

### Passo 4: Criar ISO

```bash
chmod +x build-iso.sh
sudo ./build-iso.sh
```

### Passo 5: Baixar ISO

```bash
# Na sua máquina local
scp -i sua-chave.pem ubuntu@IP-DA-INSTANCIA:~/GenesiOS/GenesiOS-*.iso .
```

---

## 🎯 Método 4: Docker (Experimental)

**Atenção:** Pode não funcionar dependendo da configuração do Docker.

```bash
# No WSL
cd GenesiOS

# Build da imagem
docker build -f Dockerfile.iso -t genesi-iso-builder .

# Executar com privilégios
docker run --privileged -v $(pwd):/output genesi-iso-builder

# ISO estará em: ./GenesiOS-*.iso
```

---

## 🧪 Testar a ISO

### VirtualBox

1. **Novo** → Nome: Genesi OS Test
2. **Tipo:** Linux, **Versão:** Ubuntu 64-bit
3. **Memória:** 4096 MB
4. **Disco:** 20 GB
5. **Configurações:**
   - Sistema → Habilitar EFI
   - Display → 128 MB VRAM, Aceleração 3D
   - Armazenamento → Adicionar ISO
6. **Iniciar**

### VMware

1. **Create a New Virtual Machine**
2. **Installer disc image:** Selecione sua ISO
3. **Guest OS:** Linux → Ubuntu 64-bit
4. **Memory:** 4 GB
5. **Disk:** 20 GB
6. **Customize Hardware:**
   - Display → Accelerate 3D graphics
7. **Power On**

### QEMU (Linha de comando)

```bash
# Criar disco virtual
qemu-img create -f qcow2 genesi-test.qcow2 20G

# Rodar ISO
qemu-system-x86_64 \
    -enable-kvm \
    -m 4096 \
    -smp 2 \
    -cdrom GenesiOS-*.iso \
    -boot d \
    -hda genesi-test.qcow2 \
    -vga virtio \
    -display gtk
```

---

## ✅ O que Esperar na VM de Teste

Quando bootar a ISO:

1. **GRUB** aparece com opções:
   - Genesi OS (normal)
   - Genesi OS (Safe Mode)
   - Genesi OS (Debug)

2. **Sistema inicia** automaticamente

3. **Genesi OS aparece** em tela cheia

4. **Navegador funciona** com:
   - ✅ **1 barra apenas** (não mais 2!)
   - ✅ **Sobreposição correta** de janelas
   - ✅ **Integração perfeita** com o WM

5. **Tudo funciona** como um OS real!

---

## 🐛 Troubleshooting

### ISO não boota
- Habilite EFI nas configurações da VM
- Ou tente modo BIOS Legacy

### Tela preta após boot
- Use opção "Safe Mode" no GRUB
- Ou adicione `nomodeset` nos parâmetros

### Genesi OS não inicia
Na VM, pressione `Ctrl+Alt+F2` e faça login:
```bash
# Usuário: genesi
# Senha: genesi

# Ver logs
journalctl -u genesi.service

# Testar manualmente
/usr/local/bin/start-genesi.sh
```

---

## 📊 Comparação dos Métodos

| Método | Tempo Setup | Custo | Dificuldade | Recomendado |
|--------|-------------|-------|-------------|-------------|
| VirtualBox | 30 min | Grátis | Fácil | ⭐⭐⭐⭐⭐ |
| VMware | 30 min | Grátis | Fácil | ⭐⭐⭐⭐ |
| AWS EC2 | 10 min | ~$0.50/hora | Médio | ⭐⭐⭐ |
| Docker | 5 min | Grátis | Difícil | ⭐⭐ |

---

## 🚀 Rebuild Rápido

Depois do primeiro build, para atualizar a ISO:

```bash
# Faça mudanças no código
vim genesi-desktop/src/App.tsx

# Rebuild rápido (5 minutos)
sudo ./rebuild-iso.sh
```

O `rebuild-iso.sh` reutiliza o sistema base e só recompila o Genesi OS.

---

## 📝 Resumo

1. ❌ **WSL não funciona** para criar ISO (limitações do chroot)
2. ✅ **Use VM Linux** (VirtualBox ou VMware)
3. ✅ **Execute `build-iso.sh`** na VM
4. ✅ **Teste a ISO** em outra VM
5. ✅ **Navegador terá 1 barra** e funcionará perfeitamente!

---

## 🎉 Próximos Passos

Depois de testar a ISO:

1. ✅ Verificar se tudo funciona
2. ✅ Ajustar configurações se necessário
3. ✅ Criar versão final
4. ✅ Distribuir!

Boa sorte! 🚀
