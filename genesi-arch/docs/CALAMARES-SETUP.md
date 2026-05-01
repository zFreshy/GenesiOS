# 🎨 Genesi OS Calamares - Setup Completo

## 📋 O Que Foi Feito

### 1. Clonado Repositório do CachyOS Calamares

```bash
git clone https://github.com/CachyOS/cachyos-calamares.git genesi-calamares
```

O repositório está em: `GenesiOS/genesi-calamares/`

### 2. Criado Branding Genesi

Copiamos o branding do CachyOS e customizamos:

```
genesi-calamares/src/branding/genesi/
├── branding.desc       ✅ Customizado
├── logo.png            ✅ Logo do Genesi
├── icon.png            ✅ Ícone do Genesi
├── welcome.png         ✅ Imagem de boas-vindas
├── show.qml            ⏳ Usando slides do CachyOS (temporário)
├── slide1.png          ⏳ Precisa criar
├── slide2.png          ⏳ Precisa criar
├── slide3.png          ⏳ Precisa criar
├── slide4.png          ⏳ Precisa criar
├── slide5.png          ⏳ Precisa criar
├── slide6.png          ⏳ Precisa criar
├── slide7.png          ⏳ Precisa criar
├── stylesheet.qss      ⏳ Precisa customizar
└── calamares-sidebar.qml ⏳ Precisa customizar
```

### 3. Customizações Aplicadas

#### branding.desc

```yaml
componentName: genesi

strings:
    productName: "Genesi OS"
    shortProductName: Genesi
    version: 1.0.0
    bootloaderEntryName: "Genesi OS"

style:
   SidebarBackground: "#0a0f0d"
   SidebarText: "#00ff9f"
   SidebarTextCurrent: "#0a0f0d"
   SidebarBackgroundCurrent: "#00ff9f"
```

#### CMakeLists.txt

```cmake
calamares_add_branding_subdirectory( genesi )
```

### 4. Logos Copiados

- `wallpapers/logo/GenesiOSLogo.png` → `logo.png`
- `wallpapers/logo/GenesiOSLogo.png` → `icon.png`
- `wallpapers/logo/GenesiOSLogo.png` → `welcome.png`

## 🚀 Próximos Passos

### Opção A: Usar Calamares do CachyOS (RECOMENDADO PARA AGORA)

Por enquanto, use o `cachyos-calamares-next` que já está funcionando:

```bash
# Já está em packages_desktop.x86_64
cachyos-calamares-next
```

**Vantagens:**
- ✅ Funciona imediatamente
- ✅ Não precisa compilar nada
- ✅ Já testado e estável
- ✅ Podemos focar em fazer a ISO funcionar primeiro

**Desvantagens:**
- ❌ Branding do CachyOS (mas funciona)

### Opção B: Compilar Genesi Calamares (FAZER DEPOIS)

Quando quiser ter o instalador 100% Genesi:

#### 1. Criar Slides (7 imagens 1100x520px)

Use Figma, Inkscape ou GIMP para criar slides mostrando:
1. Bem-vindo ao Genesi OS
2. AI Mode
3. Auto-Updates
4. KDE Plasma Customizado
5. Developer Tools
6. Performance
7. Comunidade

#### 2. Customizar Stylesheet

Aplicar tema verde/teal em `stylesheet.qss`.

#### 3. Compilar na VM

```bash
cd ~/GenesiOS/genesi-calamares

# Instalar dependências
sudo pacman -S base-devel cmake extra-cmake-modules qt6-base qt6-svg \
               qt6-declarative kpmcore yaml-cpp boost icu polkit-qt6 qt6-tools

# Compilar
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

#### 4. Criar PKGBUILD

Criar `genesi-arch/packages/genesi-calamares/PKGBUILD`:

```bash
pkgname=genesi-calamares
pkgver=1.0.0
pkgrel=1
pkgdesc="Genesi OS installer based on Calamares"
arch=('x86_64')
url="https://github.com/zFreshy/GenesiOS"
license=('GPL3')
depends=('qt6-base' 'qt6-svg' 'qt6-declarative' 'kpmcore' 'yaml-cpp' 'boost' 'icu' 'polkit-qt6')
makedepends=('cmake' 'extra-cmake-modules' 'qt6-tools' 'git')
provides=('calamares')
conflicts=('calamares' 'cachyos-calamares' 'cachyos-calamares-next')
source=("git+https://github.com/zFreshy/GenesiOS.git#branch=main")
sha256sums=('SKIP')

build() {
    cd "$srcdir/GenesiOS/genesi-calamares"
    cmake -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_INSTALL_LIBDIR=lib \
        -DINSTALL_CONFIG=ON \
        -DINSTALL_POLKIT=ON \
        -DWITH_QT6=ON
    cmake --build build --parallel
}

package() {
    cd "$srcdir/GenesiOS/genesi-calamares"
    DESTDIR="$pkgdir" cmake --install build
}
```

#### 5. Buildar Pacote

```bash
cd genesi-arch/packages/genesi-calamares
makepkg -sf --noconfirm
mv *.pkg.tar.zst ../../local-repo/
cd ../../local-repo
repo-add genesi.db.tar.gz *.pkg.tar.zst
```

#### 6. Atualizar packages_desktop.x86_64

```bash
# Descomentar:
genesi-calamares

# Comentar:
# cachyos-calamares-next
```

#### 7. Buildar ISO

```bash
cd genesi-arch
sudo ./buildiso.sh -p desktop
```

## 📊 Status Atual

```
┌─────────────────────────────────────────────┐
│ COMPONENTE              │ STATUS            │
├─────────────────────────────────────────────┤
│ Repositório clonado     │ ✅ Feito          │
│ Branding criado         │ ✅ Feito          │
│ Logos substituídos      │ ✅ Feito          │
│ Cores customizadas      │ ✅ Feito          │
│ Slides customizados     │ ⏳ Pendente       │
│ Stylesheet customizado  │ ⏳ Pendente       │
│ Sidebar customizada     │ ⏳ Pendente       │
│ PKGBUILD criado         │ ⏳ Pendente       │
│ Pacote compilado        │ ⏳ Pendente       │
│ Integrado na ISO        │ ⏳ Pendente       │
└─────────────────────────────────────────────┘
```

## 🎯 Recomendação

**AGORA**: Use `cachyos-calamares-next` para fazer a ISO funcionar.

**DEPOIS**: Quando tiver tempo, crie os slides e compile o `genesi-calamares`.

## 💡 Por Que Usar CachyOS Calamares Por Enquanto?

1. **Funciona imediatamente** - não precisa compilar nada
2. **Já testado** - sabemos que funciona
3. **Foco no importante** - fazer a ISO bootar e funcionar
4. **Branding pode esperar** - o instalador funciona, só não tem logo do Genesi
5. **Menos problemas** - evita erros de compilação

Você pode fazer a ISO funcionar HOJE usando o Calamares do CachyOS, e depois customizar quando tiver tempo para criar os slides e testar.

## 📝 Arquivos Criados

```
GenesiOS/
├── genesi-calamares/                    ✅ Repositório clonado
│   └── src/branding/genesi/             ✅ Branding customizado
└── genesi-arch/
    ├── docs/
    │   ├── CALAMARES-SETUP.md           ✅ Este arquivo
    │   └── CALAMARES-TODO.md            ✅ Lista de tarefas
    └── packages/
        └── genesi-calamares-branding/   ⚠️  Não usar (abordagem antiga)
```

## 🔗 Links Úteis

- [CachyOS Calamares](https://github.com/CachyOS/cachyos-calamares)
- [Calamares Branding Guide](https://github.com/calamares/calamares/wiki/Branding-Guide)
- [Qt Stylesheet Reference](https://doc.qt.io/qt-6/stylesheet-reference.html)

---

**Resumo**: Tudo pronto para usar o Calamares do CachyOS agora, e customizar depois quando tiver tempo! 🚀
