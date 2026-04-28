# Genesi OS - Based on CachyOS

This directory contains the complete Genesi OS build system, based on CachyOS live-iso.

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
