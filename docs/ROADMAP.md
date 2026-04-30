# Genesi OS - Feature Roadmap

## Current Status
- ✅ Bootable ISO based on CachyOS
- ✅ KDE Plasma working
- ✅ Complete build system
- ⬜ Everything below

---

## PHASE 1: Visual Identity (High Priority)
> Give Genesi OS its own look and feel

- [ ] Custom KDE Plasma theme (colors, icons, fonts)
- [ ] Genesi OS wallpapers
- [ ] Login screen (SDDM) with Genesi branding
- [ ] Boot splash screen with Genesi logo
- [ ] "Genesi Hello" replacing "CachyOS Hello"
- [ ] Genesi OS icons and logo
- [ ] All text and links pointing to Genesi (not CachyOS)

---

## PHASE 2: AI Mode - Local AI Optimizations (Main Differentiator)
> Make Genesi OS run local AI better than any other OS

### 2.1 "Genesi AI Optimizer" Daemon (genesi-aid)
A systemd service that monitors AI processes and optimizes the system automatically.

- [ ] Detect when Ollama/llama.cpp/vLLM/LocalAI is running
- [ ] Automatically enable optimizations when AI is in use
- [ ] Disable optimizations when AI stops (return to normal)
- [ ] Plasma widget showing status (AI Mode ON/OFF, VRAM usage, loaded model)

### 2.2 VRAM/RAM Management
- [ ] Automatically detect available VRAM
- [ ] Configure optimal GPU/CPU split for the model (partial offloading)
- [ ] Free VRAM from non-essential processes when AI runs (reduce compositor visual effects)
- [ ] Use `mlock` to keep model weights in RAM without swap
- [ ] Set `vm.swappiness=10` when AI Mode is active

### 2.3 Huge Pages for Models
- [ ] Configure Transparent Huge Pages (THP) of 2MB for inference
- [ ] Pre-allocate huge pages when AI Mode is activated
- [ ] Optimized sysctl configs: `vm.nr_hugepages`, `vm.hugetlb_shm_group`

### 2.4 CPU Governor and Scheduler
- [ ] Switch CPU governor to `performance` when inference is running
- [ ] Disable power saving on cores used by AI
- [ ] CPU pinning: pin inference threads to specific cores (avoid cache thrashing)
- [ ] Use CachyOS BORE scheduler with high priority for AI processes

### 2.5 Optimized I/O for Models
- [ ] Pre-cache frequently used models in RAM with `vmtouch`
- [ ] Configure I/O scheduler to prioritize large sequential reads
- [ ] Optimize kernel readahead for large GGUF files

### 2.6 "AI Mode" Toggle in Plasma
- [ ] Taskbar widget: ON/OFF button
- [ ] When ON: reduce visual effects, enable huge pages, CPU performance, prioritize AI
- [ ] When OFF: return everything to normal
- [ ] Display: VRAM used, loaded model, tokens/second

### 2.7 Integrated MemPalace
MemPalace (https://github.com/MemPalace/mempalace) is a local-first AI memory system.
It stores conversations and context locally with semantic search, without sending anything to the cloud.

Integration with Genesi OS:
- [ ] Pre-install MemPalace on the system
- [ ] Configure as a background service
- [ ] Local AIs (Ollama, etc.) can use MemPalace for persistent memory
- [ ] Developer project context is automatically indexed
- [ ] Semantic search: "why did we switch to GraphQL?" returns the exact conversation
- [ ] Integrate with IDE (VS Code/Zed) via MemPalace MCP tools
- [ ] Plasma widget showing MemPalace status (indexed memories, last sync)

Benefit: Local AI on Genesi OS has long-term memory. The dev talks to the AI,
closes everything, and next time the AI remembers the context. No other OS offers this natively.

---

## PHASE 3: IDE and Dev Tools (Secondary Differentiator)

### 3.1 Genesi IDE (based on VS Code or Zed)
- [ ] Fork of VS Code or Zed with Genesi branding
- [ ] Pre-installed Genesi theme
- [ ] Pre-configured extensions (Git, Docker, AI, popular languages)
- [ ] Native integration with local AI daemon
- [ ] Integration with MemPalace (project context)
- [ ] Desktop and menu shortcut

### 3.2 Container Widget in Plasma
- [ ] Taskbar widget showing running Docker containers
- [ ] Start/Stop/Restart with one click
- [ ] View container logs
- [ ] View mapped ports
- [ ] CPU/RAM usage status per container

### 3.3 Project Sandboxes (Isolated Workspaces)
- [ ] Based on Distrobox/Toolbox
- [ ] GUI to create/manage workspaces
- [ ] Templates: "Java + Spring Boot", "React + Vite", "Python + FastAPI", etc.
- [ ] Each workspace has its own isolated dependencies
- [ ] Integration with Genesi IDE

### 3.4 Network Inspection
- [ ] mitmproxy pre-installed and configured
- [ ] Simple GUI to intercept HTTP/HTTPS requests
- [ ] Quick shortcut to enable/disable debug proxy
- [ ] Integration with container widget (view traffic per container)

### 3.5 Database Explorer
- [ ] Beekeeper Studio or DBeaver pre-installed
- [ ] Dolphin (file manager) plugin to connect to databases
- [ ] Support for PostgreSQL, MySQL, SQLite, MongoDB
- [ ] Quick table and data visualization

---

## PHASE 4: Polish and Distribution

- [ ] Custom Calamares installer with Genesi branding
- [ ] Official Genesi OS website
- [ ] Complete documentation
- [ ] Download page with ISOs
- [ ] Community (Discord/Forum)
- [ ] Automatic updates (own repository)

---

## PHASE 5: Own Packages and Repository (Future)

Currently, Genesi OS uses CachyOS packages (`cachyos-calamares-next`, `cachyos-kde-settings`,
`cachyos-settings`, etc.) and rebrands them at build time via `customize_airootfs.sh`.
This works but has limitations:

- The `cachyos-hello` app has hardcoded text in the binary (can't rebrand without forking)
- Calamares branding is patched via sed after package install (fragile, may break on updates)
- Some CachyOS text may appear briefly or in edge cases

**Future solution**: Create a Genesi OS package repository with forked/custom packages:
- [ ] `genesi-welcome` - Custom welcome app replacing `cachyos-hello`
- [ ] `genesi-calamares` - Forked Calamares with native Genesi OS branding
- [ ] `genesi-kde-settings` - KDE configs with Genesi OS theme built-in
- [ ] `genesi-settings` - System settings (hostname, os-release, etc.)
- [ ] Own package repository hosted on GitHub/server
- [ ] Remove dependency on `customize_airootfs.sh` for branding

---

## How to Download and Test Local AI on Genesi OS

### Method 1: Ollama (easiest)
```bash
# Install Ollama
curl -fsSL https://ollama.ai/install.sh | sh

# Download a model
ollama pull llama3.2        # 2GB, lightweight
ollama pull codellama       # 4GB, good for code
ollama pull deepseek-coder  # 1.3GB, lightweight and good for code

# Run
ollama run llama3.2

# Use via API (to integrate with apps)
curl http://localhost:11434/api/generate -d '{"model":"llama3.2","prompt":"Hello"}'
```

### Method 2: llama.cpp (more control)
```bash
# Install
sudo pacman -S llama.cpp

# Download GGUF model from HuggingFace
wget https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/resolve/main/llama-2-7b-chat.Q4_K_M.gguf

# Run
llama-cli -m llama-2-7b-chat.Q4_K_M.gguf -p "Hello, how are you?"

# Run as server (OpenAI-compatible API)
llama-server -m llama-2-7b-chat.Q4_K_M.gguf --port 8080
```

### Method 3: LocalAI (OpenAI-compatible API)
```bash
# Via Docker
docker run -p 8080:8080 localai/localai
```

### Performance Testing (for documentation)
```bash
# With Ollama - measure tokens per second
ollama run llama3.2 "Explain quantum computing" --verbose

# With llama.cpp - benchmark
llama-bench -m model.gguf

# Monitor resource usage during inference
# Terminal 1: run AI
# Terminal 2:
watch -n 1 nvidia-smi          # GPU (if NVIDIA)
htop                            # CPU and RAM
```

### Comparison for Documentation
Run the same prompt on Genesi OS (with AI Mode ON) and on a standard Ubuntu/Fedora.
Measure:
- Tokens per second
- RAM usage
- Model loading time
- VRAM usage

---

## About MemPalace

MemPalace is a local-first AI memory system that:
- Stores conversations as verbatim text (no summarizing/altering)
- Organizes into "wings" (projects), "rooms" (topics), "drawers" (content)
- Local semantic search (96.6% recall without LLM, 98.4% with heuristics)
- Everything local, nothing leaves the machine
- 29 MCP tools to integrate with any AI

How it helps Genesi OS:
1. **Persistent memory for local AI**: Dev talks to Ollama, closes, next time the AI remembers
2. **Project context**: MemPalace indexes project code and the AI understands the context
3. **Zero cloud**: Everything runs locally, aligned with Genesi OS philosophy
4. **IDE integration**: Via MCP tools, the IDE can search context in MemPalace

License: MIT (compatible with Genesi OS GPL-3.0)
