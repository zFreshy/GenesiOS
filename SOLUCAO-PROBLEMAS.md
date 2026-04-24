# 🔧 Solução de Problemas - Genesi OS

## 🚨 Erro: "Port 1420 is already in use"

### Causa
A porta 1420 (usada pelo Tauri) está ocupada por um processo anterior que não foi fechado corretamente.

### Solução Rápida

**Opção 1: Script de parada**
```bash
bash stop-genesi.sh
```

**Opção 2: Comando rápido**
```bash
./cleanup
```

**Opção 3: Manual**
```bash
# Mata todos os processos
pkill -9 genesi-wm genesi-desktop cargo node

# Libera a porta 1420
lsof -ti:1420 | xargs kill -9
```

**Opção 4: Shutdown completo do WSL** (última opção)
```powershell
# No PowerShell (Windows)
wsl --shutdown
```

---

## 🔍 Diagnóstico Completo

Execute o script de diagnóstico para ver o estado do sistema:

```bash
bash diagnostico.sh
```

Isso vai mostrar:
- ✅ Processos rodando
- ✅ Status da porta 1420
- ✅ Rust/Cargo instalado
- ✅ Node.js instalado
- ✅ DISPLAY configurado
- ✅ Dependências GTK
- ✅ Binários compilados

---

## 🐛 Problemas Comuns

### 1. "cargo: command not found"

**Causa**: Rust não está instalado ou o ambiente não foi carregado.

**Solução**:
```bash
# Instala Rust
bash setup-wsl.sh

# Fecha e reabra o terminal WSL
exit
wsl

# Ou carrega manualmente
source ~/.cargo/env
```

---

### 2. "node: command not found"

**Causa**: Node.js não está instalado.

**Solução**:
```bash
bash setup-wsl.sh
```

---

### 3. "Failed to initialize GTK"

**Causa**: WSLg não está funcionando ou DISPLAY não está configurado.

**Solução**:
```bash
# 1. Verifica DISPLAY
echo $DISPLAY  # Deve mostrar :0 ou :1

# 2. Se não mostrar nada, configura
export DISPLAY=:0

# 3. Testa X11
xeyes  # Deve abrir uma janela

# 4. Se não funcionar, atualiza WSL
# No PowerShell (Windows) como Admin:
wsl --update
wsl --shutdown
```

---

### 4. Sistema abre e fecha imediatamente

**Causa**: Erro no beforeDevCommand ou porta ocupada.

**Solução**:
```bash
# 1. Limpa processos antigos
bash stop-genesi.sh

# 2. Verifica diagnóstico
bash diagnostico.sh

# 3. Tenta rodar novamente
bash run-genesi.sh
```

---

### 5. Compilação muito lenta

**Causa**: Primeira compilação do Rust é sempre lenta (5-10 minutos).

**Solução**: Seja paciente. As próximas compilações serão muito mais rápidas.

---

### 6. "Blocking waiting for file lock on package cache"

**Causa**: Outro processo Cargo está rodando.

**Solução**:
```bash
# Mata todos os processos Cargo
pkill -9 cargo

# Tenta novamente
bash run-genesi.sh
```

---

### 7. Navegador não abre

**Causa**: Chromium/Chrome não está instalado no WSL.

**Solução**:
```bash
# Instala Chromium
sudo apt update
sudo apt install chromium-browser

# Ou Google Chrome
wget https://dl.google.com/linux/direct/google-chrome-stable_current_amd64.deb
sudo dpkg -i google-chrome-stable_current_amd64.deb
sudo apt --fix-broken install
```

---

### 8. Janela do navegador aparece separada

**Causa**: Isso é esperado em desenvolvimento (WSL/Windows).

**Explicação**: 
- Em desenvolvimento, o navegador aparece como janela separada do Windows
- Quando bootar de uma ISO, o navegador vai rodar dentro do Genesi OS
- Veja `RESUMO-MUDANCAS-BROWSER.md` para mais detalhes

---

### 9. Ctrl+C não para o sistema

**Causa**: O trap do bash pode não estar funcionando corretamente.

**Solução**:
```bash
# Em outro terminal WSL
bash stop-genesi.sh

# Ou
./cleanup

# Ou força shutdown
wsl --shutdown  # No PowerShell
```

---

### 10. "Error: The beforeDevCommand terminated with a non-zero status code"

**Causa**: O comando `npm run dev` falhou antes do Tauri iniciar.

**Solução**:
```bash
# 1. Verifica se node_modules existe
cd genesi-desktop
ls node_modules  # Se não existir, instala
npm install

# 2. Testa o Vite manualmente
npm run dev  # Deve iniciar sem erros

# 3. Se der erro, verifica logs
# Procure por erros de sintaxe ou dependências faltando
```

---

## 🔄 Fluxo de Solução Padrão

Quando algo der errado, siga esta ordem:

```bash
# 1. Para tudo
bash stop-genesi.sh

# 2. Diagnóstico
bash diagnostico.sh

# 3. Se tiver processos rodando, limpa
./cleanup

# 4. Verifica se porta está livre
lsof -ti:1420  # Não deve retornar nada

# 5. Tenta rodar novamente
bash run-genesi.sh
```

---

## 📞 Ainda com problemas?

### Logs úteis para debug:

```bash
# Logs do Window Manager
cd genesi-desktop/genesi-wm
cargo run --release 2>&1 | tee wm.log

# Logs do Desktop
cd genesi-desktop
npm run tauri dev 2>&1 | tee desktop.log
```

### Informações do sistema:

```bash
# Versão do WSL
wsl --version

# Versão do Ubuntu
lsb_release -a

# Processos rodando
ps aux | grep genesi

# Portas em uso
netstat -tulpn | grep 1420
```

---

## 🎯 Checklist de Verificação

Antes de rodar o sistema, verifique:

- [ ] WSL2 instalado e atualizado
- [ ] Ubuntu no WSL funcionando
- [ ] Rust/Cargo instalado (`cargo --version`)
- [ ] Node.js instalado (`node --version`)
- [ ] DISPLAY configurado (`echo $DISPLAY`)
- [ ] Dependências GTK instaladas
- [ ] Porta 1420 livre (`lsof -ti:1420` retorna vazio)
- [ ] Nenhum processo Genesi rodando (`ps aux | grep genesi`)

Se todos os itens estiverem OK, o sistema deve rodar sem problemas!

---

## 💡 Dicas

1. **Sempre use `bash stop-genesi.sh` antes de fechar o terminal**
2. **Se travar, use `./cleanup` para limpar rapidamente**
3. **Execute `bash diagnostico.sh` quando tiver dúvidas**
4. **A primeira compilação demora, mas as seguintes são rápidas**
5. **Mantenha o WSL atualizado: `wsl --update`**

---

## 🚀 Comandos Úteis

```bash
# Rodar o sistema
bash run-genesi.sh

# Parar o sistema
bash stop-genesi.sh

# Limpeza rápida
./cleanup

# Diagnóstico
bash diagnostico.sh

# Setup inicial
bash setup-wsl.sh

# Atualizar WSL (PowerShell)
wsl --update
wsl --shutdown
```
