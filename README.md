# Genesi OS - Arch Edition

**Genesi OS Arch Edition** é uma distribuição Linux baseada em Arch Linux, utilizando tecnologias do CachyOS para máxima performance e otimização.

## 🚀 Características

### Base do Sistema
- **Distribuição Base**: Arch Linux (rolling release)
- **Kernel**: CachyOS optimized kernel com schedulers BORE/EEVL
- **Gerenciador de Pacotes**: pacman
- **Init System**: systemd
- **Otimizações**: CachyOS settings para máxima performance

### Desktop Environment
- **Compositor**: Hyprland (Wayland compositor moderno e performático)
- **Barra**: Waybar (altamente customizável)
- **Launcher**: Wofi (leve e rápido)
- **Terminal**: Kitty (GPU-accelerated)
- **File Manager**: Thunar (leve e funcional)
- **Tema**: Genesi custom theme com gradiente roxo/azul

### Instalador
- **Calamares**: Instalador gráfico profissional
- **Particionamento**: Automático e manual
- **Suporte**: UEFI e BIOS Legacy

### Otimizações CachyOS
- **Kernel Otimizado**: Schedulers BORE/EEVL para melhor responsividade
- **Sysctl Tuning**: Parâmetros otimizados de memória e I/O
- **I/O Schedulers**: BFQ para HDDs, mq-deadline para SSDs SATA, none para NVMe
- **Audio**: Configurações otimizadas para evitar crackling
- **Network**: NetworkManager com configurações otimizadas

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
- firefox (navegador)
- kate (editor de texto)
- kcalc (calculadora)
- spectacle (screenshots)
- partitionmanager (gerenciador de partições)

### Ferramentas
- git, wget, curl
- vim, nano
- htop, inxi, neofetch
- cachyos-rate-mirrors

## 🔧 Build da ISO

### Requisitos
- Sistema Arch Linux ou derivado
- Pacotes: `archiso mkinitcpio-archiso git squashfs-tools grub`

### Instalação dos Requisitos
```bash
sudo pacman -S archiso mkinitcpio-archiso git squashfs-tools grub --needed
```

### Build
```bash
cd genesi-arch
sudo ./build-genesi-arch.sh
```

### Opções de Build
- `-c` : Não limpar diretório de trabalho antes do build
- `-r` : Buildar em RAM (requer >23GB RAM)
- `-w` : Remover diretório de trabalho após build
- `-v` : Output verboso
- `-h` : Ajuda

A ISO será gerada na pasta `out/`.

## 🎨 Customização

### Hyprland
Configurações em: `genesi-arch/airootfs/etc/skel/.config/hypr/hyprland.conf`

**Atalhos principais:**
- `SUPER + RETURN` : Abrir terminal
- `SUPER + B` : Abrir navegador
- `SUPER + E` : Abrir gerenciador de arquivos
- `SUPER + D` : Abrir launcher
- `SUPER + Q` : Fechar janela
- `SUPER + F` : Fullscreen

### Waybar
Configurações em: `genesi-arch/airootfs/etc/skel/.config/waybar/`

### Tema
- Wallpaper: `genesi-arch/airootfs/usr/share/backgrounds/genesi/wallpaper.png`
- Cores: Gradiente roxo/azul (#667eea → #764ba2)

## 📚 Estrutura do Projeto

```
genesi-arch/
├── airootfs/              # Sistema de arquivos da ISO
│   ├── etc/              # Configurações do sistema
│   ├── usr/              # Binários e recursos
│   └── root/             # Home do root
├── profiledef.sh         # Definição do perfil archiso
├── packages.x86_64       # Lista de pacotes
├── pacman.conf           # Configuração do pacman
└── build-genesi-arch.sh  # Script de build
```

## 🖥️ Testando a ISO

### VirtualBox
1. Crie uma nova VM com:
   - Tipo: Linux
   - Versão: Arch Linux (64-bit)
   - RAM: 4GB mínimo
   - Disco: 20GB mínimo
2. Configurações:
   - Sistema → Habilitar EFI
   - Display → 128MB de vídeo
   - Armazenamento → Adicionar a ISO
3. Inicie a VM

### USB Bootável
```bash
# Substitua /dev/sdX pelo seu dispositivo USB
sudo dd if=genesi-arch/out/genesi-*.iso of=/dev/sdX bs=4M status=progress oflag=sync
```

## 🔗 Tecnologias Utilizadas

- [Arch Linux](https://archlinux.org/)
- [CachyOS](https://cachyos.org/)
- [Hyprland](https://hyprland.org/)
- [Calamares](https://calamares.io/)
- [archiso](https://wiki.archlinux.org/title/Archiso)

## 📝 Credenciais Padrão

- **Usuário**: genesi
- **Senha**: genesi
- **Root**: genesi

## 🤝 Contribuindo

Contribuições são bem-vindas! Abra uma issue ou PR.

## 📧 Suporte

- Issues: [GitHub Issues](https://github.com/genesi-os/genesi-arch/issues)
- Documentação: [GitHub Wiki](https://github.com/genesi-os/genesi-arch/wiki)

## 📄 Licença

GPL-3.0-or-later
