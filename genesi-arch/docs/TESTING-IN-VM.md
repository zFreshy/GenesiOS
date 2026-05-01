# 🧪 Testando Genesi OS na VM

## 📋 Pré-requisitos

- VM CachyOS/Arch Linux rodando
- ISO do Genesi OS buildada
- Acesso SSH à VM (opcional)

---

## 🚀 Passo a Passo

### 1. Buildar a ISO (na VM)

```bash
cd ~/GenesiOS/genesi-arch
sudo ./buildiso.sh -p desktop
```

**Tempo**: ~20-30 minutos

**Output**: `out/GenesiOS-YYYYMMDD-HHMM.iso`

---

### 2. Testar ISO em Nova VM

#### VirtualBox

1. Criar nova VM:
   - Nome: Genesi OS Test
   - Tipo: Linux
   - Versão: Arch Linux (64-bit)
   - RAM: 4096 MB (4 GB)
   - Disco: 20 GB

2. Configurações:
   - Sistema → Habilitar EFI
   - Display → Video Memory: 128 MB
   - Armazenamento → Adicionar ISO

3. Boot e instalar

#### VMware

1. Criar nova VM:
   - Guest OS: Linux → Other Linux 5.x kernel 64-bit
   - RAM: 4 GB
   - Disco: 20 GB

2. Adicionar ISO como CD/DVD

3. Boot e instalar

---

### 3. Verificações Após Instalação

#### Desktop e Tema

```bash
# Verificar tema Genesi
cat /etc/os-release | grep Genesi

# Verificar wallpaper
ls /usr/share/wallpapers/genesi/

# Verificar tema KDE
ls /usr/share/color-schemes/ | grep Genesi
```

**Esperado**:
- ✅ Desktop com wallpaper Genesi
- ✅ Tema escuro aplicado
- ✅ Logo Genesi no boot

#### AI Mode

```bash
# Verificar daemon AI Mode
systemctl status genesi-aid

# Ver logs
sudo journalctl -u genesi-aid -n 20

# Testar com Ollama (se instalado)
ollama run llama3.2
# Widget AI Mode deve ficar verde
```

**Esperado**:
- ✅ Daemon rodando
- ✅ Widget AI Mode na taskbar (🤖 AI)
- ✅ Widget fica verde quando IA roda

#### Sistema de Updates

```bash
# Verificar timer de updates
systemctl status genesi-updater.timer

# Ver próxima execução
systemctl list-timers genesi-updater.timer

# Forçar verificação manual
sudo /usr/local/bin/genesi-updater

# Ver estado
cat /var/lib/genesi-updater/state.json

# Verificar updates disponíveis
checkupdates
```

**Esperado**:
- ✅ Timer ativo e rodando
- ✅ Widget Updates na taskbar (🔄)
- ✅ Comando `checkupdates` funciona
- ✅ Notificações aparecem (se houver updates)

#### Repositório Genesi

```bash
# Verificar repositório configurado
cat /etc/pacman.conf | grep -A 2 "\[genesi\]"

# Testar acesso ao repositório
curl -I https://github.com/zFreshy/GenesiOS/releases/download/packages-latest/genesi.db.tar.gz

# Sincronizar database
sudo pacman -Sy

# Ver pacotes Genesi instalados
pacman -Q | grep genesi
```

**Esperado**:
```
genesi-settings 1.0.0-1
genesi-kde-settings 1.0.0-1
genesi-ai-mode 1.0.0-1
genesi-updater 1.0.0-1
```

---

### 4. Testar Funcionalidades

#### Notificações de Update

```bash
# Simular update disponível
echo "3" | sudo tee /tmp/genesi-updates-available

# Aguardar ~5 segundos
# Notificação deve aparecer
```

#### Widget Updates

1. Clicar no widget 🔄 na taskbar
2. Popup deve abrir mostrando:
   - Número de updates
   - Lista de pacotes
   - Botão "Update All"
   - Botão "Check"

#### Discover Integration

```bash
# Abrir Discover
plasma-discover --mode Update

# Ou clicar em "Update All" no widget
```

**Esperado**:
- ✅ Discover abre na aba Updates
- ✅ Lista de updates aparece
- ✅ Botão "Update All" funciona

#### AI Mode com Ollama

```bash
# Instalar Ollama
curl -fsSL https://ollama.ai/install.sh | sh

# Baixar modelo
ollama pull llama3.2

# Rodar
ollama run llama3.2

# Verificar otimizações
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
# Deve mostrar: performance

cat /proc/sys/vm/swappiness
# Deve mostrar: 10
```

---

### 5. Checklist de Testes

#### Visual
- [ ] Desktop com wallpaper Genesi
- [ ] Tema escuro aplicado
- [ ] Ícones no desktop
- [ ] Dock na parte inferior
- [ ] Widgets na taskbar

#### Widgets
- [ ] Widget AI Mode aparece (🤖 AI)
- [ ] Widget Updates aparece (🔄)
- [ ] Widgets respondem ao clique
- [ ] Popups funcionam

#### AI Mode
- [ ] Daemon genesi-aid rodando
- [ ] Detecta Ollama/llama.cpp
- [ ] Aplica otimizações
- [ ] Widget fica verde quando IA roda
- [ ] Volta ao normal quando IA para

#### Updates
- [ ] Timer genesi-updater ativo
- [ ] Comando checkupdates funciona
- [ ] Notificações aparecem
- [ ] Widget mostra contador
- [ ] Discover abre corretamente
- [ ] Updates instalam via Discover

#### Repositório
- [ ] Repositório Genesi configurado
- [ ] Pacman sincroniza database
- [ ] Pacotes Genesi instalados
- [ ] Updates funcionam via pacman

---

### 6. Logs Úteis

```bash
# Logs do sistema
sudo journalctl -b

# Logs AI Mode
sudo journalctl -u genesi-aid -f

# Logs Updates
sudo journalctl -u genesi-updater -f
tail -f /var/log/genesi-updater.log

# Logs Plasma
journalctl --user -u plasma-plasmashell -f

# Logs pacman
tail -f /var/log/pacman.log
```

---

### 7. Troubleshooting

#### Widget não aparece

```bash
# Reiniciar Plasma
kquitapp5 plasmashell && kstart5 plasmashell

# Adicionar widget manualmente
# Right-click taskbar → Add Widgets → Procurar "Genesi"
```

#### Daemon não roda

```bash
# Ver erro
sudo systemctl status genesi-aid
sudo systemctl status genesi-updater

# Reiniciar
sudo systemctl restart genesi-aid
sudo systemctl restart genesi-updater.timer
```

#### Notificações não aparecem

```bash
# Testar notify-send
notify-send "Test" "This is a test"

# Verificar notifier rodando
ps aux | grep genesi-update-notifier

# Reiniciar
killall genesi-update-notifier
/usr/local/bin/genesi-update-notifier &
```

#### Repositório não acessível

```bash
# Testar conectividade
ping github.com

# Testar URL do repositório
curl -I https://github.com/zFreshy/GenesiOS/releases/download/packages-latest/genesi.db.tar.gz

# Forçar refresh
sudo pacman -Syy
```

---

## 📊 Resultado Esperado

Após todos os testes, você deve ter:

```
✅ Desktop Genesi funcionando
✅ Tema e wallpaper aplicados
✅ Widget AI Mode operacional
✅ Widget Updates operacional
✅ Notificações funcionando
✅ Repositório acessível
✅ Updates via Discover funcionando
✅ AI Mode detectando e otimizando
```

---

## 🎉 Sucesso!

Se todos os itens acima funcionam, o Genesi OS está **100% operacional**!

---

**Próximo passo**: Testar em hardware real (opcional)

**Documentação**: Ver `AUTO-UPDATE-SYSTEM.md` para detalhes
