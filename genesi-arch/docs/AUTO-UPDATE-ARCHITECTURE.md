# Arquitetura do Sistema de Atualização Automática

## 🏗️ Visão Geral

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         GENESI OS AUTO-UPDATE SYSTEM                     │
└─────────────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────────────┐
│                              GITHUB SIDE                                 │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  Developer                                                               │
│     │                                                                    │
│     ├─► Edit PKGBUILD (bump version)                                    │
│     │                                                                    │
│     ├─► git commit -m "feat: update package"                            │
│     │                                                                    │
│     └─► git push origin arch-base                                       │
│              │                                                           │
│              ▼                                                           │
│  ┌──────────────────────────────────────────────────────────┐          │
│  │         GitHub Actions Workflow                          │          │
│  │  (.github/workflows/publish-packages.yml)                │          │
│  ├──────────────────────────────────────────────────────────┤          │
│  │  1. Detect push to packages/                             │          │
│  │  2. Spin up Arch Linux container                         │          │
│  │  3. Install build dependencies                           │          │
│  │  4. Run build-packages.sh                                │          │
│  │  5. Create release with timestamp                        │          │
│  │  6. Upload .pkg.tar.zst files                            │          │
│  │  7. Upload genesi.db.tar.gz                              │          │
│  │  8. Update packages-latest release                       │          │
│  └──────────────────────────────────────────────────────────┘          │
│              │                                                           │
│              ▼                                                           │
│  ┌──────────────────────────────────────────────────────────┐          │
│  │         GitHub Releases (CDN)                            │          │
│  │  https://github.com/.../releases/download/packages-*     │          │
│  ├──────────────────────────────────────────────────────────┤          │
│  │  • genesi-settings-1.0.0-1-any.pkg.tar.zst              │          │
│  │  • genesi-kde-settings-1.0.0-1-any.pkg.tar.zst          │          │
│  │  • genesi-ai-mode-1.0.0-1-any.pkg.tar.zst               │          │
│  │  • genesi-updater-1.0.0-1-any.pkg.tar.zst               │          │
│  │  • genesi.db.tar.gz                                      │          │
│  │  • genesi.files.tar.gz                                   │          │
│  └──────────────────────────────────────────────────────────┘          │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘

                                    │
                                    │ HTTP GET
                                    ▼

┌─────────────────────────────────────────────────────────────────────────┐
│                              USER SIDE                                   │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                          │
│  ┌──────────────────────────────────────────────────────────┐          │
│  │         Systemd Timer                                    │          │
│  │  (genesi-updater.timer)                                  │          │
│  ├──────────────────────────────────────────────────────────┤          │
│  │  • Runs every 1 hour                                     │          │
│  │  • First run: 5 min after boot                           │          │
│  │  • Randomized delay: ±10 min                             │          │
│  │  • Persistent (runs missed checks)                       │          │
│  └──────────────────────────────────────────────────────────┘          │
│              │                                                           │
│              ▼ (triggers)                                                │
│  ┌──────────────────────────────────────────────────────────┐          │
│  │         Update Checker Daemon                            │          │
│  │  (/usr/local/bin/genesi-updater)                         │          │
│  ├──────────────────────────────────────────────────────────┤          │
│  │  1. Run checkupdates (no root needed)                    │          │
│  │  2. Parse output (package list)                          │          │
│  │  3. Save to state.json                                   │          │
│  │  4. Create flag file if updates found                    │          │
│  │  5. Send desktop notification                            │          │
│  └──────────────────────────────────────────────────────────┘          │
│              │                                                           │
│              ├─► /var/lib/genesi-updater/state.json                     │
│              │   {                                                       │
│              │     "last_check": "2026-05-01T14:30:00",                 │
│              │     "updates_available": 3,                              │
│              │     "packages": [...]                                    │
│              │   }                                                       │
│              │                                                           │
│              └─► /tmp/genesi-updates-available                          │
│                  "3"                                                     │
│                                                                          │
│  ┌──────────────────────────────────────────────────────────┐          │
│  │         Notification Daemon                              │          │
│  │  (/usr/local/bin/genesi-update-notifier)                 │          │
│  ├──────────────────────────────────────────────────────────┤          │
│  │  • Runs on user login (autostart)                        │          │
│  │  • Monitors flag file every 5 min                        │          │
│  │  • Shows notification when count changes                 │          │
│  │  • Uses libnotify (notify-send)                          │          │
│  └──────────────────────────────────────────────────────────┘          │
│              │                                                           │
│              ▼                                                           │
│  ┌──────────────────────────────────────────────────────────┐          │
│  │         Desktop Notification                             │          │
│  │  ┌────────────────────────────────────────────┐          │          │
│  │  │ 🔄 3 Updates Available                     │          │          │
│  │  │                                            │          │          │
│  │  │ Packages: genesi-ai-mode, firefox, ...    │          │          │
│  │  │                                            │          │          │
│  │  │ Click to open Discover and update.        │          │          │
│  │  └────────────────────────────────────────────┘          │          │
│  └──────────────────────────────────────────────────────────┘          │
│                                                                          │
│  ┌──────────────────────────────────────────────────────────┐          │
│  │         Plasma Widget                                    │          │
│  │  (/usr/share/plasma/plasmoids/org.genesi.updater/)       │          │
│  ├──────────────────────────────────────────────────────────┤          │
│  │  • Reads state.json every 5 seconds                      │          │
│  │  • Shows pulsing icon when updates available             │          │
│  │  • Badge with update count                               │          │
│  │  • Popup with package list                               │          │
│  │  • "Update All" button → opens Discover                  │          │
│  │  • "Check" button → runs daemon manually                 │          │
│  └──────────────────────────────────────────────────────────┘          │
│              │                                                           │
│              ▼ (user clicks "Update All")                                │
│  ┌──────────────────────────────────────────────────────────┐          │
│  │         KDE Discover                                     │          │
│  │  (plasma-discover --mode Update)                         │          │
│  ├──────────────────────────────────────────────────────────┤          │
│  │  • Shows list of updates                                 │          │
│  │  • User clicks "Update All"                              │          │
│  │  • Calls pacman -Syu                                     │          │
│  │  • Installs updates                                      │          │
│  │  • Shows progress                                        │          │
│  └──────────────────────────────────────────────────────────┘          │
│              │                                                           │
│              ▼                                                           │
│  ┌──────────────────────────────────────────────────────────┐          │
│  │         Pacman                                           │          │
│  │  (/usr/bin/pacman)                                       │          │
│  ├──────────────────────────────────────────────────────────┤          │
│  │  1. Download packages from GitHub Releases               │          │
│  │  2. Verify integrity                                     │          │
│  │  3. Install packages                                     │          │
│  │  4. Run post-install scripts                             │          │
│  │  5. Update system                                        │          │
│  └──────────────────────────────────────────────────────────┘          │
│              │                                                           │
│              ▼                                                           │
│         ✅ System Updated!                                              │
│                                                                          │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 🔄 Fluxo de Dados

### 1. Publicação de Update (Developer → GitHub)

```
Developer
   │
   ├─► Edit PKGBUILD (pkgrel++)
   │
   ├─► git commit + push
   │
   └─► GitHub Actions
        │
        ├─► Build in container
        │
        ├─► Create release
        │
        └─► Upload to GitHub Releases
             │
             └─► CDN distributes worldwide
```

### 2. Detecção de Update (System → User)

```
Timer (every 1h)
   │
   └─► genesi-updater daemon
        │
        ├─► checkupdates (query pacman)
        │
        ├─► Parse results
        │
        ├─► Save state.json
        │
        ├─► Create flag file
        │
        └─► notify-send
             │
             └─► Desktop notification appears
```

### 3. Instalação de Update (User → System)

```
User clicks notification/widget
   │
   └─► Discover opens
        │
        ├─► Shows update list
        │
        └─► User clicks "Update All"
             │
             └─► pacman -Syu
                  │
                  ├─► Download from GitHub
                  │
                  ├─► Install packages
                  │
                  └─► System updated ✅
```

---

## 📁 Estrutura de Arquivos

### Sistema de Arquivos

```
/usr/local/bin/
├── genesi-updater              # Daemon principal (Python)
└── genesi-update-notifier      # Daemon de notificações (Bash)

/usr/lib/systemd/system/
├── genesi-updater.service      # Systemd service
└── genesi-updater.timer        # Systemd timer

/etc/
├── genesi-updater.conf         # Configuração
└── xdg/autostart/
    └── genesi-update-notifier.desktop  # Autostart

/var/lib/genesi-updater/
└── state.json                  # Estado persistente

/var/log/
└── genesi-updater.log          # Logs

/tmp/
├── genesi-updates-available    # Flag file (contador)
└── genesi-last-notified        # Última notificação

/usr/share/plasma/plasmoids/org.genesi.updater/
├── metadata.json               # Widget metadata
└── contents/ui/
    └── main.qml                # Widget UI
```

---

## 🔌 Componentes e Responsabilidades

### 1. genesi-updater (Daemon)
**Linguagem**: Python  
**Execução**: Via systemd timer (1h)  
**Responsabilidades**:
- Verificar updates disponíveis
- Salvar estado em JSON
- Criar flag file
- Enviar notificação inicial

### 2. genesi-updater.timer (Systemd Timer)
**Tipo**: Systemd timer unit  
**Intervalo**: 1 hora  
**Responsabilidades**:
- Agendar execuções do daemon
- Garantir execução após boot
- Randomizar para evitar sobrecarga

### 3. genesi-update-notifier (Notifier)
**Linguagem**: Bash  
**Execução**: Autostart no login  
**Responsabilidades**:
- Monitorar flag file
- Enviar notificações desktop
- Evitar notificações duplicadas

### 4. Plasma Widget
**Linguagem**: QML  
**Execução**: Plasma Shell  
**Responsabilidades**:
- Ler state.json periodicamente
- Mostrar ícone e badge
- Popup com lista de updates
- Abrir Discover ao clicar

### 5. GitHub Actions
**Linguagem**: YAML  
**Execução**: GitHub runners  
**Responsabilidades**:
- Build de pacotes
- Criação de releases
- Upload de arquivos
- Atualização de database

---

## 🔐 Segurança

### Permissões

```
/usr/local/bin/genesi-updater
  Owner: root:root
  Perms: 755 (rwxr-xr-x)
  
/var/lib/genesi-updater/
  Owner: nobody:nobody
  Perms: 755 (rwxr-xr-x)
  
/etc/genesi-updater.conf
  Owner: root:root
  Perms: 644 (rw-r--r--)
```

### Systemd Hardening

```ini
[Service]
User=nobody
Group=nobody
PrivateTmp=yes
NoNewPrivileges=yes
ProtectSystem=strict
ProtectHome=yes
ReadWritePaths=/var/lib/genesi-updater /var/log /tmp
```

### Pacman Repository

```ini
[genesi]
SigLevel = Optional TrustAll  # Para desenvolvimento
# Produção: SigLevel = Required DatabaseOptional
Server = https://github.com/.../releases/download/packages-latest
```

---

## 📊 Performance

### Recursos Utilizados

| Componente | RAM | CPU | Disco |
|------------|-----|-----|-------|
| genesi-updater | <10MB | <1% | <1MB |
| genesi-update-notifier | <5MB | <0.1% | 0 |
| Plasma Widget | <20MB | <0.5% | <1MB |
| **Total** | **<35MB** | **<2%** | **<2MB** |

### Tempos de Execução

| Operação | Tempo |
|----------|-------|
| Verificação de updates | <5s |
| Notificação desktop | <1s |
| Widget refresh | <0.5s |
| Build de pacotes (CI) | ~5min |
| Download de update | ~10s |

---

## 🔄 Estados do Sistema

```
┌─────────────────┐
│  No Updates     │  ← Estado inicial
│  (Check verde)  │
└────────┬────────┘
         │
         │ Timer dispara
         ▼
┌─────────────────┐
│  Checking...    │
│  (Loading)      │
└────────┬────────┘
         │
         ├─► Updates found
         │   │
         │   ▼
         │  ┌─────────────────┐
         │  │  Updates Avail  │
         │  │  (Badge + Pulse)│
         │  └────────┬────────┘
         │           │
         │           │ User clicks "Update All"
         │           ▼
         │  ┌─────────────────┐
         │  │  Updating...    │
         │  │  (Progress)     │
         │  └────────┬────────┘
         │           │
         │           ▼
         └──────────────────────► Back to "No Updates"
```

---

## 🐛 Error Handling

### Daemon Errors

```python
try:
    updates = check_updates()
except subprocess.TimeoutExpired:
    log.error("Update check timed out")
    return None
except Exception as e:
    log.error(f"Error: {e}")
    return None
```

### Network Errors

- Timeout após 60s
- Retry automático no próximo timer
- Logs detalhados em `/var/log/genesi-updater.log`

### Widget Errors

- Fallback para estado "unknown"
- Não crasha Plasma Shell
- Mostra mensagem de erro no popup

---

## 🔮 Futuras Melhorias

- [ ] Suporte a AUR packages
- [ ] Auto-update de pacotes Genesi (seguro)
- [ ] Changelog no widget
- [ ] Rollback de updates
- [ ] Estatísticas de uso
- [ ] Mirror servers
- [ ] GPG signing de pacotes
- [ ] Notificações push (opcional)
- [ ] Update scheduling (agendar para depois)
- [ ] Bandwidth throttling

---

**Versão**: 1.0.0  
**Última atualização**: 2026-05-01  
**Status**: ✅ Implementado
