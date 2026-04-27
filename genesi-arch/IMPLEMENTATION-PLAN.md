# Genesi OS - Arch Edition Implementation Plan

## 🎯 Objetivo

Migrar completamente do Ubuntu-based para Arch Linux-based, utilizando tecnologias do CachyOS para máxima performance.

## ✅ Decisões Tomadas

### 1. Arquitetura Base
- **Base**: Arch Linux (rolling release)
- **Kernel**: linux-cachyos com schedulers BORE/EEVL
- **Build System**: archiso (ferramenta oficial do Arch)
- **Gerenciador de Pacotes**: pacman + yay (AUR)

### 2. Desktop Environment
- **Principal**: Hyprland (Wayland compositor moderno)
  - Leve, rápido, altamente customizável
  - Animações suaves e bonitas
  - Configuração via arquivo de texto
- **Alternativo**: Genesi GTK4 Desktop (opcional)
  - Pode ser portado depois como opção
  - Foco inicial no Hyprland

### 3. Instalador
- **Calamares**: Sim, incluído
  - Instalador gráfico profissional
  - Customizado do CachyOS
  - Suporte a particionamento automático e manual
  - Interface amigável

### 4. Componentes do Desktop
- **Barra**: Waybar (altamente customizável)
- **Launcher**: Wofi (leve e rápido)
- **Terminal**: Kitty (GPU-accelerated)
- **File Manager**: Thunar (leve e funcional)
- **Wallpaper**: swaybg
- **Notificações**: Mako
- **Screenshots**: grim + slurp

## 📦 Tecnologias CachyOS Integradas

### 1. Kernel Otimizado
- **linux-cachyos**: Kernel com patches de performance
- **Schedulers**: BORE (Burst-Oriented Response Enhancer) ou EEVL
- **Otimizações**: x86-64-v3 para CPUs modernas

### 2. Otimizações de Sistema (cachyos-settings)
- **ZRAM**: Swap comprimido em RAM
- **Sysctl**: Parâmetros otimizados de memória e I/O
- **I/O Schedulers**: BFQ para HDDs, mq-deadline para SSDs
- **Audio**: Configurações para evitar crackling
- **Udev Rules**: Automação de configurações

### 3. Repositórios
- **cachyos**: Pacotes otimizados gerais
- **cachyos-v3**: Pacotes compilados para x86-64-v3
- **cachyos-core-v3**: Core packages otimizados
- **cachyos-extra-v3**: Extra packages otimizados

### 4. Ferramentas
- **cachyos-kernel-manager**: Gerenciar kernels facilmente
- **game-performance**: Scripts para otimizar jogos
- **pci-latency**: Reduzir latência de áudio

## 🏗️ Estrutura do Projeto

```
genesi-arch/
├── profiledef.sh              # Definição do perfil archiso
├── pacman.conf                # Configuração do pacman com repos CachyOS
├── packages.x86_64            # Lista de pacotes a instalar
├── build-genesi-arch.sh       # Script de build da ISO
├── airootfs/                  # Sistema de arquivos da ISO
│   ├── etc/
│   │   ├── hostname           # Nome do host
│   │   ├── locale.conf        # Configuração de locale
│   │   ├── locale.gen         # Locales a gerar
│   │   ├── vconsole.conf      # Configuração do console
│   │   ├── pacman.d/          # Mirrorlist do CachyOS
│   │   ├── systemd/system/    # Services customizados
│   │   └── skel/              # Arquivos padrão para novos usuários
│   │       └── .config/
│   │           ├── hypr/      # Configuração do Hyprland
│   │           ├── waybar/    # Configuração da barra
│   │           └── kitty/     # Configuração do terminal
│   ├── usr/
│   │   └── share/
│   │       └── backgrounds/   # Wallpapers
│   └── root/
│       └── .automated_script.sh  # Script de setup automático
├── efiboot/                   # Boot UEFI
└── syslinux/                  # Boot BIOS
```

## 🚀 Roadmap de Implementação

### Fase 1: Setup Básico ✅
- [x] Criar estrutura de diretórios
- [x] Configurar profiledef.sh
- [x] Configurar pacman.conf com repos CachyOS
- [x] Criar lista de pacotes (packages.x86_64)
- [x] Script de build (build-genesi-arch.sh)

### Fase 2: Configuração do Sistema ✅
- [x] Configurar hostname, locale, vconsole
- [x] Criar script de setup automático
- [x] Configurar auto-login
- [x] Configurar NetworkManager

### Fase 3: Desktop Hyprland ✅
- [x] Configuração do Hyprland
- [x] Configuração do Waybar
- [x] Configuração do Kitty
- [x] Keybindings e atalhos
- [x] Tema e cores Genesi

### Fase 4: Recursos Visuais 🔄
- [ ] Copiar wallpaper do Genesi
- [ ] Criar ícones customizados
- [ ] Configurar GTK theme
- [ ] Configurar cursor theme

### Fase 5: Calamares Installer 🔄
- [ ] Configurar módulos do Calamares
- [ ] Customizar branding
- [ ] Configurar particionamento
- [ ] Testar instalação

### Fase 6: Otimizações CachyOS 🔄
- [ ] Integrar cachyos-settings
- [ ] Configurar ZRAM
- [ ] Aplicar sysctl tweaks
- [ ] Configurar I/O schedulers

### Fase 7: Testing 🔄
- [ ] Build da ISO em sistema Arch
- [ ] Testar boot em VirtualBox
- [ ] Testar Hyprland
- [ ] Testar instalação via Calamares
- [ ] Testar aplicativos

### Fase 8: Documentação 🔄
- [ ] README completo
- [ ] Guia de build
- [ ] Guia de customização
- [ ] Troubleshooting

## 🔧 Requisitos para Build

### Sistema Host
- Arch Linux ou derivado
- Pacotes: `archiso mkinitcpio-archiso git squashfs-tools grub`
- Espaço em disco: ~10GB
- RAM: 4GB mínimo

### Comando de Build
```bash
cd genesi-arch
sudo ./build-genesi-arch.sh
```

## 📝 Notas Importantes

### Diferenças do Ubuntu
1. **Pacman vs APT**: Gerenciador de pacotes diferente
2. **Rolling Release**: Atualizações contínuas (sem versões)
3. **AUR**: Repositório comunitário com milhares de pacotes
4. **Arch Wiki**: Documentação excepcional

### Vantagens do Arch
1. **Performance**: Sistema mais leve e rápido
2. **Atualizado**: Sempre com software mais recente
3. **Customização**: Controle total do sistema
4. **CachyOS**: Otimizações adicionais de performance

### Desafios
1. **Curva de Aprendizado**: Arch é mais técnico
2. **Build Environment**: Precisa de sistema Arch para buildar
3. **Manutenção**: Rolling release requer atualizações frequentes
4. **Compatibilidade**: Alguns pacotes podem não estar disponíveis

## 🎨 Tema Visual Genesi

### Cores
- **Primary**: #667eea (Azul)
- **Secondary**: #764ba2 (Roxo)
- **Background**: #1a1a2e (Escuro)
- **Text**: #ffffff (Branco)

### Gradiente
```css
linear-gradient(135deg, #667eea 0%, #764ba2 100%)
```

### Fontes
- **Monospace**: Hack
- **UI**: DejaVu Sans
- **Icons**: Font Awesome

## 🔗 Recursos

- [Arch Linux Wiki](https://wiki.archlinux.org/)
- [CachyOS GitHub](https://github.com/CachyOS)
- [Hyprland Docs](https://wiki.hyprland.org/)
- [archiso Guide](https://wiki.archlinux.org/title/Archiso)
- [Calamares Docs](https://calamares.io/docs/)

## 📧 Próximos Passos

1. **Copiar wallpaper** do projeto atual
2. **Testar build** em sistema Arch
3. **Configurar Calamares** para instalação
4. **Criar branch separada** no git
5. **Documentar processo** de build

---

**Status**: Em Desenvolvimento  
**Última Atualização**: 2024-01-01  
**Versão**: 0.1.0-alpha
