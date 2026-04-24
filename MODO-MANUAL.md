# 🔧 Modo Manual - Genesi OS

## Por que usar o modo manual?

Rodar o Window Manager e o Desktop Environment em terminais separados oferece:

- ✅ **Mais controle**: Você vê exatamente o que cada componente está fazendo
- ✅ **Logs separados**: Fácil identificar onde está o problema
- ✅ **Reinício independente**: Pode reiniciar um sem afetar o outro
- ✅ **Melhor para desenvolvimento**: Ideal para debugar e testar mudanças
- ✅ **Sem conflitos de porta**: Evita o problema da porta 1420

## 🚀 Como Rodar

### Terminal 1: Window Manager

```bash
cd genesi-desktop/genesi-wm

# Primeira vez (compila)
cargo build --release

# Roda o WM
cargo run --release
```

**O que você vai ver:**
```
Compiling genesi-wm v0.1.0
Finished release [optimized] target(s) in X.XXs
Running target/release/genesi-wm

[INFO] Wayland compositor iniciado
[INFO] Socket: wayland-1
[INFO] Aguardando conexões...
```

### Terminal 2: Desktop Environment

```bash
cd genesi-desktop

# Primeira vez (instala dependências)
npm install

# Roda o Desktop
npm run tauri dev
```

**O que você vai ver:**
```
> genesi-desktop@0.1.0 dev
> vite

VITE v7.0.4  ready in XXX ms

➜  Local:   http://localhost:1420/
➜  Network: use --host to expose

[INFO] Starting Tauri application...
```

## 🛑 Como Parar

### Opção 1: Ctrl+C em cada terminal
Pressione `Ctrl+C` em cada um dos terminais.

### Opção 2: Script de parada
Em um terceiro terminal:
```bash
bash stop-genesi.sh
```

### Opção 3: Limpeza rápida
```bash
./cleanup
```

## 🔍 Logs e Debug

### Logs do Window Manager (Terminal 1)

```
[INFO] Wayland compositor iniciado
[INFO] Cliente conectado: genesi-desktop
[INFO] Janela criada: Chrome
[INFO] xdg_decoration: client requested ServerSide
[DEBUG] Renderizando frame...
```

**O que procurar:**
- ✅ "Wayland compositor iniciado" - WM está rodando
- ✅ "Cliente conectado" - Desktop se conectou
- ❌ Erros de compilação - Problema no código Rust
- ❌ "Failed to bind socket" - Porta Wayland ocupada

### Logs do Desktop (Terminal 2)

```
VITE v7.0.4  ready in 234 ms
➜  Local:   http://localhost:1420/

[INFO] Starting Tauri application...
[INFO] Window created
[INFO] WebView initialized
```

**O que procurar:**
- ✅ "ready in XXX ms" - Vite iniciou
- ✅ "Starting Tauri application" - Tauri está iniciando
- ❌ "Port 1420 is already in use" - Porta ocupada
- ❌ "Failed to initialize GTK" - Problema com WSLg

## 🐛 Solução de Problemas

### Terminal 1: "Failed to bind socket"

**Causa**: Socket Wayland já está em uso.

**Solução**:
```bash
# Mata processos do WM
pkill -9 genesi-wm

# Tenta novamente
cargo run --release
```

### Terminal 2: "Port 1420 is already in use"

**Causa**: Vite já está rodando.

**Solução**:
```bash
# Mata processos Node
pkill -9 node

# Libera porta
lsof -ti:1420 | xargs kill -9

# Tenta novamente
npm run tauri dev
```

### Terminal 2: "Failed to initialize GTK"

**Causa**: WSLg não está funcionando.

**Solução**:
```bash
# Verifica DISPLAY
echo $DISPLAY  # Deve mostrar :0

# Se não mostrar, configura
export DISPLAY=:0

# Testa X11
xeyes

# Se não funcionar, atualiza WSL (PowerShell)
wsl --update
wsl --shutdown
```

## 🔄 Reiniciando Componentes

### Reiniciar apenas o Window Manager

**Terminal 1:**
```bash
# Ctrl+C para parar
^C

# Roda novamente
cargo run --release
```

O Desktop continua rodando e se reconecta automaticamente.

### Reiniciar apenas o Desktop

**Terminal 2:**
```bash
# Ctrl+C para parar
^C

# Roda novamente
npm run tauri dev
```

O Window Manager continua rodando e aceita a nova conexão.

## 💡 Dicas

### Hot Reload do Rust (WM)

```bash
# Instala cargo-watch
cargo install cargo-watch

# Roda com hot reload
cargo watch -x run
```

Agora o WM reinicia automaticamente quando você edita o código!

### Hot Reload do React (Desktop)

O Vite já faz hot reload automaticamente. Edite os arquivos `.tsx` e veja as mudanças instantaneamente.

### Logs Detalhados

**Window Manager:**
```bash
RUST_LOG=debug cargo run --release
```

**Desktop:**
```bash
npm run tauri dev -- --verbose
```

### Salvar Logs em Arquivo

**Terminal 1:**
```bash
cargo run --release 2>&1 | tee wm.log
```

**Terminal 2:**
```bash
npm run tauri dev 2>&1 | tee desktop.log
```

Agora você tem os logs salvos em `wm.log` e `desktop.log`.

## 📊 Comparação: Script vs Manual

| Aspecto | Script Automático | Modo Manual |
|---------|------------------|-------------|
| **Facilidade** | ✅ Muito fácil | ⚠️ Requer 2 terminais |
| **Controle** | ⚠️ Limitado | ✅ Total |
| **Logs** | ⚠️ Misturados | ✅ Separados |
| **Debug** | ⚠️ Difícil | ✅ Fácil |
| **Reinício** | ❌ Tudo junto | ✅ Independente |
| **Desenvolvimento** | ⚠️ OK | ✅ Ideal |
| **Uso diário** | ✅ Ideal | ⚠️ OK |

## 🎯 Quando Usar Cada Modo

### Use o Script Automático quando:
- ✅ Você só quer usar o sistema
- ✅ Não está desenvolvendo
- ✅ Quer simplicidade

### Use o Modo Manual quando:
- ✅ Está desenvolvendo/debugando
- ✅ Quer ver logs detalhados
- ✅ Precisa reiniciar componentes separadamente
- ✅ Está tendo problemas com o script automático

## 🚀 Fluxo de Desenvolvimento Recomendado

```bash
# Terminal 1: WM com hot reload
cd genesi-desktop/genesi-wm
cargo watch -x run

# Terminal 2: Desktop com hot reload (automático)
cd genesi-desktop
npm run tauri dev

# Terminal 3: Comandos e testes
cd genesi-desktop
# Aqui você pode rodar comandos, git, etc.
```

Agora você tem:
- 🔥 Hot reload no Rust (WM)
- 🔥 Hot reload no React (Desktop)
- 📝 Terminal livre para comandos

**Produtividade máxima!** 🚀

## 📚 Comandos Úteis

```bash
# Compilar WM (release)
cd genesi-desktop/genesi-wm
cargo build --release

# Compilar WM (debug - mais rápido)
cargo build

# Rodar WM (release)
cargo run --release

# Rodar WM (debug)
cargo run

# Rodar WM com logs
RUST_LOG=debug cargo run

# Rodar Desktop
cd genesi-desktop
npm run tauri dev

# Rodar Desktop (verbose)
npm run tauri dev -- --verbose

# Limpar build do Rust
cargo clean

# Limpar node_modules
rm -rf node_modules
npm install
```

## ✅ Checklist

Antes de rodar no modo manual:

- [ ] Rust instalado (`cargo --version`)
- [ ] Node.js instalado (`node --version`)
- [ ] DISPLAY configurado (`echo $DISPLAY`)
- [ ] Dependências instaladas (`npm install`)
- [ ] Porta 1420 livre (`lsof -ti:1420` retorna vazio)
- [ ] Nenhum processo Genesi rodando (`ps aux | grep genesi`)

Se todos os itens estiverem OK, pode rodar! 🎉
