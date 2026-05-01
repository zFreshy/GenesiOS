# genesi-kde-settings

KDE Plasma theme and settings for Genesi OS.

## Components

### Themes
- `usr/share/color-schemes/` - GenesiOS color scheme (dark green/teal)
- `usr/share/wallpapers/genesi/` - Genesi OS wallpapers
- `usr/share/sddm/themes/genesi/` - SDDM login theme
- `usr/share/plymouth/themes/genesi/` - Plymouth boot splash

### Plasmoids
- `usr/share/plasma/plasmoids/org.genesi.logo/` - Genesi logo widget
- `usr/share/plasma/plasmoids/org.genesi.aimode/` - AI Mode widget

### Default Configs
- `etc/skel/.config/` - KDE configs (kdeglobals, kwinrc, plasmarc, etc.)
- `etc/skel/Desktop/` - Desktop icons (Home, Settings, Terminal, etc.)

## Replaces

- `cachyos-kde-settings` (provides and conflicts)
- `cachyos-desktop-settings` (provides and conflicts)

## Build

```bash
cd genesi-kde-settings
makepkg -sf
```

## Install

```bash
sudo pacman -U genesi-kde-settings-*.pkg.tar.zst
```

## Features

- Dark green/teal color scheme (#1D9E75, #04342C, #E1F5EE)
- Glassmorphic effects (blur + transparency)
- Custom wallpapers
- Floating panel with app icons
- Desktop widgets (clock, CPU, RAM, notes)
- Custom SDDM login screen
- Plymouth boot animation
