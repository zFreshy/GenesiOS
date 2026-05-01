# 🚀 Quick Start - Sistema de Atualização Automática

## TL;DR

```bash
# 1. Build packages
cd genesi-arch/packages
bash build-packages.sh

# 2. Test locally (opcional)
sudo pacman -U repo/genesi-updater-*.pkg.tar.zst

# 3. Push to GitHub
git add .
git commit -m "feat: add auto-update system"
git push origin arch-base

# 4. GitHub Actions faz o resto automaticamente! 🎉
```

---

## O Que Você Acabou de Criar

### 🎯 Sistema Completo de Updates

✅ **Daemon** que verifica updates a cada hora  
✅ **Notificações** desktop quando há updates  
✅ **Widget** visual na taskbar com contador  
✅ **Integração** com KDE Discover (GUI)  
✅ **GitHub Actions** para publicar automaticamente  
✅ **Zero config** - funciona out of the box  

---

## Como Funciona

### Para o Usuário

1. Sistema instalado → Timer roda a cada hora
2. Updates disponíveis → Notificação aparece
3. Usuário clica → Discover abre
4. Clica "Update All" → Instalado!

### Para Você (Desenvolvedor)

1. Edita pacote → Incrementa versão
2. Commit + push → GitHub Actions detecta
3. Build automático → Release criado
4. Usuários recebem → Update automático

---

## Arquivos Criados

```
genesi-arch/
├── packages/
│   ├── genesi-updater/          ← Novo pacote!
│   │   ├── PKGBUILD
│   │   ├── genesi-updater       (daemon Python)
│   │   ├── genesi-updater.service
│   │   ├── genesi-updater.timer
│   │   ├── genesi-updater.conf
│   │   ├── genesi-update-notifier
│   │   ├── plasmoid/            (widget KDE)
│   │   └── README.md
│   └── build-packages.sh        ← Build script
├── docs/
│   ├── AUTO-UPDATE-SYSTEM.md    ← Documentação completa
│   ├── TEST-AUTO-UPDATE.md      ← Guia de testes
│   └── WORKFLOW-AUTO-UPDATE-SUMMARY.md
└── test-auto-update.sh          ← Script de teste

.github/workflows/
└── publish-packages.yml         ← GitHub Actions
```

---

## Próximos Passos

### 1️⃣ Testar Localmente (Opcional)

```bash
# Build
cd genesi-arch/packages
bash build-packages.sh

# Instalar
sudo pacman -U repo/genesi-updater-*.pkg.tar.zst

# Verificar
systemctl status genesi-updater.timer
sudo /usr/local/bin/genesi-updater
cat /var/lib/genesi-updater/state.json
```

### 2️⃣ Publicar no GitHub

```bash
# Commit
git add .
git commit -m "feat: add auto-update system with notifications and widget"
git push origin arch-base

# Ver GitHub Actions
# https://github.com/zFreshy/GenesiOS/actions
```

### 3️⃣ Integrar na ISO

Adicionar ao `packages_desktop.x86_64`:
```
genesi-updater
```

Rebuild ISO:
```bash
sudo ./buildiso.sh -p desktop
```

### 4️⃣ Testar em VM

1. Boot ISO em VM
2. Verificar: `systemctl status genesi-updater.timer`
3. Verificar widget na taskbar
4. Simular update disponível
5. Testar notificação e Discover

---

## Comandos Úteis

### Ver Status
```bash
# Timer
systemctl status genesi-updater.timer
systemctl list-timers genesi-updater.timer

# Logs
sudo journalctl -u genesi-updater -f
tail -f /var/log/genesi-updater.log

# Estado
cat /var/lib/genesi-updater/state.json
cat /tmp/genesi-updates-available
```

### Forçar Verificação
```bash
sudo /usr/local/bin/genesi-updater
```

### Reiniciar Widget
```bash
kquitapp5 plasmashell && kstart5 plasmashell
```

---

## Troubleshooting

### ❌ Daemon não roda
```bash
sudo /usr/local/bin/genesi-updater
# Ver erro detalhado
```

### ❌ Notificações não aparecem
```bash
ps aux | grep genesi-update-notifier
killall genesi-update-notifier
/usr/local/bin/genesi-update-notifier &
```

### ❌ Widget não aparece
```bash
ls -la /usr/share/plasma/plasmoids/org.genesi.updater/
kquitapp5 plasmashell && kstart5 plasmashell
```

### ❌ GitHub Actions falha
- Ver logs em: https://github.com/zFreshy/GenesiOS/actions
- Verificar se container Arch Linux está OK
- Verificar se dependências estão corretas

---

## Documentação Completa

- **Sistema completo**: `docs/AUTO-UPDATE-SYSTEM.md`
- **Guia de testes**: `docs/TEST-AUTO-UPDATE.md`
- **Resumo**: `docs/WORKFLOW-AUTO-UPDATE-SUMMARY.md`
- **Pacote**: `packages/genesi-updater/README.md`

---

## 🎉 Pronto!

Você agora tem um **sistema profissional de atualização automática**!

**Diferencial**: Poucos distros Linux têm isso integrado nativamente.

**Benefícios**:
- ✅ Usuários sempre atualizados
- ✅ Zero manutenção manual
- ✅ Publicação automática via GitHub
- ✅ Interface visual moderna
- ✅ Gratuito (GitHub Releases)

---

**Dúvidas?** Veja a documentação completa em `docs/AUTO-UPDATE-SYSTEM.md`

**Bugs?** Abra uma issue no GitHub

**Sugestões?** Pull requests são bem-vindos!

---

**Status**: ✅ PRONTO PARA USAR

**Versão**: 1.0.0

**Data**: 2026-05-01
