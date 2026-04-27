# Genesi OS - Arch Edition

**Genesi OS Arch Edition** é uma distribuição Linux baseada em Arch Linux, utilizando tecnologias do CachyOS para máxima performance e otimização.

## 🚀 Características

### Base do Sistema
- **Distribuição Base**: Arch Linux (rolling release)
- **Kernel**: CachyOS optimized kernels com schedulers BORE/EEVL
- **Gerenciador de Pacotes**: pacman + yay (AUR)
- **Init System**: systemd

### Desktop Environment
- **Compositor Principal**: Hyprland (Wayland compositor moderno e performático)
- **Desktop Alternativo**: Genesi GTK4 Desktop (opcional)
- **Tema**: Genesi custom theme com gradiente roxo/azul
- **Wallpaper**: Wallpaper customizado do Genesi

### Instalador
- **Calamares**: Instalador gráfico customizado do CachyOS
- **Particionamento**: Automático e manual
- **Suporte**: UEFI e BIOS Legacy

### Otimizações CachyOS
- **Kernel Otimizado**: Schedulers BORE/EEVL para melhor responsividade
- **Sysctl Tuning**: Parâmetros otimizados de memória e I/O
- **ZRAM**: Swap comprimido em RAM com zstd
- **I/O Schedulers**: BFQ para HDDs, mq-deadline para SSDs SATA, none para NVMe
- **Audio**: Configurações otimizadas para evitar crackling
- **Gaming**: Scripts de performance para jogos

## 📦 Pacotes Incluídos

### Sistema Base
- linux-cachyos (kernel otimizado)
- base, base-devel
- networkmanager
- grub, efibootmgr
- cachyos-settings (otimizações do sistema)

### Desktop Hyprland
- hyprland (compositor Wayland)
- waybar (barra de status)
- wofi (launcher de aplicativos)
- kitty (terminal)
- thunar (gerenciador de arquivos)
- swaybg (wallpaper)

### Aplicativos
- chromium (navegador)
- gnome-system-monitor (monitor do sistema)
- gnome-calculator
- gedit (editor de texto)

### Ferramentas
- git, wget, curl
- vim, nano
- htop, neofetch
- cachyos-kernel-manager

## 🔧 Build da ISO

### Requisitos
- Sistema Arch Linux ou derivado
- Pacotes: archiso, mkinitcpio-archiso, git, squashfs-tools, grub

### Instalação dos Requisitos
```bash
sudo pacman -S archiso mkinitcpio-archiso git squashfs-tools grub --needed
```

### Build
```bash
cd genesi-arch
sudo ./build-genesi-arch.sh
```

A ISO será gerada na pasta `out/`.

## 🎨 Customização

### Hyprland
Configurações em: `airootfs/etc/skel/.config/hypr/hyprland.conf`

### Tema
- GTK Theme: `airootfs/etc/skel/.config/gtk-3.0/`
- Wallpaper: `airootfs/usr/share/backgrounds/genesi/`

### Calamares
Configurações em: `airootfs/etc/calamares/`

## 📚 Estrutura do Projeto

```
genesi-arch/
├── airootfs/              # Sistema de arquivos da ISO
│   ├── etc/              # Configurações do sistema
│   ├── usr/              # Binários e recursos
│   └── root/             # Home do root
├── efiboot/              # Boot UEFI
├── syslinux/             # Boot BIOS
├── profiledef.sh         # Definição do perfil archiso
├── packages.x86_64       # Lista de pacotes
└── build-genesi-arch.sh  # Script de build
```

## 🔗 Tecnologias Utilizadas

- [Arch Linux](https://archlinux.org/)
- [CachyOS](https://cachyos.org/)
- [Hyprland](https://hyprland.org/)
- [Calamares](https://calamares.io/)
- [archiso](https://wiki.archlinux.org/title/Archiso)

## 📝 Licença

GPL-3.0-or-later

## 🤝 Contribuindo

Contribuições são bem-vindas! Abra uma issue ou PR.

## 📧 Contato

- GitHub: [Genesi OS](https://github.com/seu-usuario/genesi-os)
