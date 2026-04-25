# Correção: Erro de Conexão WM (Connection Reset by Peer)

## Problema Identificado

O Genesi Desktop não conseguia conectar ao Genesi WM, resultando no erro:
```
Gdk-Message: Error 104 (Connection reset by peer) dispatching to Wayland display
```

### Causa Raiz

O **Genesi WM usa backend Winit**, que é projetado para aplicativos que rodam **dentro** de outro compositor Wayland, não como compositor standalone. Isso causava:

1. **Conflito de Arquitetura**: O WM tentava ser servidor E cliente Wayland simultaneamente
2. **Falta de Backend DRM**: Sem backend DRM, o WM não conseguia rodar direto no hardware
3. **Inicialização Duplicada**: O Desktop era iniciado em dois lugares:
   - No código do WM (linha 650 do main.rs) via `npm run tauri dev`
   - No script de autostart da ISO via `./genesi-desktop`

## Solução Implementada

### 1. Removida Inicialização Automática do Desktop no WM

**Arquivo**: `genesi-desktop/genesi-wm/src/main.rs`

Removido o código que tentava iniciar o Desktop automaticamente:
```rust
// REMOVIDO:
info!("✨ Iniciando o Genesi Desktop Environment automaticamente...");
let _desktop_process = std::process::Command::new("npm")
    .arg("run")
    .arg("tauri")
    .arg("dev")
    .current_dir("../")
    .env("WAYLAND_DISPLAY", &socket_name)
    .spawn()
    .expect("Falha ao iniciar o Genesi Desktop");
```

### 2. Adicionado Weston como Compositor Base

**Arquivo**: `build-iso.sh`

- **Instalado Weston**: Compositor Wayland com backend DRM nativo
- **Arquitetura em Camadas**:
  ```
  Hardware (DRM/KMS)
       ↓
  Weston (wayland-0) ← Compositor base com acesso direto ao hardware
       ↓
  Genesi WM (wayland-1) ← Window Manager rodando dentro do Weston
       ↓
  Genesi Desktop ← Interface gráfica conectando ao WM
       ↓
  Apps (Firefox, etc.) ← Aplicativos do usuário
  ```

### 3. Atualizado Script de Autostart

**Sequência de Inicialização**:

1. **Weston inicia no DRM** (TTY1, acesso direto ao hardware)
   - Cria socket `wayland-0`
   - Fornece ambiente Wayland funcional

2. **Genesi WM inicia dentro do Weston**
   - Conecta ao `wayland-0` (Weston)
   - Cria socket `wayland-1` para clientes
   - Gerencia janelas e decorações

3. **Genesi Desktop conecta ao WM**
   - Usa `WAYLAND_DISPLAY=wayland-1`
   - Interface gráfica completa
   - Lança aplicativos

## Próximos Passos (Futuro)

Para tornar o Genesi WM verdadeiramente standalone:

1. **Implementar Backend DRM no WM**
   - Adicionar `smithay::backend::drm`
   - Remover dependência do Winit
   - Acesso direto ao hardware

2. **Remover Dependência do Weston**
   - WM rodará direto no DRM
   - Arquitetura simplificada
   - Melhor performance

## Como Testar

### No Ubuntu (VM):
```bash
cd ~/GenesiOS
bash build-iso.sh
```

### Na ISO Gerada:
1. Boot da ISO
2. Sistema inicia automaticamente
3. Weston → Genesi WM → Genesi Desktop
4. Desktop deve aparecer sem erros

### Verificar Logs:
```bash
# No TTY (Ctrl+Alt+F2)
cat /tmp/genesi-startup.log
```

## Arquivos Modificados

- `genesi-desktop/genesi-wm/src/main.rs` - Removida inicialização automática
- `build-iso.sh` - Adicionado Weston e nova sequência de startup
