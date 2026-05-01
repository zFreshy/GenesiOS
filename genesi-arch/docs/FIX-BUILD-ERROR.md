# 🔧 Fix: Build ISO Error - genesi.db not found

## ❌ Erro

```
Failed retrieving file 'genesi.db' from disk
Could not open file /root/genesi-arch/local-repo/genesi.db
```

## 🔍 Causa

Você tentou buildar a ISO **sem buildar os pacotes Genesi primeiro**.

O `pacman.conf` está configurado para usar o repositório local:

```ini
[genesi]
SigLevel = Optional TrustAll
Server = file:///root/genesi-arch/local-repo
```

Mas esse repositório não existe ainda porque os pacotes não foram buildados!

## ✅ Solução

### Passo 1: Buildar os Pacotes Genesi

```bash
cd ~/GenesiOS/genesi-arch

# Executar script de build
bash build-local-packages.sh
```

**O que vai acontecer:**
- Vai buildar 4 pacotes:
  - `genesi-settings`
  - `genesi-kde-settings`
  - `genesi-ai-mode`
  - `genesi-updater`
- Vai criar diretório `local-repo/`
- Vai criar database `genesi.db.tar.gz`
- Vai mover os `.pkg.tar.zst` para `local-repo/`

**Output esperado:**
```
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🔨 Building Genesi OS Packages Locally
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Building: genesi-settings
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ Built: genesi-settings

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Building: genesi-kde-settings
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ Built: genesi-kde-settings

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Building: genesi-ai-mode
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ Built: genesi-ai-mode

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Building: genesi-updater
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ Built: genesi-updater

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
Creating repository database...
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

✅ Repository database created!

=== Built Packages ===
-rw-r--r-- 1 user user 2.5K genesi-settings-1.0.0-1-any.pkg.tar.zst
-rw-r--r-- 1 user user 3.2K genesi-kde-settings-1.0.0-2-any.pkg.tar.zst
-rw-r--r-- 1 user user 4.1K genesi-ai-mode-1.0.0-1-any.pkg.tar.zst
-rw-r--r-- 1 user user 5.8K genesi-updater-1.0.0-1-any.pkg.tar.zst

=== Repository Files ===
-rw-r--r-- 1 user user 1.2K genesi.db.tar.gz
-rw-r--r-- 1 user user 2.4K genesi.files.tar.gz

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
✅ All packages built successfully!
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Local repository: /root/genesi-arch/local-repo

Next step: Run buildiso.sh to build the ISO with these packages
```

### Passo 2: Verificar Repositório Local

```bash
# Verificar se os arquivos foram criados
ls -lh local-repo/

# Deve mostrar:
# genesi-settings-1.0.0-1-any.pkg.tar.zst
# genesi-kde-settings-1.0.0-2-any.pkg.tar.zst
# genesi-ai-mode-1.0.0-1-any.pkg.tar.zst
# genesi-updater-1.0.0-1-any.pkg.tar.zst
# genesi.db.tar.gz
# genesi.files.tar.gz
```

### Passo 3: Agora Sim, Buildar a ISO

```bash
# Agora pode buildar a ISO
sudo ./buildiso.sh -p desktop
```

## 🐛 Se o Build dos Pacotes Falhar

### Erro: "makepkg: command not found"

```bash
sudo pacman -S base-devel
```

### Erro: "Permission denied"

```bash
# Não use sudo para buildar pacotes!
# makepkg não pode rodar como root
bash build-local-packages.sh  # SEM sudo
```

### Erro em um pacote específico

```bash
# Ir para o diretório do pacote
cd packages/genesi-settings  # ou outro que falhou

# Ver o que está errado
cat PKGBUILD

# Tentar buildar manualmente
makepkg -sf --noconfirm

# Ver erros detalhados
```

### Erro: "No such file or directory"

Verifique se os arquivos fonte existem:

```bash
# Para genesi-settings
ls -la packages/genesi-settings/
# Deve ter: PKGBUILD, os-release, issue, etc.

# Para genesi-kde-settings
ls -la packages/genesi-kde-settings/usr/share/wallpapers/genesi/
# Deve ter: wallpaper.png, metadata.desktop

# Para genesi-ai-mode
ls -la packages/genesi-ai-mode/
# Deve ter: PKGBUILD, genesi-aid, genesi-aid.service, etc.

# Para genesi-updater
ls -la packages/genesi-updater/
# Deve ter: PKGBUILD, genesi-updater, genesi-updater.service, etc.
```

## 📝 Ordem Correta de Build

```
1. Build pacotes Genesi
   ↓
2. Criar repositório local
   ↓
3. Build ISO (que vai usar o repositório local)
```

**NUNCA** pule o passo 1!

## 🎯 Resumo

```bash
# 1. Buildar pacotes (OBRIGATÓRIO)
cd ~/GenesiOS/genesi-arch
bash build-local-packages.sh

# 2. Verificar
ls -lh local-repo/

# 3. Buildar ISO
sudo ./buildiso.sh -p desktop
```

## 💡 Dica

Se você quiser usar apenas pacotes do CachyOS (sem os pacotes Genesi), você pode:

1. Comentar a seção `[genesi]` no `archiso/pacman.conf`
2. Comentar os pacotes Genesi no `archiso/packages_desktop.x86_64`

Mas aí você não terá:
- ❌ Branding do Genesi
- ❌ AI Mode
- ❌ Sistema de updates
- ❌ Tema customizado

Por isso é melhor buildar os pacotes! 😉
