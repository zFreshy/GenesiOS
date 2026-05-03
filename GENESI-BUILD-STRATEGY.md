# 🎯 ESTRATÉGIA DE BUILD DO GENESI OS

## 📋 REPOSITÓRIOS CLONADOS

### ✅ Já temos:
1. **cachyos-live-iso-full/** - Sistema completo de build do ISO
2. **cachyos-calamares-full/** - Instalador Calamares completo
3. **cachyos-pkgbuilds/** - Todos os PKGBUILDs dos pacotes
4. **genesi-settings-full/** - Settings base
5. **genesi-welcome-full/** - App de boas-vindas
6. **cachyos-kernel-manager-full/** - Gerenciador de kernels

---

## 🔄 NOVA ABORDAGEM

### ❌ ABORDAGEM ANTIGA (não funcionava):
- Criar pacotes minimalistas
- Tentar sobrescrever com scripts
- Conflitos de arquivos
- Branding incompleto

### ✅ ABORDAGEM NOVA (vai funcionar):
1. **Usar o cachyos-live-iso-full/ como BASE**
2. **Modificar DIRETAMENTE os arquivos fonte**
3. **Buildar a partir do código modificado**
4. **Sem conflitos, sem gambiarras**

---

## 🛠️ PLANO DE EXECUÇÃO

### FASE 1: Preparar o ambiente de build
```bash
# Copiar cachyos-live-iso-full para genesi-iso
cp -r cachyos-live-iso-full genesi-iso

# Entrar no diretório
cd genesi-iso
```

### FASE 2: Modificar TODOS os arquivos de branding
```bash
# Substituir TODAS as referências
find . -type f -exec sed -i 's/CachyOS/Genesi OS/g' {} +
find . -type f -exec sed -i 's/cachyos/genesi/g' {} +
find . -type f -exec sed -i 's/CACHYOS/GENESI/g' {} +

# Cores (azul → verde)
find . -type f -exec sed -i 's/#3daee9/#00ff9f/g' {} +
find . -type f -exec sed -i 's/#232629/#0a0f0d/g' {} +
```

### FASE 3: Substituir logos e imagens
```bash
# Copiar nossos logos para todos os lugares
# - SDDM theme
# - Plymouth theme
# - Calamares
# - Wallpapers
# - Ícones
```

### FASE 4: Modificar lista de pacotes
```bash
# Em genesi-iso/archiso/packages_desktop.x86_64
# Remover: cachyos-settings, cachyos-kde-settings, cachyos-hello
# Adicionar: genesi-settings, genesi-kde-settings, genesi-hello
```

### FASE 5: Buildar o ISO
```bash
cd genesi-iso
sudo ./buildiso.sh -p desktop
```

---

## 📦 PACOTES QUE VAMOS CRIAR

### 1. genesi-settings (baseado em CachyOS-Settings)
- Configurações do sistema
- Scripts utilitários
- Configurações de rede, audio, etc

### 2. genesi-kde-settings (baseado em cachyos-kde-settings)
- Tema KDE completo
- Glassmorphism
- Cores verde/teal
- Widgets personalizados

### 3. genesi-calamares-branding (baseado em cachyos-calamares)
- Branding do instalador
- Slides personalizados
- Logo e cores

### 4. genesi-hello (baseado em cachyos-hello)
- App de boas-vindas
- Links para documentação
- Ferramentas úteis

### 5. genesi-ai-mode (NOSSO - único)
- Detecção de AI workloads
- Otimizações automáticas
- Widget do Plasma

### 6. genesi-updater (NOSSO - único)
- Sistema de atualizações
- Notificações

---

## 🎨 ARQUIVOS DE BRANDING A MODIFICAR

### Logos:
- `/usr/share/pixmaps/cachyos.svg` → `genesi.svg`
- `/usr/share/icons/cachyos.svg` → `genesi.svg`
- SDDM theme logos
- Plymouth theme logos
- Calamares logos

### Wallpapers:
- `/usr/share/wallpapers/cachyos/` → `/usr/share/wallpapers/genesi/`
- SDDM background
- Calamares background

### Temas:
- `/usr/share/color-schemes/CachyOS.colors` → `GenesiOS.colors`
- `/usr/share/plasma/desktoptheme/cachyos/` → `genesi/`
- `/usr/share/sddm/themes/cachyos/` → `genesi/`
- `/usr/share/plymouth/themes/cachyos/` → `genesi/`

### Configurações:
- `/etc/os-release`
- `/etc/lsb-release`
- `/etc/hostname`
- `/etc/issue`

---

## ✅ VANTAGENS DESTA ABORDAGEM

1. **Controle Total** - Modificamos o código fonte diretamente
2. **Sem Conflitos** - Não tentamos sobrescrever pacotes instalados
3. **Completo** - Pegamos TUDO do CachyOS, não só partes
4. **Manutenível** - Podemos fazer merge de updates do CachyOS
5. **Profissional** - É assim que distros derivadas são feitas

---

## 🚀 PRÓXIMOS PASSOS

1. ✅ Clonar todos os repos (FEITO)
2. ⏳ Copiar cachyos-live-iso-full → genesi-iso
3. ⏳ Fazer substituições em massa (sed)
4. ⏳ Substituir logos e imagens
5. ⏳ Modificar lista de pacotes
6. ⏳ Buildar e testar
7. ⏳ Ajustar o que faltar
8. ⏳ Repetir até perfeito

---

## 💡 OBSERVAÇÕES

- Vamos manter os repos clonados em `.gitignore`
- Eles são apenas "source" para nosso build
- O `genesi-iso/` será nosso diretório de trabalho
- Podemos fazer `git pull` nos repos do CachyOS para pegar updates

---

**ESTA É A ABORDAGEM CORRETA!** 🎯
