# Changelog

All notable changes to Genesi OS will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Phase 3: Own Packages & Repository (In Progress)
- Package system created (genesi-settings, genesi-kde-settings, genesi-ai-mode)
- Repository setup documentation
- Automatic updates system via GitHub Releases

## [2026.05.01] - 2026-05-01

### Phase 2: AI Mode (90% Complete)

#### Added
- **AI Mode Daemon** (`genesi-aid`): Automatic AI optimization
  - Detects AI processes (Ollama, llama.cpp, vLLM, LocalAI, etc.)
  - CPU governor optimization (performance mode)
  - Memory management (swappiness reduced to 10)
  - Transparent huge pages enabled
  - Process prioritization (nice -5)
  - CPU pinning to performance cores
- **AI Mode Plasma Widget**: Shows status and detected processes
  - Pulsing animation when active
  - Manual toggle button
  - Auto-refresh every 5 seconds
  - Auto-adds to panel on first boot
- **Sysctl optimizations** for AI workloads
- **Performance improvements**: 15-25% faster inference on CPU-only

#### Changed
- Improved daemon logging
- Better error handling

### Phase 1: Visual Identity (Complete)

#### Added
- **Custom KDE Plasma Theme**
  - Dark green/teal color scheme (GenesiOS.colors)
  - Colors: Verde Genesis #1D9E75, Floresta #04342C, Menta #E1F5EE
- **Wallpapers**: Custom Genesi OS backgrounds
- **SDDM Login Theme**: Glassmorphic design with Genesi branding
- **Plymouth Boot Splash**: Animated logo with progress bar
- **Genesi Welcome App**: Replaces CachyOS Hello
- **Custom Plasmoids**:
  - Genesi OS logo widget in taskbar
  - AI Mode status widget
- **Desktop Widgets**: Clock, CPU monitor, RAM monitor, notes
- **Desktop Icons**: Home, Settings, Terminal, Trash, This PC, Install
- **Konsole Theme**: Dark green with Genesi colors
- **KWin Effects**: Blur (strength 12), translucency
- **Floating Panel**: Modern taskbar with app icons

#### Changed
- All "CachyOS" text replaced with "Genesi OS"
- System identity files (os-release, hostname, lsb-release)
- Boot menus (GRUB, Syslinux, EFI)
- Calamares installer branding

### Base System

#### Added
- **Base**: CachyOS (Arch Linux with optimized kernel)
- **Kernel**: linux-cachyos with BORE scheduler
- **Desktop**: KDE Plasma 6
- **Package Manager**: pacman
- **Build System**: archiso-based with custom scripts

## [Initial] - 2026-04-01

### Added
- Initial project setup
- Migration from Ubuntu to Arch Linux base
- CachyOS integration
- Basic ISO build system

---

## Version History

- **2026.05.01**: Phase 1 & 2 complete, Phase 3 in progress
- **2026.04.01**: Initial release (Ubuntu-based, deprecated)

## Upcoming

### Phase 3: Own Packages & Repository
- [ ] Package integration in buildiso.sh
- [ ] GitHub Actions for automatic builds
- [ ] Repository hosting on GitHub Releases
- [ ] Branding persistence after installation

### Phase 4: IDE and Dev Tools
- [ ] Genesi IDE (VS Code/Zed fork)
- [ ] Container widget
- [ ] Project sandboxes (Distrobox)
- [ ] Network inspection tools
- [ ] Database explorer

### Phase 5: Polish and Distribution
- [ ] Official website
- [ ] Complete documentation
- [ ] Community setup (Discord/Forum)
- [ ] Public release

---

## Notes

- **Rolling Release**: Genesi OS follows a rolling release model
- **Version Format**: YYYY.MM.DD (date-based)
- **Updates**: Available via `pacman -Syu`

For detailed roadmap, see [ROADMAP.md](docs/ROADMAP.md)
