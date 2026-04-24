# Firefox - Correção do Problema das Duas Topbars

## 🐛 Problema

O Firefox estava aparecendo com **duas topbars**:
1. Uma topbar do próprio Firefox (CSD - Client-Side Decoration) - escura/preta
2. Uma topbar do Genesi OS (SSD - Server-Side Decoration)

## 🔍 Causa Raiz

O Firefox é extremamente teimoso com decorações de janela. Mesmo quando o compositor Wayland (genesi-wm) responde com `ServerSide` no protocolo `xdg-decoration`, o Firefox **ignora** e desenha sua própria topbar usando a biblioteca `libdecor`.

### Por que isso acontece?

1. **Firefox usa libdecor** - Uma biblioteca que força CSD no Wayland
2. **Ignora xdg-decoration** - Não respeita a resposta `ServerSide` do compositor
3. **Reutiliza processos** - Se já existe uma instância rodando, o `LD_PRELOAD` não funciona
4. **Sandbox de conteúdo** - Bloqueia `LD_PRELOAD` por segurança

## ✅ Solução Implementada

### 1. **nocsd.c - Interceptor de CSD** (Melhorado)

Biblioteca `LD_PRELOAD` que intercepta as funções que o Firefox usa para desenhar CSD:

```c
// Bloqueia libdecor (usado pelo Firefox)
void *libdecor_new(void *display, const void *iface) {
    return NULL;  // Sem decorador = sem CSD
}

// Bloqueia GTK HeaderBar
void *gtk_header_bar_new(void) {
    return NULL;
}

// Força variáveis de ambiente
char *getenv(const char *name) {
    if (strcmp(name, "GTK_CSD") == 0) return "0";
    if (strcmp(name, "MOZ_GTK_TITLEBAR_DECORATION") == 0) return "system";
    // ...
}
```

**Novas interceptações adicionadas:**
- `gtk_header_bar_new()` - Bloqueia criação de HeaderBar (GTK4)
- `gtk_window_get_titlebar()` - Mente dizendo que não existe titlebar
- `getenv()` - Fallback para apps que não usam `secure_getenv`
- `libdecor_unref()` e `libdecor_frame_unref()` - Previne crashes

### 2. **Compilação Automática do nocsd.so**

O Tauri agora compila automaticamente o `nocsd.so` se não existir:

```rust
fn get_nocsd_path() -> Option<String> {
    // Tenta cache primeiro
    let cached = read("/tmp/genesi-nocsd-path.txt");
    if cached.exists() { return Some(cached); }
    
    // Compila se necessário
    Command::new("cc")
        .args(&["-shared", "-fPIC", "-ldl", "-o", "/tmp/genesi_nocsd.so", "nocsd.c"])
        .output()?;
    
    Some("/tmp/genesi_nocsd.so")
}
```

### 3. **Lançamento Isolado do Firefox**

O Firefox agora é lançado com:

```rust
Command::new("firefox")
    .arg("--new-instance")              // Nova instância isolada
    .arg("--profile")
    .arg("/tmp/genesi-firefox-profile") // Profile temporário limpo
    .env("MOZ_DISABLE_CONTENT_SANDBOX", "1")  // Permite LD_PRELOAD
    .env("MOZ_GTK_TITLEBAR_DECORATION", "system")
    .env("MOZ_X11_EGL", "0")            // Força Wayland puro
    .env("GTK_CSD", "0")
    .env("GDK_BACKEND", "wayland")
    .env("LIBDECOR_PLUGIN_DIR", "/dev/null")  // Mata libdecor
    .env("LD_PRELOAD", nocsd_path)
    .spawn()
```

### 4. **Window Manager - xdg-decoration Forçado**

O compositor já estava correto, mas para garantir:

```rust
impl XdgDecorationHandler for GenesiState {
    fn new_decoration(&mut self, toplevel: ToplevelSurface) {
        toplevel.with_pending_state(|state| {
            state.decoration_mode = Some(Mode::ServerSide);
        });
        toplevel.send_configure();
    }
    
    fn request_mode(&mut self, toplevel: ToplevelSurface, _mode: Mode) {
        // SEMPRE ServerSide, ignora pedido do cliente
        toplevel.with_pending_state(|state| {
            state.decoration_mode = Some(Mode::ServerSide);
        });
        toplevel.send_configure();
    }
}
```

## 🧪 Como Testar

### Opção 1: Script de Teste (Recomendado)

```bash
cd genesi-desktop
bash test-firefox-ssd.sh
```

O script vai:
1. ✅ Compilar o `nocsd.so`
2. ✅ Matar instâncias antigas do Firefox
3. ✅ Criar profile temporário limpo
4. ✅ Lançar Firefox com todas as proteções
5. ✅ Abrir `about:support` para verificação

### Opção 2: Manual

```bash
# 1. Compila nocsd.so
cd genesi-desktop/genesi-wm
cc -shared -fPIC -ldl -o /tmp/genesi_nocsd.so nocsd.c

# 2. Mata Firefox antigo
pkill -9 firefox

# 3. Lança com proteções
LD_PRELOAD=/tmp/genesi_nocsd.so \
MOZ_ENABLE_WAYLAND=1 \
MOZ_GTK_TITLEBAR_DECORATION=system \
MOZ_DISABLE_CONTENT_SANDBOX=1 \
GTK_CSD=0 \
GDK_BACKEND=wayland \
LIBDECOR_PLUGIN_DIR=/dev/null \
firefox --new-instance --profile /tmp/ff-test
```

### Opção 3: Pelo Genesi Desktop

Simplesmente clique no ícone do Chrome/Firefox no desktop ou menu iniciar. O Tauri vai aplicar todas as correções automaticamente.

## ✅ Verificação

Após lançar o Firefox, verifique:

1. **Visual**: Deve ter APENAS UMA topbar (a do Genesi OS com os 3 botões coloridos)
2. **about:support**: 
   - Procure por "Window Protocol: **wayland**" (não xwayland)
   - Deve mostrar "Compositing: **WebRender**"
3. **Stderr**: Deve aparecer `[nocsd] libdecor_new() bloqueado - forçando SSD`

## 🚨 Troubleshooting

### Ainda aparece duas topbars?

1. **Verifique se o compositor está rodando:**
   ```bash
   ps aux | grep genesi-wm
   echo $WAYLAND_DISPLAY  # Deve mostrar wayland-0 ou wayland-1
   ```

2. **Verifique se o nocsd.so foi compilado:**
   ```bash
   ls -lh /tmp/genesi_nocsd.so
   file /tmp/genesi_nocsd.so  # Deve mostrar "shared object"
   ```

3. **Verifique se o Firefox está usando Wayland:**
   - Abra `about:support` no Firefox
   - Procure por "Window Protocol"
   - Se mostrar "x11" ou "xwayland", o Firefox não está usando Wayland nativo

4. **Mate TODAS as instâncias do Firefox:**
   ```bash
   pkill -9 firefox
   pkill -9 firefox-bin
   rm -rf ~/.mozilla/firefox/*.default*/sessionstore*
   ```

5. **Verifique logs do compositor:**
   ```bash
   # Se rodando via systemd
   journalctl -u genesi-wm -f
   
   # Se rodando manualmente
   # Veja o terminal onde iniciou o genesi-wm
   ```

### Firefox não abre?

- Instale o Firefox: `sudo apt install firefox`
- Ou instale o Chromium (preferido): `sudo apt install chromium-browser`
- Verifique se `cc` está instalado: `sudo apt install build-essential`

## 📚 Referências Técnicas

- [Wayland xdg-decoration Protocol](https://wayland.app/protocols/xdg-decoration-unstable-v1)
- [Firefox Wayland Support](https://wiki.archlinux.org/title/Firefox#Wayland)
- [GTK Client-Side Decorations](https://wiki.gnome.org/Initiatives/CSD)
- [libdecor Documentation](https://gitlab.freedesktop.org/libdecor/libdecor)

## 🎯 Chromium vs Firefox

**Chromium é o navegador PREFERIDO** do Genesi OS porque:

1. ✅ Implementa `xdg-decoration` corretamente
2. ✅ Respeita a resposta `ServerSide` do compositor
3. ✅ Não precisa de `LD_PRELOAD` ou hacks
4. ✅ Usa `--ozone-platform=wayland` nativamente

**Firefox requer hacks** porque:

1. ❌ Usa `libdecor` que força CSD
2. ❌ Ignora `xdg-decoration ServerSide`
3. ❌ Precisa de `LD_PRELOAD` para funcionar
4. ❌ Sandbox bloqueia `LD_PRELOAD` por padrão

## 📝 Changelog

### v1.1 (Atual)
- ✅ Adicionado `gtk_header_bar_new()` interceptor
- ✅ Adicionado `getenv()` fallback
- ✅ Compilação automática do `nocsd.so`
- ✅ Profile temporário isolado
- ✅ `MOZ_DISABLE_CONTENT_SANDBOX=1`
- ✅ Script de teste `test-firefox-ssd.sh`

### v1.0 (Anterior)
- ✅ Implementação básica do `nocsd.c`
- ✅ `xdg-decoration` forçado no compositor
- ✅ Variáveis de ambiente básicas
