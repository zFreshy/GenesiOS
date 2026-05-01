# 🔧 Guia de Build Manual - Genesi OS

## 🎯 Objetivo

Buildar a ISO do Genesi OS manualmente na VM, sem depender do GitHub Actions (por enquanto).

---

## 📋 Pré-requisitos

- VM com CachyOS ou Arch Linux
- Acesso SSH (opcional)
- Git instalado

---

## 🚀 Passo a Passo Completo

### 1️⃣ Preparar VM

```bash
# SSH na VM
ssh user@vm-ip

# Atualizar sistema
sudo pacman -Syu

# Instalar dependências
sudo pacman -S base-devel git archiso
```

### 2️⃣ Clonar Repositório

```bash
# Clonar repo
cd ~
git clone https://github.com/zFreshy/GenesiOS.git
cd GenesiOS

# Ou atualizar se já existe
cd ~/GenesiOS
git pull origin main
```

### 3️⃣ Buildar ISO

```bash
cd genesi-arch

# Build ISO (20-30 minutos)
sudo ./buildiso.sh -p desktop
```

**Output esperado**: `out/GenesiOS-YYYYMMDD-HHMM.iso`

---

## 🧪 Testar ISO

### Opção A: QEMU (rápido)

```bash
# Instalar QEMU
sudo pacman -S qemu-full

# Rodar ISO
qemu-system-x86_64 \
  -m 4096 \
  -smp 4 \
  -cdrom out/GenesiOS-*.iso \
  -boot d \
  -enable-kvm
```

### Opção B: VirtualBox

1. Copiar ISO para Windows:
   ```bash
   # Na VM
   scp out/GenesiOS-*.iso user@windows-ip:/path/
   ```

2. Criar VM no VirtualBox:
   - RAM: 4GB
   - Disco: 20GB
   - Montar ISO

3. Boot e testar

---

## ✅ Verificações Após Boot

### Desktop e Tema

```bash
# Verificar OS
cat /etc/os-release | grep Genesi

# Verificar wallpaper
ls /usr/share/wallpapers/genesi/

# Verificar pacotes Genesi
pacman -Q | grep genesi
```

**Esperado**:
```
genesi-settings 1.0.0-1
genesi-kde-settings 1.0.0-2
genesi-ai-mode 1.0.0-1
genesi-updater 1.0.0-1
```

### AI Mode

```bash
# Verificar daemon
systemctl status genesi-aid

# Ver logs
sudo journalctl -u genesi-aid -n 20
```

### Sistema de Updates

```bash
# Verificar timer
systemctl status genesi-updater.timer

# Verificar repositório
cat /etc/pacman.conf | grep -A 2 "\[genesi\]"

# Testar comando
checkupdates
```

---

## 🐛 Troubleshooting

### Build falha

```bash
# Ver logs
cat build.log

# Limpar e tentar novamente
sudo rm -rf build/ out/ work/
sudo ./buildiso.sh -p desktop
```

### Pacotes não instalam

```bash
# Verificar se pacotes existem
ls -la archiso/airootfs/usr/share/wallpapers/genesi/

# Verificar packages_desktop.x86_64
cat archiso/packages_desktop.x86_64 | grep genesi
```

### Wallpaper não aparece

```bash
# Verificar arquivo
ls -lh archiso/airootfs/usr/share/wallpapers/genesi/wallpaper.png

# Verificar configuração KDE
cat archiso/airootfs/etc/skel/.config/plasma-org.kde.plasma.desktop-appletsrc | grep wallpaper
```

---

## 📦 Build de Pacotes (Opcional)

Se quiser buildar apenas os pacotes sem a ISO:

```bash
cd genesi-arch/packages

# Build todos os pacotes
bash build-packages.sh

# Pacotes estarão em repo/
ls -lh repo/*.pkg.tar.zst
```

---

## 🎯 Resultado Final

Após build e boot da ISO, você deve ter:

```
✅ Desktop Genesi com wallpaper correto
✅ Tema escuro aplicado
✅ Widget AI Mode na taskbar
✅ Widget Updates na taskbar (se houver updates)
✅ Pacotes Genesi instalados
✅ Comandos funcionando (checkupdates, etc)
```

---

## 📊 Tempo Estimado

| Etapa | Tempo |
|-------|-------|
| Preparar VM | 5 min |
| Clonar repo | 2 min |
| Build ISO | 20-30 min |
| Testar | 10 min |
| **Total** | **~40-50 min** |

---

## 🚀 Próximos Passos

Depois que tudo estiver funcionando:

1. ✅ Testar em hardware real
2. ✅ Criar release manual no GitHub
3. ✅ Configurar GitHub Actions (depois)
4. ✅ Distribuir ISO

---

## 💡 Dicas

- **Use tmux/screen** para não perder o build se SSH cair
- **Salve logs** com `tee build.log`
- **Teste em VM** antes de hardware real
- **Faça backup** da ISO que funciona

---

**Status**: Guia completo para build manual

**Próximo**: Buildar na VM e testar! 🚀
