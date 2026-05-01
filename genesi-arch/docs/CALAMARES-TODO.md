# 🔧 Calamares Genesi - TODO List

## ✅ Feito

1. ✅ Clonado repositório do CachyOS Calamares
2. ✅ Criado branding `genesi` (cópia do `cachyos`)
3. ✅ Customizado `branding.desc`:
   - Nome: Genesi OS
   - Cores: Verde/teal (#00ff9f sobre #0a0f0d)
   - Versão: 1.0.0
4. ✅ Substituído logos (logo.png, icon.png, welcome.png)
5. ✅ Atualizado CMakeLists.txt para usar branding `genesi`

## ⏳ Pendente

### 1. Criar Slides Customizados (PRIORITÁRIO)

Precisamos criar 7 slides (1100x520px cada) mostrando:

**Slide 1**: Bem-vindo ao Genesi OS
- Logo grande do Genesi
- Texto: "A primeira distribuição Linux otimizada para IA local"

**Slide 2**: AI Mode
- Ícone de IA
- Explicar detecção automática de processos de IA
- Otimizações de performance

**Slide 3**: Auto-Updates
- Ícone de atualização
- Sistema de updates automático
- Notificações e widget

**Slide 4**: KDE Plasma Customizado
- Screenshot do desktop Genesi
- Tema verde/teal
- Widgets customizados

**Slide 5**: Developer Tools
- Ferramentas pré-instaladas
- Terminal, editores, etc.

**Slide 6**: Performance
- Baseado em CachyOS
- Kernel otimizado
- Benchmarks

**Slide 7**: Comunidade
- GitHub, Discord, etc.
- Como contribuir
- Suporte

### 2. Customizar Stylesheet (stylesheet.qss)

Aplicar tema verde/teal em todos os componentes:
- Botões
- Inputs
- Checkboxes
- Progress bars
- Sidebar
- etc.

### 3. Customizar Sidebar QML (calamares-sidebar.qml)

Aplicar cores e estilo do Genesi na sidebar customizada.

### 4. Testar Compilação

```bash
cd genesi-calamares
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DINSTALL_CONFIG=ON \
      -DINSTALL_POLKIT=ON \
      -DWITH_QT6=ON \
      ..
make -j$(nproc)
```

### 5. Criar PKGBUILD

Criar pacote `genesi-calamares` em `genesi-arch/packages/genesi-calamares/`.

### 6. Integrar na ISO

Substituir `cachyos-calamares-next` por `genesi-calamares` em `packages_desktop.x86_64`.

### 7. Testar Instalação

- Bootar ISO em VM
- Testar instalador
- Verificar branding
- Verificar slideshow
- Instalar e verificar sistema instalado

## 🎨 Design dos Slides

### Paleta de Cores

```
Background:     #0a0f0d (verde muito escuro)
Primary:        #00ff9f (verde/teal brilhante)
Secondary:      #00cc7f (verde médio)
Accent:         #009966 (verde escuro)
Text:           #00ff9f (verde brilhante)
Text Secondary: #ffffff (branco para contraste)
```

### Fontes

- Títulos: Bold, 32-36px
- Subtítulos: Regular, 20-24px
- Corpo: Regular, 16-18px

### Layout

```
┌─────────────────────────────────────┐
│                                     │
│         [LOGO/ÍCONE]                │
│                                     │
│         TÍTULO PRINCIPAL            │
│                                     │
│         Subtítulo explicativo       │
│                                     │
│         • Feature 1                 │
│         • Feature 2                 │
│         • Feature 3                 │
│                                     │
└─────────────────────────────────────┘
```

## 📝 Comandos Úteis

### Compilar apenas o branding

```bash
cd genesi-calamares/build
make install-branding
```

### Testar Calamares sem instalar

```bash
cd genesi-calamares/build
./calamares -d
```

### Ver logs do Calamares

```bash
journalctl -u calamares -f
```

## 🚀 Workflow Recomendado

1. **Criar slides** (pode ser feito no Figma, Inkscape, GIMP)
2. **Exportar como PNG** (1100x520px)
3. **Copiar para** `genesi-calamares/src/branding/genesi/`
4. **Customizar stylesheet.qss**
5. **Compilar e testar** localmente
6. **Criar PKGBUILD**
7. **Buildar pacote** com `makepkg`
8. **Adicionar à ISO**
9. **Testar ISO completa**

## 💡 Dicas

- Use o Calamares do CachyOS como referência (já está muito bom)
- Mantenha a funcionalidade, mude apenas o visual
- Teste em VM antes de fazer ISO final
- Os slides são a parte mais importante visualmente
- O stylesheet pode ser copiado do tema KDE do Genesi

## 🔗 Referências

- [Calamares Branding Guide](https://github.com/calamares/calamares/wiki/Branding-Guide)
- [CachyOS Calamares](https://github.com/CachyOS/cachyos-calamares)
- [Qt Stylesheet Reference](https://doc.qt.io/qt-6/stylesheet-reference.html)
