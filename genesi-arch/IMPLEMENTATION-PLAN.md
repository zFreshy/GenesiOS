# Genesi OS - Implementation Plan

## Overview

Genesi OS is a Linux distribution based on CachyOS/Arch Linux, designed for developers with native local AI optimizations.

## Current Status

- ✅ Bootable ISO based on CachyOS (complete build system)
- ✅ KDE Plasma desktop working
- ✅ Calamares installer functional
- ✅ CachyOS kernel with BORE scheduler
- ✅ All CachyOS optimizations included
- ⬜ Custom visual identity
- ⬜ AI Mode optimizations
- ⬜ Developer tools integration

## Architecture

### System Base
- **Base**: Arch Linux (rolling release)
- **Kernel**: linux-cachyos with BORE/EEVL schedulers
- **Build System**: CachyOS buildiso.sh + archiso
- **Package Manager**: pacman
- **Desktop**: KDE Plasma (Wayland)
- **Display Manager**: plasma-login-manager
- **Installer**: Calamares

### Build Requirements
- **Host OS**: CachyOS (recommended) or Arch-based
- **Packages**: `archiso git`
- **Disk Space**: ~15GB for build
- **RAM**: 4GB minimum

### Build Command
```bash
cd genesi-arch
chmod +x buildiso.sh util*.sh
sudo ./buildiso.sh -p desktop
```

ISO output: `genesi-arch/out/desktop/`

## Project Structure

```
genesi-arch/
├── archiso/                    # ISO profile (CachyOS-based)
│   ├── airootfs/               # Filesystem overlay
│   ├── efiboot/                # EFI boot config
│   ├── grub/                   # GRUB config
│   ├── syslinux/               # Syslinux config
│   ├── packages_desktop.x86_64 # Package list
│   ├── pacman.conf             # Pacman config (with CachyOS repos)
│   └── profiledef.sh           # Archiso profile definition
├── buildiso.sh                 # Main build script
├── util-iso.sh                 # ISO utilities (display manager, motd, etc.)
├── util-iso-mount.sh           # Mount utilities
├── util-msg.sh                 # Message/logging utilities
└── util.sh                     # General utilities
```

## Roadmap

See [docs/ROADMAP.md](../docs/ROADMAP.md) for the full feature roadmap.

## License

GPL-3.0-or-later. Based on CachyOS.
