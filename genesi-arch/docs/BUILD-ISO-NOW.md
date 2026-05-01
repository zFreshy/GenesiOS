# 🚀 Como Buildar a ISO do Genesi OS AGORA

## 📋 Resumo do Que Temos

✅ **Sistema de Auto-Update** - Completo e funcional
✅ **AI Mode** - Detecta e otimiza processos de IA
✅ **KDE Settings** - Tema customizado verde/teal
✅ **Wallpaper** - Genesi wallpaper configurado
✅ **Calamares** - Usando `cachyos-calamares-next` (funciona!)
✅ **Pacotes Genesi** - Todos criados e prontos

## 🎯 Objetivo

Buildar a ISO do Genesi OS na VM e testar!

## 📝 Passo a Passo

### 1. Na VM CachyOS

```bash
# Atualizar repositório
cd ~/GenesiOS
git pull origin main

# Ir para o diretório genesi-arch
cd genesi-arch
```

### 2. Buildar Pacotes Localmente

```bash
# Executar script de build
bash build-local-packages.sh
```

**O que vai acontecer:**
- Vai buildar 4 pacotes:
  - `genesi-settings`
  - `genesi-kde-settings`
  - `genesi-ai-mode`
  - `genesi-updater`
- Vai criar repositório local em `local-repo/`
- Vai criar database `genesi.db.tar.gz`

**Tempo estimado:** 2-5 minutos

### 3. Buildar ISO

```bash
# Buildar ISO (vai demorar 20-30 minutos)
sudo ./buildiso.sh -p desktop
```

**O que vai acontecer:**
- Vai usar pacman.conf com repositório local Genesi
- Vai instalar pacotes do CachyOS + Genesi
- Vai usar `cachyos-calamares-next` como instalador
- Vai gerar ISO em `out/GenesiOS-YYYYMMDD-HHMM.iso`

**Tempo estimado:** 20-30 minutos

### 4. Testar ISO

#### Opção A: QEMU (rápido)

```bash
qemu-system-x86_64 \
  -m 4096 \
  -smp 4 \
  -cdrom out/GenesiOS-*.iso \
  -boot d \
  -enable-kvm
```

#### Opção B: VirtualBox (melhor para testar instalação)

1. Copiar ISO para Windows
2. Criar VM no VirtualBox
3. Montar ISO
4. Bootar e testar

## ✅ O Que Verificar Após Boot

### No Live ISO

```bash
# 1. Verificar OS
cat /etc/os-release
# Deve mostrar: NAME="Genesi OS"

# 2. Verificar pacotes Genesi
pacman -Q | grep genesi
# Deve mostrar:
# genesi-settings 1.0.0-1
# genesi-kde-settings 1.0.0-2
# genesi-ai-mode 1.0.0-1
# genesi-updater 1.0.0-1

# 3. Verificar wallpaper
ls /usr/share/wallpapers/genesi/
# Deve ter: wallpaper.png e metadata.desktop

# 4. Verificar AI Mode daemon
systemctl status genesi-aid
# Deve estar rodando

# 5. Verificar update timer
systemctl status genesi-updater.timer
# Deve estar ativo
```

### No Desktop

- ✅ Wallpaper do Genesi aparecendo
- ✅ Tema verde/teal aplicado
- ✅ Widget AI Mode na taskbar (se tiver processo de IA rodando)
- ✅ Widget Updates na taskbar (se houver updates)
- ✅ Instalador Calamares abre (mesmo que seja do CachyOS)

### Após Instalação

```bash
# Instalar o sistema usando Calamares
# Depois de reiniciar:

# 1. Verificar OS
cat /etc/os-release

# 2. Verificar pacotes
pacman -Q | grep genesi

# 3. Verificar updates
checkupdates

# 4. Verificar AI Mode
systemctl status genesi-aid

# 5. Testar AI Mode
# Rodar algum processo de IA (se tiver)
# Verificar se otimizações são aplicadas
```

## 🐛 Troubleshooting

### Build de pacotes falha

```bash
# Ver qual pacote falhou
# Ir para o diretório do pacote
cd packages/genesi-settings  # ou outro que falhou

# Tentar buildar manualmente
makepkg -sf --noconfirm

# Ver erros
```

### Build da ISO falha

```bash
# Ver logs
cat build.log

# Problemas comuns:
# 1. Pacotes não encontrados → Verificar local-repo/
# 2. Permissões → Usar sudo
# 3. Espaço em disco → Limpar /tmp
```

### Wallpaper não aparece

```bash
# Verificar arquivo
ls -lh /usr/share/wallpapers/genesi/wallpaper.png

# Verificar config KDE
cat ~/.config/plasma-org.kde.plasma.desktop-appletsrc | grep wallpaper
```

### AI Mode não funciona

```bash
# Ver logs
sudo journalctl -u genesi-aid -n 50

# Verificar se daemon está rodando
systemctl status genesi-aid

# Reiniciar daemon
sudo systemctl restart genesi-aid
```

## 📊 Checklist Final

Antes de considerar a ISO pronta:

- [ ] ISO boota no QEMU
- [ ] ISO boota no VirtualBox
- [ ] Desktop aparece com wallpaper correto
- [ ] Tema verde/teal aplicado
- [ ] Pacotes Genesi instalados
- [ ] AI Mode daemon rodando
- [ ] Update timer ativo
- [ ] Calamares abre e funciona
- [ ] Instalação completa sem erros
- [ ] Sistema instalado boota
- [ ] Todas as features funcionam após instalação

## 🎉 Próximos Passos Após ISO Funcionar

1. ✅ Testar em hardware real
2. ✅ Criar release no GitHub
3. ✅ Distribuir ISO
4. ⏳ Criar slides customizados para Calamares
5. ⏳ Compilar `genesi-calamares` customizado
6. ⏳ Substituir `cachyos-calamares-next` por `genesi-calamares`

## 💡 Dicas

- **Use tmux/screen** para não perder o build se SSH cair
- **Salve logs** com `tee build.log`
- **Teste em VM** antes de hardware real
- **Faça backup** da ISO que funciona
- **Documente problemas** que encontrar

## 🔗 Documentação Relacionada

- `MANUAL-BUILD-GUIDE.md` - Guia detalhado de build
- `TESTING-IN-VM.md` - Como testar na VM
- `CALAMARES-SETUP.md` - Setup do Calamares customizado
- `CALAMARES-TODO.md` - O que falta fazer no Calamares

---

## 🚀 TL;DR - Comandos Rápidos

```bash
# Na VM
cd ~/GenesiOS/genesi-arch

# Build pacotes
bash build-local-packages.sh

# Build ISO
sudo ./buildiso.sh -p desktop

# Testar
qemu-system-x86_64 -m 4096 -smp 4 -cdrom out/GenesiOS-*.iso -boot d -enable-kvm
```

**Tempo total:** ~30-40 minutos

**Resultado:** ISO do Genesi OS funcionando! 🎉
