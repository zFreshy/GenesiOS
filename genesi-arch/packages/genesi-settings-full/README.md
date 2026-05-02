# genesi-settings

System branding package for Genesi OS.

## Files

- `os-release` - System identity (`/etc/os-release`)
- `lsb-release` - LSB information (`/etc/lsb-release`)
- `issue` - Login banner (`/etc/issue`)
- `hostname` - Default hostname (`/etc/hostname`)

## Replaces

- `cachyos-settings` (provides and conflicts)

## Build

```bash
cd genesi-settings
makepkg -sf
```

## Install

```bash
sudo pacman -U genesi-settings-*.pkg.tar.zst
```
