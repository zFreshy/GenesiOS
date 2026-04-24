# 🚀 Quick Start - Genesi OS no WSL

## TL;DR (Muito Rápido)

```bash
# 1. Abra o WSL (Ubuntu)
wsl

# 2. Vá para o diretório do projeto
cd /mnt/d/Desenvolvimento/Genesi

# 3. Execute o setup (primeira vez apenas)
bash setup-wsl.sh

# 4. Feche e reabra o terminal WSL
exit
wsl
cd /mnt/d/Desenvolvimento/Genesi

# 5. Rode o sistema
bash run-genesi.sh
```

## ⚡ Comandos Úteis

```bash
# Rodar o sistema
bash run-genesi.sh

# Parar o sistema (Ctrl+C ou)
bash stop-genesi.sh

# Parar rápido
./cleanup

# Verificar se tudo está instalado
cargo --version
node --version
echo $DISPLAY
```

## 🐛 Problemas?

### "cargo: command not found"
```bash
bash setup-wsl.sh
# Depois feche e reabra o terminal
```

### "Failed to initialize GTK"
```powershell
# No PowerShell (Windows) como Admin:
wsl --update
wsl --shutdown
# Depois reabra o WSL
```

### Compilação travou?
Seja paciente. A primeira compilação demora 5-10 minutos.

## 📖 Guia Completo

Para mais detalhes, veja: `WSL-SETUP-GUIDE.md`
