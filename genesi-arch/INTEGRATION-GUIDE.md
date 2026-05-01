# 🚀 Guia de Integração Completo - Genesi OS

## ❓ Sua Pergunta

> "Se eu criar a ISO, bootar numa VM e instalar, vai aparecer:
> - Welcome do Genesi?
> - SO bonitinho do mock?
> - Botão da IA?
> - Popup automático de atualizações?"

## ✅ Resposta Atual

### O Que JÁ Funciona (Sem Fazer Nada)

Se você buildar a ISO **AGORA**:

✅ **Genesi branding** - Logo, wallpaper, tema escuro  
✅ **KDE Plasma** customizado  
✅ **AI Mode daemon** - Detecta Ollama e otimiza  
✅ **Widget AI Mode** - Mostra quando IA está rodando  

### ❌ O Que NÃO Funciona (Precisa Integrar)

❌ **genesi-updater** - Não está na ISO  
❌ **Widget de updates** - Não vai aparecer  
❌ **Notificações de update** - Não vão funcionar  
❌ **Repositório** - Ainda não existe no GitHub  

---

## 📋 Passo a Passo para Ter TUDO Funcionando

### Fase 1: Preparar Repositório GitHub (10 min)

#### Opção A: Manual (Primeira Vez)

```bash
# 1. Build os pacotes
cd genesi-arch/packages
bash build-packages.sh

# 2. Ir no GitHub:
# https://github.com/zFreshy/GenesiOS/releases/new

# 3. Criar release:
Tag: packages-latest
Title: Genesi OS Packages - Latest

# 4. Upload estes arquivos de packages/repo/:
- genesi-settings-1.0.0-1-any.pkg.tar.zst
- genesi-kde-settings-1.0.0-1-any.pkg.tar.zst
- genesi-ai-mode-1.0.0-1-any.pkg.tar.zst
- genesi-updater-1.0.0-1-any.pkg.tar.zst
- genesi.db.tar.gz
- genesi.files.tar.gz

# 5. Publicar release
```

#### Opção B: Automático (GitHub Actions)

```bash
# Apenas commit e push
git add .
git commit -m "feat: add auto-update system"
git push origin arch-base

# GitHub Actions faz tudo automaticamente!
# Ver progresso: https://github.com/zFreshy/GenesiOS/actions
```

---

### Fase 2: Adicionar Pacotes à ISO (2 min)

Editar `genesi-arch/archiso/packages_desktop.x86_64`:

```bash
# Remover pacotes CachyOS (se ainda tiver)
#cachyos-settings
#cachyos-kde-settings
#cachyos-hello

# Adicionar pacotes Genesi
genesi-settings
genesi-kde-settings
genesi-ai-mode
genesi-updater
```

---

### Fase 3: Configurar Repositório na ISO (5 min)

Criar arquivo `genesi-arch/archiso/airootfs/etc/pacman.conf.d/genesi.conf`:

```ini
[genesi]
SigLevel = Optional TrustAll
Server = https://github.com/zFreshy/GenesiOS/releases/download/packages-latest
```

Ou editar o `pacman.conf` principal para adicionar o repositório.

---

### Fase 4: Build da ISO (20-30 min)

```bash
cd genesi-arch
sudo ./buildiso.sh -p desktop
```

---

### Fase 5: Testar em VM (15 min)

```bash
# 1. Boot ISO em VirtualBox/VMware
# 2. Instalar sistema
# 3. Reboot
# 4. Verificar tudo funcionando
```

---

## ✅ O Que Você Vai Ver Após Instalar

### 1. Boot e Login
```
┌─────────────────────────────────────┐
│                                     │
│         🌌 GENESI OS                │
│                                     │
│    [Login Screen com tema escuro]   │
│                                     │
└─────────────────────────────────────┘
```

### 2. Desktop
```
┌─────────────────────────────────────────────────────────────┐
│  [Wallpaper Genesi]                                         │
│                                                             │
│  🗑️  📁  ⚙️  📊  🌐                                        │
│  Bin Files Set  Task  Web                                   │
│                                                             │
│                                                             │
│                                                             │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│ [G] | 🦊 📁 ⚙️ 📊 🌐 |  🤖 AI  🔄 3  | 🔋 📶 🇧🇷 14:30 │
│ Dock                    Widgets           System Tray      │
└─────────────────────────────────────────────────────────────┘
```

### 3. Widgets na Taskbar

**AI Mode Widget** (já funciona):
```
┌────┐
│ 🤖 │  ← Verde quando IA rodando
│ AI │
└────┘
```

**Update Widget** (após integração):
```
┌────┐
│ 🔄 │  ← Pulsante quando há updates
│  3 │  ← Badge com número
└────┘
```

### 4. Notificação de Update
```
┌─────────────────────────────────────┐
│ 🔄 3 Updates Available              │
│                                     │
│ Packages: firefox, genesi-ai-mode   │
│                                     │
│ Click to open Discover and update.  │
└─────────────────────────────────────┘
```

### 5. Comandos Disponíveis

```bash
# Verificar updates manualmente
checkupdates

# Forçar verificação do daemon
sudo /usr/local/bin/genesi-updater

# Ver estado de updates
cat /var/lib/genesi-updater/state.json

# Ver status do timer
systemctl status genesi-updater.timer

# Ver logs
sudo journalctl -u genesi-updater -f

# Atualizar sistema
sudo pacman -Syu

# Ou via GUI
plasma-discover --mode Update
```

---

## 🔧 Configuração no GitHub

### Você Precisa Fazer NO GITHUB:

1. **Habilitar GitHub Actions** (se não estiver)
   - Settings → Actions → General
   - Allow all actions

2. **Criar primeiro release** (manual ou via Actions)
   - Releases → New release
   - Tag: `packages-latest`
   - Upload os 6 arquivos

3. **Verificar permissões**
   - Actions deve ter permissão de criar releases
   - Settings → Actions → General → Workflow permissions
   - Marcar "Read and write permissions"

---

## 📦 Estrutura Final do Repositório

```
GitHub Releases:
└── packages-latest/
    ├── genesi-settings-1.0.0-1-any.pkg.tar.zst
    ├── genesi-kde-settings-1.0.0-1-any.pkg.tar.zst
    ├── genesi-ai-mode-1.0.0-1-any.pkg.tar.zst
    ├── genesi-updater-1.0.0-1-any.pkg.tar.zst
    ├── genesi.db.tar.gz
    └── genesi.files.tar.gz

URL do repositório:
https://github.com/zFreshy/GenesiOS/releases/download/packages-latest
```

---

## 🧪 Checklist de Integração

### Antes de Buildar ISO

- [ ] Commit e push do código
- [ ] GitHub Actions rodou com sucesso
- [ ] Release `packages-latest` existe
- [ ] 6 arquivos estão no release
- [ ] Adicionar pacotes ao `packages_desktop.x86_64`
- [ ] Configurar repositório no `pacman.conf`

### Após Buildar ISO

- [ ] ISO gerada sem erros
- [ ] Boot da ISO em VM
- [ ] Instalar sistema
- [ ] Reboot
- [ ] Desktop aparece com tema Genesi
- [ ] Widget AI Mode aparece
- [ ] Widget Update aparece
- [ ] Timer está ativo: `systemctl status genesi-updater.timer`
- [ ] Comando funciona: `checkupdates`
- [ ] Notificação aparece (se houver updates)

---

## 🐛 Troubleshooting

### Release não aparece no GitHub

```bash
# Verificar GitHub Actions
https://github.com/zFreshy/GenesiOS/actions

# Ver logs do workflow
# Clicar no workflow → Ver detalhes

# Criar release manualmente se necessário
```

### Pacotes não instalam na ISO

```bash
# Verificar se repositório está configurado
cat /etc/pacman.conf | grep genesi

# Deve mostrar:
# [genesi]
# SigLevel = Optional TrustAll
# Server = https://github.com/.../packages-latest

# Testar acesso ao repositório
curl -I https://github.com/zFreshy/GenesiOS/releases/download/packages-latest/genesi.db.tar.gz

# Deve retornar: HTTP/1.1 200 OK
```

### Widget não aparece

```bash
# Verificar se pacote foi instalado
pacman -Q genesi-updater

# Verificar arquivos do widget
ls -la /usr/share/plasma/plasmoids/org.genesi.updater/

# Reiniciar Plasma
kquitapp5 plasmashell && kstart5 plasmashell

# Adicionar widget manualmente
# Right-click taskbar → Add Widgets → Genesi Updater
```

---

## 📊 Resumo Visual

```
┌─────────────────────────────────────────────────────────────┐
│                    WORKFLOW COMPLETO                         │
└─────────────────────────────────────────────────────────────┘

1. Você faz:
   git push origin arch-base
   
2. GitHub Actions:
   ├─► Build pacotes
   ├─► Cria release
   └─► Publica no GitHub
   
3. Você edita:
   packages_desktop.x86_64
   (adiciona genesi-updater)
   
4. Você builda:
   sudo ./buildiso.sh -p desktop
   
5. ISO gerada com:
   ├─► Genesi branding ✅
   ├─► AI Mode ✅
   ├─► Update System ✅
   └─► Repositório configurado ✅
   
6. Usuário instala:
   ├─► Desktop bonitinho ✅
   ├─► Widget AI ✅
   ├─► Widget Updates ✅
   └─► Notificações ✅
```

---

## 🎯 Resposta Direta

### Sua Pergunta Original:

> "vai aparecer welcome do genesi, SO bonitinho, botão da IA, popup de atualizações?"

### Resposta:

**DEPOIS de fazer a integração acima**: ✅ SIM!

**AGORA, sem integrar**: ❌ Só AI Mode funciona

**O que falta**: 
1. Criar release no GitHub (1x)
2. Adicionar `genesi-updater` ao `packages_desktop.x86_64`
3. Rebuild ISO

**Tempo total**: ~30-40 minutos

---

## 🚀 Próximo Passo AGORA

```bash
# 1. Commit tudo
git add .
git commit -m "feat: add auto-update system"
git push origin arch-base

# 2. Aguardar GitHub Actions (5 min)
# Ver: https://github.com/zFreshy/GenesiOS/actions

# 3. Verificar release criado
# Ver: https://github.com/zFreshy/GenesiOS/releases

# 4. Editar packages_desktop.x86_64
# Adicionar: genesi-updater

# 5. Rebuild ISO
cd genesi-arch
sudo ./buildiso.sh -p desktop

# 6. Testar em VM
# Boot → Install → Reboot → Verificar tudo! 🎉
```

---

**TL;DR**: Código tá pronto, mas precisa integrar na ISO e criar release no GitHub. Depois disso, TUDO funciona! 🚀
