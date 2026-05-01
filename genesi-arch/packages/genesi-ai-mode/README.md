# genesi-ai-mode

AI Mode automatic optimization daemon and Plasma widget for Genesi OS.

## Components

- `genesi-aid` - Python daemon that monitors AI processes
- `genesi-aid.service` - Systemd service
- `99-genesi-ai.conf` - Sysctl optimizations
- `plasmoid-aimode/` - Plasma widget showing AI Mode status
- `genesi-add-aimode-widget.sh` - Auto-add widget to panel
- `genesi-add-aimode-widget.desktop` - XDG autostart

## Features

- Automatic detection of AI processes (Ollama, llama.cpp, vLLM, etc.)
- CPU governor optimization (performance mode)
- Memory management (reduced swappiness)
- Transparent huge pages
- Process prioritization
- CPU pinning to performance cores
- Plasma widget with status and manual toggle

## Build

```bash
cd genesi-ai-mode
makepkg -sf
```

## Install

```bash
sudo pacman -U genesi-ai-mode-*.pkg.tar.zst
```

The service will be enabled automatically and start on next boot.

## Usage

```bash
# Check status
sudo systemctl status genesi-aid

# View logs
sudo journalctl -u genesi-aid -f

# Check state
cat /var/run/genesi-aid.state
```
