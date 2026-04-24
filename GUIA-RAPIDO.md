# ⚡ Guia Rápido - Genesi OS

## 🚀 Como Rodar

### Primeira Vez

```bash
# 1. Setup (instala dependências)
bash setup-wsl.sh

# 2. Fecha e reabra o terminal WSL
exit
wsl

# 3. Roda o sistema
bash start.sh
```

### Próximas Vezes

#### Opção 1: Script Automático (Recomendado)
```bash
bash start.sh  # Limpa e inicia automaticamente
```

#### Opção 2: Manual (2 passos)
```bash
bash stop-genesi.sh  # Limpa primeiro
bash run-genesi.sh   # Depois inicia
```

#### Opção 3: Manual Completo (2 terminais)

**Terminal 1 - Window Manager:**
```bash
cd genesi-desktop/genesi-wm
cargo build --release
cargo run --release
```

**Terminal 2 - Desktop Environment:**
```bash
cd genesi-desktop
npm run tauri dev
```

**Por que usar 2 terminais?**
- ✅ Mais controle
- ✅ Logs separados (fácil debugar)
- ✅ Pode reiniciar um sem afetar o outro
- ✅ Melhor para desenvolvimento

---

## 🛑 Como Parar

### Opção 1: Ctrl+C (Recomendado)
Pressione `Ctrl+C` no terminal onde está rodando.

### Opção 2: Script de parada
```bash
bash stop-genesi.sh
```

### Opção 3: Limpeza rápida
```bash
./cleanup
```

### Opção 4: Shutdown completo (última opção)
```powershell
# No PowerShell (Windows)
wsl --shutdown
```

---

## 🔧 Solução de Problemas

### Erro: "Port 1420 is already in use"

```bash
# Limpa processos antigos
bash stop-genesi.sh

# Ou limpeza rápida
./cleanup

# Tenta novamente
bash run-genesi.sh
```

### Sistema não inicia

```bash
# 1. Diagnóstico
bash diagnostico.sh

# 2. Limpa tudo
bash stop-genesi.sh

# 3. Tenta novamente
bash run-genesi.sh
```

### Mais problemas?

Veja o guia completo: `SOLUCAO-PROBLEMAS.md`

---

## 📊 Comandos Úteis

```bash
# Rodar (automático)
bash start.sh

# Rodar (manual)
bash run-genesi.sh

# Parar
bash stop-genesi.sh

# Limpeza rápida
./cleanup

# Diagnóstico
bash diagnostico.sh

# Setup inicial
bash setup-wsl.sh
```

### Modo Manual (2 terminais)

```bash
# Terminal 1
cd genesi-desktop/genesi-wm
cargo run --release

# Terminal 2
cd genesi-desktop
npm run tauri dev
```

---

## 🎯 Fluxo Normal de Uso

```
1. bash run-genesi.sh
   ↓
2. Use o sistema
   ↓
3. Ctrl+C ou bash stop-genesi.sh
   ↓
4. Repita quando quiser usar novamente
```

---

## 💡 Dicas

- ✅ Sempre pare o sistema antes de fechar o terminal
- ✅ Use `bash diagnostico.sh` se tiver dúvidas
- ✅ A primeira compilação demora (5-10 min), mas as seguintes são rápidas
- ✅ Se travar, use `./cleanup` para limpar rapidamente

---

## 📚 Documentação Completa

- `README.md` - Documentação geral
- `SOLUCAO-PROBLEMAS.md` - Solução de problemas detalhada
- `WSL-SETUP-GUIDE.md` - Guia completo de setup no WSL
- `QUICK-START-WSL.md` - Início rápido no WSL
- `RESUMO-MUDANCAS-BROWSER.md` - Como funciona o navegador
- `genesi-desktop/BROWSER-ARCHITECTURE.md` - Arquitetura do navegador
- `genesi-desktop/FIREFOX-SSD-FIX.md` - Fix para Firefox dual topbar
