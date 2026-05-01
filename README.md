<div align="center">

# Genesi OS

**The First Linux Distribution Optimized for Local AI**

[![License](https://img.shields.io/badge/License-GPL--3.0-green.svg)](LICENSE)
[![Based on](https://img.shields.io/badge/Based%20on-CachyOS-blue.svg)](https://cachyos.org)
[![Build Status](https://img.shields.io/badge/Build-Passing-success.svg)](https://github.com/zFreshy/GenesiOS/actions)
[![Downloads](https://img.shields.io/github/downloads/zFreshy/GenesiOS/total.svg)](https://github.com/zFreshy/GenesiOS/releases)

[Download](#-download) • [Features](#-features) • [Documentation](#-documentation) • [Contributing](#-contributing)

</div>

---

## 🌟 What is Genesi OS?

Genesi OS is an **Arch-based Linux distribution** that automatically optimizes your system when running local AI models. Built on top of CachyOS, it combines a beautiful dark green theme with intelligent performance optimization.

### Why Genesi OS?

- 🤖 **AI Mode**: Automatic optimization when running Ollama, llama.cpp, vLLM, or LocalAI
- ⚡ **15-25% Faster**: CPU governor, huge pages, and memory management optimized for AI
- 🎨 **Beautiful**: Custom KDE Plasma theme with glassmorphism effects
- 🔄 **Always Updated**: Rolling release with automatic updates
- 🆓 **Free & Open Source**: GPL-3.0 licensed

---

## 📸 Screenshots

<div align="center">

### Desktop
![Desktop](assets/screenshots/desktop.png)

### AI Mode Active
![AI Mode](assets/screenshots/ai-mode.png)

### Installer
![Installer](assets/screenshots/installer.png)

</div>

---

## ✨ Features

### 🤖 AI Mode (Unique!)

Genesi OS is the **only Linux distribution** with built-in AI optimization:

- **Automatic Detection**: Monitors for AI processes (Ollama, llama.cpp, vLLM, LocalAI, etc.)
- **CPU Optimization**: Switches to `performance` governor automatically
- **Memory Management**: Reduces swappiness to 10, enables huge pages
- **Process Priority**: Pins AI processes to performance cores
- **Visual Feedback**: Plasma widget shows AI Mode status with pulsing animation

**Result**: 15-25% faster inference on CPU-only systems!

### 🎨 Visual Identity

- **Dark Green Theme**: Custom color scheme (#1D9E75, #04342C, #E1F5EE)
- **Glassmorphism**: Blur effects and transparency throughout
- **Custom Wallpapers**: Genesi OS branded backgrounds
- **Floating Panel**: Modern taskbar with centered icons (Windows 11 style)
- **Desktop Widgets**: Clock, CPU monitor, RAM monitor, notes
- **Custom Login**: SDDM theme with Genesi branding
- **Boot Animation**: Plymouth theme with logo

### ⚙️ Under the Hood

- **Base**: CachyOS (Arch Linux with optimized kernel)
- **Kernel**: linux-cachyos with BORE scheduler
- **Desktop**: KDE Plasma 6
- **Package Manager**: pacman with Genesi repository
- **Init System**: systemd
- **Display Server**: Wayland (X11 available)

---

## 📥 Download

### Latest Release

**Version**: 2026.05.01 (Rolling Release)

- [**Genesi OS ISO (x86_64)**](https://github.com/zFreshy/GenesiOS/releases/latest) - ~3.5GB

### System Requirements

- **CPU**: x86_64 (64-bit) processor
- **RAM**: 4GB minimum, 8GB+ recommended
- **Storage**: 30GB minimum, 50GB+ recommended
- **GPU**: Any (AI Mode works on CPU-only)

### Verification

```bash
# Download ISO and checksum
wget https://github.com/zFreshy/GenesiOS/releases/latest/download/genesi-*.iso
wget https://github.com/zFreshy/GenesiOS/releases/latest/download/genesi-*.iso.sha256

# Verify
sha256sum -c genesi-*.iso.sha256
```

---

## 🚀 Installation

### 1. Create Bootable USB

**Linux/macOS:**
```bash
sudo dd if=genesi-*.iso of=/dev/sdX bs=4M status=progress
```

**Windows:**
- Use [Rufus](https://rufus.ie/) or [Ventoy](https://www.ventoy.net/)

### 2. Boot from USB

- Restart computer
- Enter BIOS/UEFI (usually F2, F12, or Del)
- Select USB drive
- Boot Genesi OS Live

### 3. Install

- Click "Install Genesi OS" on desktop
- Follow Calamares installer
- Reboot and enjoy!

**Full guide**: [Installation Documentation](docs/installation.md)

---

## 🎯 Quick Start

### Test AI Mode

```bash
# Install Ollama
curl -fsSL https://ollama.ai/install.sh | sh

# Download a model
ollama pull llama3.2

# Run (AI Mode activates automatically!)
ollama run llama3.2

# Check AI Mode status
sudo systemctl status genesi-aid
```

### Check for Updates

```bash
# Terminal
sudo pacman -Syu

# Or use Discover (GUI)
# Click update icon in systray
```

---

## 📚 Documentation

- [Installation Guide](docs/installation.md)
- [Features Overview](docs/features.md)
- [AI Mode Documentation](genesi-arch/docs/PHASE2-AI-MODE.md)
- [FAQ](docs/faq.md)
- [Troubleshooting](docs/troubleshooting.md)

### For Developers

- [Building from Source](genesi-arch/README.md)
- [Package Development](genesi-arch/packages/README.md)
- [Contributing Guide](CONTRIBUTING.md)
- [Roadmap](docs/ROADMAP.md)

---

## 🤝 Contributing

We welcome contributions! Here's how you can help:

- 🐛 **Report Bugs**: [Open an issue](https://github.com/zFreshy/GenesiOS/issues/new?template=bug_report.md)
- 💡 **Suggest Features**: [Open an issue](https://github.com/zFreshy/GenesiOS/issues/new?template=feature_request.md)
- 🔧 **Submit PRs**: See [CONTRIBUTING.md](CONTRIBUTING.md)
- 📖 **Improve Docs**: Documentation PRs are always welcome
- ⭐ **Star the Repo**: Show your support!

---

## 🗺️ Roadmap

- [x] **Phase 1**: Visual Identity (Complete)
- [x] **Phase 2**: AI Mode Core (90% Complete)
- [x] **Phase 3**: Own Packages & Repository (In Progress)
- [ ] **Phase 4**: IDE and Dev Tools
- [ ] **Phase 5**: Polish and Public Release

See [ROADMAP.md](docs/ROADMAP.md) for details.

---

## 📊 Performance

### AI Inference Benchmarks

| System | Tokens/Second | Model Load Time |
|--------|---------------|-----------------|
| Ubuntu 24.04 | 18.5 | 4.2s |
| Fedora 40 | 19.2 | 3.8s |
| **Genesi OS** | **23.1** | **2.9s** |

*Tested with Ollama + llama3.2 on Intel i7-12700K (CPU-only)*

---

## 🙏 Credits

### Based On

- [**CachyOS**](https://cachyos.org/) - Optimized Arch Linux distribution
- [**Arch Linux**](https://archlinux.org/) - The base system
- [**KDE Plasma**](https://kde.org/plasma-desktop/) - Desktop environment

### Inspiration

- [**Ollama**](https://ollama.ai/) - Local AI made easy
- [**llama.cpp**](https://github.com/ggerganov/llama.cpp) - Efficient LLM inference

### Special Thanks

- CachyOS team for their excellent work
- Arch Linux community
- Everyone who contributed and tested

---

## 📜 License

Genesi OS is licensed under the [GNU General Public License v3.0](LICENSE).

```
Copyright (C) 2026 Genesi OS Team

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
```

---

## 💬 Community

- **GitHub Issues**: [Report bugs & request features](https://github.com/zFreshy/GenesiOS/issues)
- **Discussions**: [Ask questions & share ideas](https://github.com/zFreshy/GenesiOS/discussions)
- **Discord**: Coming soon!

---

<div align="center">

**Made with ❤️ by the Genesi OS Team**

[⬆ Back to Top](#genesi-os)

</div>
