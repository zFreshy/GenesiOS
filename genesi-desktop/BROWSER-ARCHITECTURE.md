# Arquitetura do Navegador - Genesi OS

## 🎯 Objetivo

Integrar navegadores nativos (Chrome/Firefox) no Genesi OS de forma que:
- ✅ Rodem como janelas normais do sistema
- ✅ Tenham apenas UMA barra superior (a do navegador)
- ✅ Sejam gerenciados pelo Window Manager
- ✅ Funcionem tanto em desenvolvimento quanto no OS bootável

## 🏗️ Arquitetura

### Desenvolvimento (WSL/Windows)

```
┌─────────────────────────────────────────────┐
│         Windows (Host)                      │
│  ┌───────────────────────────────────────┐  │
│  │  Genesi Desktop (Tauri Window)        │  │
│  │  - Mostra placeholder "Abrindo..."    │  │
│  └───────────────────────────────────────┘  │
│  ┌───────────────────────────────────────┐  │
│  │  Chrome/Firefox (Janela Separada)    │  │ ← Aparece como app separado
│  │  - Lançado via WSL                    │  │
│  │  - Gerenciado pelo Windows            │  │
│  └───────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

**Limitação atual**: No desenvolvimento, o navegador aparece como janela separada do Windows porque:
- O Tauri roda dentro do Windows
- O navegador é lançado via WSL/Wayland
- O Windows não entende Wayland, então cria janela separada

### Sistema Real (ISO Bootável)

```
┌─────────────────────────────────────────────┐
│         Genesi OS (Sistema Real)            │
│  ┌───────────────────────────────────────┐  │
│  │  Window Manager (Wayland Compositor)  │  │
│  │  ┌─────────────────────────────────┐  │  │
│  │  │  Chrome/Firefox                 │  │  │ ← Dentro do OS
│  │  │  - Processo separado            │  │  │
│  │  │  - Gerenciado pelo WM           │  │  │
│  │  │  - Uma barra apenas             │  │  │
│  │  └─────────────────────────────────┘  │  │
│  │  ┌─────────────────────────────────┐  │  │
│  │  │  Terminal                       │  │  │
│  │  └─────────────────────────────────┘  │  │
│  └───────────────────────────────────────┘  │
└─────────────────────────────────────────────┘
```

**Funcionamento correto**: Quando bootar de verdade:
- O Window Manager captura a janela do navegador
- O navegador roda como processo separado
- O WM gerencia posição, tamanho, foco, etc.
- Apenas UMA barra (a do navegador)

## 🔧 Componentes

### 1. WaylandBrowserApp.tsx

Componente React que:
- Mostra placeholder "Abrindo navegador..."
- Chama `launch_browser_wayland()` do Rust
- Fecha automaticamente após 2 segundos
- Explica como funciona para o usuário

```tsx
<WaylandBrowserApp 
  onClose={() => closeApp(a.id)}
  onMinimize={() => toggleMinimize(a.id)}
  onMaximize={() => toggleMaximize(a.id)}
/>
```

### 2. launch_browser_wayland() - Rust

Função que lança o navegador nativo via Wayland:

```rust
#[tauri::command]
fn launch_browser_wayland() -> Result<(), String> {
    // Configura variáveis de ambiente para evitar dual topbar
    let wayland_envs = [
        ("GTK_CSD", "0"),                    // Desabilita CSD
        ("LIBDECOR_PLUGIN_DIR", "/dev/null"), // Mata libdecor
        ("MOZ_ENABLE_WAYLAND", "1"),          // Força Wayland
        ("GDK_BACKEND", "wayland"),           // Força Wayland no GTK
    ];
    
    // Tenta Chromium primeiro (melhor suporte a SSD)
    Command::new("chromium-browser")
        .args(&["--ozone-platform=wayland", "--gtk-version=4"])
        .envs(wayland_envs)
        .spawn()?;
}
```

### 3. nocsd.so - Interceptor CSD

Biblioteca compartilhada que intercepta funções GTK/libdecor:
- `gtk_header_bar_new()` → retorna NULL
- `gtk_window_get_titlebar()` → retorna NULL
- `getenv("GTK_CSD")` → retorna "0"

Usado especialmente para Firefox, que ignora variáveis de ambiente.

## 🎨 Evitando Dual Topbar

### Problema

Navegadores modernos usam **Client-Side Decorations (CSD)**:
- Desenham sua própria barra de título
- Ignoram o compositor Wayland
- Resultado: 2 barras (uma do navegador + uma do OS)

### Solução

**1. Chromium/Chrome** (Preferido):
```bash
--ozone-platform=wayland    # Força Wayland nativo
--gtk-version=4             # GTK4 respeita SSD
GTK_CSD=0                   # Desabilita CSD
```

Chromium implementa `xdg-decoration-unstable-v1` corretamente:
- Pergunta ao compositor: "Você quer desenhar a barra?"
- Compositor responde: "Sim, eu desenho (SSD)"
- Chromium: "OK, não vou desenhar então"
- **Resultado**: Uma barra apenas ✅

**2. Firefox** (Requer nocsd.so):
```bash
MOZ_ENABLE_WAYLAND=1
GTK_CSD=0
LIBDECOR_PLUGIN_DIR=/dev/null
LD_PRELOAD=/tmp/genesi_nocsd.so  # Interceptor
--new-instance                    # Força nova instância
```

Firefox usa libdecor internamente e **ignora** a resposta do compositor:
- Sempre desenha CSD, mesmo quando o compositor diz "não precisa"
- Solução: `nocsd.so` intercepta as funções e retorna NULL
- **Resultado**: Uma barra apenas ✅

## 📊 Comparação de Abordagens

### ❌ Abordagem Antiga (NativeBrowserApp)

```tsx
// Criava janelas WebView separadas
create_browser_window(url, title)
  → WebviewWindowBuilder::new()
  → Janela separada do Windows
  → Aparece na barra de tarefas do Windows
```

**Problemas**:
- Janelas aparecem fora do OS
- Não funciona quando bootar de verdade
- Difícil de gerenciar

### ✅ Abordagem Nova (WaylandBrowserApp)

```tsx
// Lança navegador nativo via Wayland
launch_browser_wayland()
  → Command::new("chromium-browser")
  → Processo separado
  → WM captura e gerencia
```

**Vantagens**:
- Navegador roda como processo real
- WM gerencia a janela
- Funciona no OS bootável
- Uma barra apenas

## 🚀 Fluxo de Execução

### Quando o usuário clica no ícone do Chrome:

1. **App.tsx**: `openApp('chrome')`
2. **App.tsx**: Cria janela com `<WaylandBrowserApp />`
3. **WaylandBrowserApp**: Mostra "Abrindo navegador..."
4. **WaylandBrowserApp**: Chama `invoke('launch_browser_wayland')`
5. **Rust**: Configura variáveis de ambiente (GTK_CSD=0, etc.)
6. **Rust**: Lança `chromium-browser --ozone-platform=wayland`
7. **Chromium**: Abre e pergunta ao WM sobre decorações
8. **WM**: Responde "ServerSide" (eu desenho a barra)
9. **Chromium**: Não desenha barra própria
10. **WM**: Captura janela e gerencia
11. **WaylandBrowserApp**: Fecha placeholder após 2s

**Resultado**: Navegador rodando com uma barra apenas! ✅

## 🔍 Debugging

### Verificar se CSD está desabilitado:

```bash
# No terminal onde o navegador foi lançado
echo $GTK_CSD  # Deve ser "0"
echo $LIBDECOR_PLUGIN_DIR  # Deve ser "/dev/null"
```

### Verificar se nocsd.so está carregado (Firefox):

```bash
# Verificar processos do Firefox
ps aux | grep firefox

# Verificar LD_PRELOAD
cat /proc/$(pgrep firefox)/environ | tr '\0' '\n' | grep LD_PRELOAD
```

### Verificar se o WM está respondendo SSD:

```bash
# Logs do genesi-wm
cd genesi-desktop/genesi-wm
cargo run --release

# Procure por:
# "xdg_decoration: client requested ServerSide"
```

## 📝 Notas Importantes

1. **Desenvolvimento vs Produção**:
   - Em desenvolvimento (WSL), o navegador aparece como janela separada
   - No OS real (ISO), o navegador é gerenciado pelo WM
   - Isso é esperado e normal

2. **Chromium vs Firefox**:
   - Chromium é preferido (melhor suporte a SSD)
   - Firefox requer nocsd.so (mais complexo)
   - Ordem de tentativa: Chromium → Chrome → Firefox → Epiphany

3. **Uma barra apenas**:
   - Garantido por: GTK_CSD=0 + LIBDECOR_PLUGIN_DIR=/dev/null + nocsd.so
   - Chromium respeita naturalmente
   - Firefox precisa de interceptação

4. **Performance**:
   - Navegador roda como processo separado (não afeta Tauri)
   - WM gerencia via Wayland (protocolo leve)
   - Sem overhead de WebView embutido

## 🎓 Referências

- [xdg-decoration Protocol](https://wayland.app/protocols/xdg-decoration-unstable-v1)
- [GTK Client-Side Decorations](https://wiki.gnome.org/Initiatives/CSD)
- [Chromium Ozone Platform](https://chromium.googlesource.com/chromium/src/+/master/docs/ozone_overview.md)
- [Firefox Wayland Support](https://wiki.mozilla.org/Platform/GFX/Wayland)

## ✅ Checklist de Implementação

- [x] Criar WaylandBrowserApp.tsx
- [x] Remover NativeBrowserApp
- [x] Configurar launch_browser_wayland() com flags corretas
- [x] Implementar nocsd.so para Firefox
- [x] Testar Chromium (uma barra)
- [x] Testar Firefox (uma barra)
- [x] Documentar arquitetura
- [ ] Testar em ISO bootável (quando criar)
- [ ] Adicionar suporte a múltiplas abas (futuro)
- [ ] Adicionar barra de endereço integrada (futuro)
