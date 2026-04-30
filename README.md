# Genesi OS

**Genesi OS** is a Linux distribution based on Arch Linux, built on top of CachyOS technologies for maximum performance and optimization. Designed for developers, with native local AI optimizations.

## 🚀 Features

### System Base
- **Base Distribution**: Arch Linux (rolling release)
- **Kernel**: CachyOS optimized kernel with BORE/EEVL schedulers
- **Package Manager**: pacman
- **Init System**: systemd
- **Optimizations**: CachyOS settings for maximum performance

### Desktop Environment
- **Desktop**: KDE Plasma (Wayland)
- **Display Manager**: plasma-login-manager (auto-login for live session)
- **Terminal**: Konsole
- **File Manager**: Dolphin
- **Browser**: Firefox

### Installer
- **Calamares**: Professional graphical installer
- **Partitioning**: Automatic and manual
- **Support**: UEFI and BIOS Legacy

### CachyOS Optimizations
- **Optimized Kernel**: BORE/EEVL schedulers for better responsiveness
- **Sysctl Tuning**: Optimized memory and I/O parameters
- **I/O Schedulers**: BFQ for HDDs, mq-deadline for SATA SSDs, none for NVMe
- **Audio**: Optimized settings to avoid crackling
- **Network**: NetworkManager with optimized settings

## 🔧 Building the ISO

### ⚠️ Important Requirements

**You MUST build on CachyOS!** (recommended) or another Arch-based system.

✅ **Works on**:
- CachyOS (recommended)
- Arch Linux
- Manjaro (partial - some packages may fail)
- EndeavourOS

❌ **Does NOT work on**:
- Ubuntu / Debian
- Fedora
- Windows / macOS

### Install Dependencies

```bash
sudo pacman -S archiso git --needed
```

### Clone and Build

```bash
git clone https://github.com/zFreshy/GenesiOS.git
cd GenesiOS
git checkout arch-base
cd genesi-arch
chmod +x buildiso.sh util*.sh
sudo ./buildiso.sh -p desktop
```

The ISO will be generated in `genesi-arch/out/desktop/`

### Build Options
- `-c` : Don't clean work directory before build
- `-v` : Verbose output
- `-r` : Build in RAM (requires >23GB RAM, much faster)
- `-w` : Remove build directory after ISO is built

### Cleaning Previous Build

```bash
sudo rm -rf build/ out/
```

## 📚 Project Structure

```
genesi-arch/
├── archiso/                   # ISO profile (CachyOS-based)
│   ├── airootfs/              # Filesystem overlay
│   │   ├── etc/               # System configuration
│   │   ├── usr/               # Binaries and resources
│   │   └── root/              # Root home
│   ├── efiboot/               # EFI boot configuration
│   ├── grub/                  # GRUB configuration
│   ├── syslinux/              # Syslinux configuration
│   ├── packages_desktop.x86_64  # Package list
│   ├── pacman.conf            # Pacman configuration
│   └── profiledef.sh          # Archiso profile definition
├── buildiso.sh                # Main build script
├── util-iso.sh                # ISO build utilities
├── util-iso-mount.sh          # Mount utilities
├── util-msg.sh                # Message utilities
└── util.sh                    # General utilities
```

## 🖥️ Testing the ISO

### VirtualBox
1. Create a new VM:
   - Type: Other / Other 64-bit
   - RAM: 4GB minimum
   - Disk: 20GB minimum
2. Settings:
   - Display → Video Memory: 128MB
   - Storage → Add the ISO as optical disk
3. Start the VM

### Bootable USB
```bash
# Replace /dev/sdX with your USB device
sudo dd if=genesi-arch/out/desktop/genesi-*.iso of=/dev/sdX bs=4M status=progress oflag=sync
```

## 📝 Default Credentials (Live Session)

- **User**: liveuser
- **Password**: (empty)

## 🗺️ Roadmap

See [docs/ROADMAP.md](docs/ROADMAP.md) for the full feature roadmap, including:
- Phase 1: Visual Identity (custom theme, branding)
- Phase 2: AI Mode (local AI optimizations, MemPalace integration)
- Phase 3: Dev Tools (IDE, containers, sandboxes)
- Phase 4: Polish and Distribution

## 🔗 Technologies

- [Arch Linux](https://archlinux.org/)
- [CachyOS](https://cachyos.org/)
- [KDE Plasma](https://kde.org/plasma-desktop/)
- [Calamares](https://calamares.io/)
- [archiso](https://wiki.archlinux.org/title/Archiso)

## 🤝 Contributing

Contributions are welcome! Open an issue or PR.

## 📧 Support

- Issues: [GitHub Issues](https://github.com/zFreshy/GenesiOS/issues)

## 📄 License

GPL-3.0-or-later

Based on [CachyOS](https://cachyos.org/) - credit to the CachyOS team for the excellent base system.
