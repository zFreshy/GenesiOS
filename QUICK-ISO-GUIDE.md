# ⚡ Guia Rápido: Criar ISO do Genesi OS

## 🚫 Problema: WSL não funciona!

```
❌ WSL → sudo chroot → "unable to allocate pty: No such device"
```

**Solução:** Usar Linux nativo (VM ou cloud)

---

## ✅ Solução Rápida (30 minutos)

### 1️⃣ Instalar VirtualBox
- Download: https://www.virtualbox.org/
- Instalar no Windows

### 2️⃣ Baixar Ubuntu
- Download: https://ubuntu.com/download/desktop
- Versão: 22.04 LTS

### 3️⃣ Criar VM
```
VirtualBox → Novo
├─ Nome: Ubuntu-Build
├─ RAM: 4 GB
├─ Disco: 30 GB
└─ Adicionar ISO do Ubuntu
```

### 4️⃣ Instalar Ubuntu na VM
- Iniciar VM
- Seguir instalação
- Reiniciar

### 5️⃣ Transferir Código
**Opção A: Git**
```bash
git clone https://github.com/seu-usuario/GenesiOS.git
```

**Opção B: Pasta Compartilhada**
```
VirtualBox → Configurações → Pastas Compartilhadas
Adicionar: C:\caminho\GenesiOS → /mnt/genesi
```

### 6️⃣ Criar ISO
```bash
cd GenesiOS
chmod +x build-iso.sh
sudo ./build-iso.sh
```

⏱️ **Aguarde 15-20 minutos**

### 7️⃣ Pegar ISO
```bash
# ISO criada em:
ls -lh GenesiOS-*.iso

# Transferir para Windows via pasta compartilhada
# ou servidor HTTP:
python3 -m http.server 8000
# Acesse: http://IP-DA-VM:8000
```

---

## 🧪 Testar ISO

### Criar VM de Teste
```
VirtualBox → Novo
├─ Nome: Genesi OS Test
├─ RAM: 4 GB
├─ Disco: 20 GB
├─ Sistema → Habilitar EFI
├─ Display → 128 MB VRAM
└─ Armazenamento → Adicionar GenesiOS.iso
```

### Iniciar
```
Iniciar VM → GRUB → Genesi OS
```

### Resultado Esperado
```
✅ Sistema inicia automaticamente
✅ Genesi OS em tela cheia
✅ Navegador com 1 barra (não 2!)
✅ Janelas sobrepõem corretamente
✅ Tudo funciona perfeitamente!
```

---

## 🔄 Rebuild Rápido

Depois do primeiro build:

```bash
# Fazer mudanças no código
vim genesi-desktop/src/App.tsx

# Rebuild (5 minutos)
sudo ./rebuild-iso.sh
```

---

## 📊 Checklist

- [ ] VirtualBox instalado
- [ ] Ubuntu 22.04 ISO baixado
- [ ] VM Ubuntu criada (4GB RAM, 30GB disco)
- [ ] Ubuntu instalado na VM
- [ ] Código transferido para VM
- [ ] `build-iso.sh` executado
- [ ] ISO gerada com sucesso
- [ ] VM de teste criada
- [ ] ISO testada e funcionando

---

## 🆘 Problemas Comuns

### "ISO não boota"
→ Habilitar EFI nas configurações da VM

### "Tela preta após boot"
→ Usar opção "Safe Mode" no GRUB

### "Genesi OS não inicia"
→ Pressionar Ctrl+Alt+F2 e ver logs:
```bash
journalctl -u genesi.service
```

---

## 📞 Resumo

1. **WSL não funciona** → Use VM Linux
2. **VirtualBox** → Mais fácil
3. **build-iso.sh** → Automatizado
4. **15-20 min** → Primeira vez
5. **5 min** → Rebuilds

🎉 **Pronto para criar sua ISO!**
