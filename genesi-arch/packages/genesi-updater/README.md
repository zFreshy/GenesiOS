# genesi-updater

Sistema de atualização automática para Genesi OS.

## Funcionalidades

✅ **Verificação automática** de updates a cada hora  
✅ **Notificações desktop** quando updates disponíveis  
✅ **Widget Plasma** com contador visual  
✅ **Integração com Discover** (GUI nativa)  
✅ **Zero configuração** - funciona out of the box  

## Componentes

- `genesi-updater` - Daemon de verificação
- `genesi-updater.timer` - Systemd timer (1 hora)
- `genesi-update-notifier` - Daemon de notificações
- Plasma widget - Widget visual na taskbar

## Instalação

```bash
sudo pacman -S genesi-updater
```

O timer é habilitado automaticamente após instalação.

## Uso

### Verificação Manual

```bash
# Forçar verificação
sudo /usr/local/bin/genesi-updater

# Ver estado
cat /var/lib/genesi-updater/state.json

# Ver logs
sudo journalctl -u genesi-updater -f
```

### Configuração

Editar `/etc/genesi-updater.conf`:

```ini
# Intervalo em segundos (padrão: 3600 = 1 hora)
check_interval=3600

# Mostrar notificações
notify_user=true

# Incluir AUR (requer yay/paru)
include_aur=false
```

### Habilitar/Desabilitar

```bash
# Desabilitar
sudo systemctl disable --now genesi-updater.timer

# Habilitar
sudo systemctl enable --now genesi-updater.timer

# Status
systemctl status genesi-updater.timer
```

## Widget

O widget aparece automaticamente na taskbar após instalação.

**Funcionalidades**:
- Ícone pulsante quando há updates
- Badge com número de updates
- Popup com lista de pacotes
- Botão "Update All" abre Discover

**Adicionar manualmente**:
1. Right-click na taskbar
2. Add Widgets
3. Procurar "Genesi Updater"
4. Arrastar para taskbar

## Arquivos

```
/usr/local/bin/genesi-updater              # Daemon principal
/usr/local/bin/genesi-update-notifier      # Notificações
/usr/lib/systemd/system/genesi-updater.*   # Systemd units
/etc/genesi-updater.conf                   # Configuração
/var/lib/genesi-updater/state.json         # Estado
/var/log/genesi-updater.log                # Logs
/usr/share/plasma/plasmoids/org.genesi.updater/  # Widget
```

## Troubleshooting

### Notificações não aparecem

```bash
# Verificar se notifier está rodando
ps aux | grep genesi-update-notifier

# Reiniciar
killall genesi-update-notifier
/usr/local/bin/genesi-update-notifier &
```

### Widget não aparece

```bash
# Reiniciar Plasma
kquitapp5 plasmashell && kstart5 plasmashell
```

### Timer não executa

```bash
# Ver próxima execução
systemctl list-timers genesi-updater.timer

# Forçar execução
sudo systemctl start genesi-updater.service
```

## Dependências

- `python` - Runtime
- `python-requests` - HTTP requests
- `python-packaging` - Version parsing
- `pacman-contrib` - checkupdates command
- `libnotify` - Desktop notifications
- `plasma-workspace` - KDE integration

## Licença

GPL-3.0-or-later

## Autor

Genesi OS Team
