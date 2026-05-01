# Genesi OS - Based on CachyOS

This directory contains the complete Genesi OS build system, based on CachyOS live-iso.

## What Makes Genesi OS Different?

Genesi OS is the **first Linux distribution optimized for local AI development**. It automatically detects when you're running AI models (Ollama, llama.cpp, vLLM, etc.) and optimizes the system for maximum performance - no configuration needed.

### Key Features

- ✅ **AI Mode**: Automatic system optimization when running local AI
- ✅ **Custom KDE Plasma Theme**: Dark green/teal glassmorphic design
- ✅ **Zero Configuration**: Everything works out of the box
- ✅ **Based on CachyOS**: Optimized kernel and packages
- ✅ **Developer-Focused**: Built for AI developers and enthusiasts

### Phase 1: Visual Identity ✅ COMPLETE
- Custom Genesi OS branding throughout
- Dark green theme with glassmorphism
- Custom wallpapers, login screen, boot splash
- Desktop widgets and custom Plasma plasmoids
- Genesi Welcome app

### Phase 2: AI Mode ✅ CORE COMPLETE (Phase 2.1 pending)
- Automatic AI process detection
- CPU governor optimization (performance mode)
- Memory management (reduced swappiness)
- Transparent huge pages for faster inference
- Process prioritization
- Plasma widget showing AI Mode status
- 15-25% performance improvement on CPU-only systems
- **Pending for Phase 2.1**: Manual toggle, VRAM monitoring, GPU detection, model caching

See `docs/PHASE2-AI-MODE.md` for testing guide.

## Structure

This is a **complete copy** of the CachyOS archiso structure with Genesi branding.

- `profiledef.sh` - ISO profile definition (adapted for Genesi)
- `pacman.conf` - Package manager configuration
- `packages_desktop.x86_64` - Package list
- `buildiso.sh` - Build script
- `airootfs/` - Root filesystem overlay
- `syslinux/` - BIOS boot configuration
- `grub/` - UEFI boot configuration
- `efiboot/` - EFI boot files

## Build

```bash
sudo ./buildiso.sh
```

The ISO will be generated in `out/`.

## Credits

Based on [CachyOS](https://cachyos.org/) - All credits to the CachyOS team for their excellent work.

## License

GPL-3.0-or-later (same as CachyOS)
