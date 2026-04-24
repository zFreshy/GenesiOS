# Guia de Setup - Genesi OS no WSL

## 🎯 Objetivo

Rodar o Genesi OS completamente no WSL (Ubuntu no Windows), sem usar o Windows para nada além de hospedar o WSL.

## 📋 Pré-requisitos

- Windows 10/11 com WSL2 instalado
- Ubuntu no WSL (ou outra distro Linux)
- WSLg habilitado (vem por padrão no WSL2 atualizado)

## 🚀 Instalação Rápida

### Passo 1: Abra o terminal WSL (Ubuntu)

No Windows, abra o terminal e digite:
```bash
wsl
```

Ou abra diretamente o Ubuntu pelo menu Iniciar.

### Passo 2: Navegue até o diretório do projeto

```bash
cd /mnt/d/Desenvolvimento/Genesi
```

**Nota**: Ajuste o caminho conforme onde você clonou o repositório.

### Passo 3: Execute o script de setup

```bash
bash setup-wsl.sh
```

Este script irá instalar:
- ✅ Dependências do Tauri (WebKit2GTK, GTK3, etc.)
- ✅ Node.js 20 e npm
- ✅ Rust e Cargo
- ✅ Ferramentas gráficas (X11, Mesa)
- ✅ Configuração do DISPLAY

**Tempo estimado**: 5-10 minutos (dependendo da internet)

### Passo 4: Feche e reabra o terminal WSL

Isso é necessário para carregar o ambiente do Rust.

```bash
exit
```

Depois abra novamente:
```bash
wsl
cd /mnt/d/Desenvolvimento/Genesi
```

### Passo 5: Rode o Genesi OS

```bash
bash run-genesi.sh
```

Pronto! O sistema deve iniciar.

## 🛑 Como Parar

### Opção 1: Ctrl+C (Recomendado)
Pressione `Ctrl+C` no terminal onde o sistema está rodando.

### Opção 2: Script de parada
Em outro terminal WSL:
```bash
bash stop-genesi.sh
```

### Opção 3: Comando rápido
```bash
./cleanup
```

### Opção 4: Shutdown completo do WSL
No PowerShell (Windows):
```powershell
wsl --shutdown
```

**Nota**: Esta opção fecha TUDO no WSL, não apenas o Genesi OS.

## 🐛 Solução de Problemas

### Erro: "cargo: command not found"

**Causa**: O Rust não foi instalado ou o ambiente não foi carregado.

**Solução**:
1. Execute o setup: `bash setup-wsl.sh`
2. Feche e reabra o terminal WSL
3. Tente novamente

### Erro: "Failed to initialize GTK"

**Causa**: WSLg não está funcionando ou DISPLAY não está configurado.

**Solução**:
1. Verifique se o DISPLAY está configurado:
   ```bash
   echo $DISPLAY
   ```
   Deve mostrar `:0` ou `:1`

2. Teste se o X11 funciona:
   ```bash
   xeyes
   ```
   Deve abrir uma janela com olhos animados.

3. Se não funcionar, atualize o WSL no PowerShell (como Admin):
   ```powershell
   wsl --update
   wsl --shutdown
   ```

4. Reabra o WSL e tente novamente.

### Erro: "node: command not found"

**Causa**: Node.js não foi instalado.

**Solução**:
```bash
bash setup-wsl.sh
```

### Compilação muito lenta

**Causa**: A primeira compilação do Rust é sempre lenta (5-10 minutos).

**Solução**: Seja paciente. As próximas compilações serão muito mais rápidas (segundos).

### Janela não abre

**Causa**: O Window Manager pode não estar rodando corretamente.

**Solução**:
1. Verifique os logs no terminal
2. Tente rodar apenas o Desktop (sem WM):
   ```bash
   cd genesi-desktop
   npm run tauri dev
   ```

## 📊 Verificação do Sistema

Para verificar se tudo está instalado corretamente:

```bash
# Verifica Rust
cargo --version

# Verifica Node.js
node --version
npm --version

# Verifica DISPLAY
echo $DISPLAY

# Verifica X11
xdpyinfo | head -5

# Verifica GTK
dpkg -l | grep libgtk-3
dpkg -l | grep libwebkit2gtk
```

## 🎓 Entendendo a Arquitetura

O Genesi OS no WSL funciona assim:

```
┌─────────────────────────────────────┐
│         Windows (Host)              │
│  ┌───────────────────────────────┐  │
│  │      WSL2 (Ubuntu)            │  │
│  │  ┌─────────────────────────┐  │  │
│  │  │  Genesi Window Manager  │  │  │
│  │  │  (Wayland Compositor)   │  │  │
│  │  └─────────────────────────┘  │  │
│  │  ┌─────────────────────────┐  │  │
│  │  │  Genesi Desktop         │  │  │
│  │  │  (Tauri + React)        │  │  │
│  │  └─────────────────────────┘  │  │
│  │           ↕                    │  │
│  │  ┌─────────────────────────┐  │  │
│  │  │  WSLg (X11/Wayland)     │  │  │
│  │  └─────────────────────────┘  │  │
│  └───────────────────────────────┘  │
│           ↕                         │
│  ┌───────────────────────────────┐  │
│  │  Windows Display Driver       │  │
│  └───────────────────────────────┘  │
└─────────────────────────────────────┘
```

1. **Window Manager**: Compositor Wayland que gerencia janelas
2. **Desktop**: Interface gráfica (Tauri + React)
3. **WSLg**: Ponte entre Linux e Windows para gráficos
4. **Windows**: Apenas exibe a janela final

## 💡 Dicas

- **Primeira execução**: Demora mais (compilação)
- **Execuções seguintes**: Muito mais rápidas
- **Desenvolvimento**: Use `cargo watch` para hot reload
- **Logs**: Fique de olho no terminal para ver erros
- **Performance**: WSLg é rápido, mas não tão rápido quanto Linux nativo

## 🔗 Links Úteis

- [Documentação do WSL](https://docs.microsoft.com/pt-br/windows/wsl/)
- [WSLg (GUI no WSL)](https://github.com/microsoft/wslg)
- [Tauri](https://tauri.app/)
- [Rust](https://www.rust-lang.org/)

## ❓ Ainda com problemas?

Se nada funcionar, tente rodar apenas o Desktop (sem o WM):

```bash
cd genesi-desktop
npm install
npm run tauri dev
```

Isso vai rodar apenas a interface, sem o compositor Wayland.
