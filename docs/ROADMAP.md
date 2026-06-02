# Genesi OS — Feature Roadmap

> Genesi OS is a CachyOS-based (Arch under the hood) Linux distribution built
> around one idea: **the system should optimize itself for local AI** while
> staying beautiful, fast, and effortless to maintain.

## Current Status

| Area | Status |
|------|--------|
| Bootable ISO based on CachyOS | ✅ Complete |
| KDE Plasma 6 desktop | ✅ Complete |
| Reproducible build system (archiso + Calamares) | ✅ Complete |
| **Phase 1 — Visual Identity** | ✅ Complete |
| **Phase 2 — AI Mode (local AI optimizations)** | 🟩 ~90% (core shipping) |
| **Phase 3 — Own Packages & Repository** | ✅ Operational (8 packages shipping) |
| **Phase 4 — IDE & Dev Tools** | ⬜ Pending |
| **Phase 5 — Polish & Distribution** | ⬜ Pending |

### Two production CI pipelines

Genesi OS ships through **two independent GitHub Actions pipelines**:

1. **Package / Update pipeline** (`.github/workflows/publish-packages.yml`) —
   builds the eight Genesi packages inside a `cachyos-v3` container, runs
   `repo-add`, and commits the resulting pacman repository to
   `genesi-arch/repo/x86_64`. Installed systems pull from this repo, so a
   normal `pacman -Syu` (or the in-OS update notifier) delivers updates in near
   real time. `main` = **stable** channel, `develop` = **testing** channel.
2. **ISO pipeline** (`.github/workflows/iso-pipeline.yml`) — a two-stage build
   that first **validates the install** (dependency dry-run + a real `pacstrap`
   into a throwaway root) and only then runs `mkarchiso` to produce a fresh
   `.iso`. Artifacts are uploaded per run; pushing a `v*` tag cuts a GitHub
   Release. It only fires on ISO inputs (docs-only commits are skipped).

See [Build & Release Infrastructure](#build--release-infrastructure) for details.

---

## Phase Order

1. **Phase 1** — Visual Identity ✅ **Complete**
2. **Phase 2** — AI Mode (local AI optimizations) 🟩 **~90%**
3. **Phase 3** — Own Packages & Repository (infrastructure) ✅ **Operational**
4. **Phase 4** — IDE & Dev Tools ⬜ Pending
5. **Phase 5** — Polish & Distribution ⬜ Pending

---

## PHASE 1: Visual Identity ✅ COMPLETE
> Give Genesi OS its own look and feel.

- [x] Custom KDE Plasma theme (colors, icons, fonts)
- [x] Genesi OS wallpapers
- [x] Login screen (SDDM) with Genesi branding
- [x] Boot splash (Plymouth theme — activates when Plymouth is installed)
- [x] "Genesi Welcome" app replacing "CachyOS Hello"
- [x] Genesi OS icons and logo (hicolor 48/64/256px)
- [x] All text and links pointing to Genesi (not CachyOS)
- [x] Custom color scheme (`GenesiOS.colors`) — dark green/teal palette
- [x] Konsole theme with Genesi colors
- [x] Desktop icons (This PC, Home, Settings, Trash, Terminal, Install)
- [x] Desktop widgets (clock, CPU monitor, RAM monitor, notes)
- [x] Custom GenesiOS logo plasmoid in the taskbar
- [x] Wallpaper applied automatically on boot
- [x] Hostname, `os-release`, `lsb-release` all branded "Genesi OS"
- [x] Boot message (MOTD) shows "Welcome to Genesi OS"
- [x] GRUB / Syslinux / EFI boot menus show "Genesi OS"
- [x] Calamares installer rebranded to "Genesi OS Installer"
- [x] KWin blur and translucency (glassmorphism)
- [x] Floating panel with app icons
- [x] **Rounded window corners** — 14px via Klassy (shipped in `genesi-settings`)

---

## PHASE 2: AI Mode — Local AI Optimizations 🟩 ~90%
> Make Genesi OS run local AI better than any other desktop OS.

### 2.1 "Genesi AI Optimizer" daemon (`genesi-aid`)
A systemd service that monitors AI processes and tunes the system automatically.

- [x] Detect when Ollama / llama.cpp / vLLM / LocalAI is running
- [x] Automatically enable optimizations when AI is in use
- [x] Disable optimizations when AI stops (**fully reversible** — captures and
      restores every knob; survives a daemon restart via `/run` snapshot)
- [x] Reliable user↔daemon IPC over `/run/genesi-ai-mode` (works under the
      service's `PrivateTmp` hardening)
- [x] `genesi-ai-mode` CLI — `on` / `off` / `auto` / `toggle` / `status`
- [x] Plasma widget showing status (AI Mode ON/OFF, detected processes)

### 2.2 VRAM/RAM management
- [x] Set `vm.swappiness=10` when AI Mode is active (restored on exit)
- [ ] Detect available VRAM per GPU (NVIDIA `nvidia-smi`, AMD sysfs) → see 2.8
- [ ] Configure optimal GPU/CPU split for the model (partial offloading)
- [ ] Use `mlock`/`vmtouch` to keep model weights in RAM without swap
- [ ] Free VRAM by trimming compositor effects **from the user session** (the
      old root-side `qdbus` call could never reach the user's KWin — removed)

### 2.3 Huge pages for models
- [x] Toggle Transparent Huge Pages to `always` while AI runs (restored on exit)
- [x] Slim sysctl (`vm.max_map_count`, `vm.vfs_cache_pressure`) — **dropped the
      permanent `vm.nr_hugepages=512`** that reserved ~1 GB even with no AI running
- [ ] Allocate explicit huge pages **on demand** at enable, free them at disable

### 2.4 CPU governor and scheduler
- [x] Switch CPU governor to `performance` when inference is running (restored)
- [x] High priority for AI processes (`nice -5`, idempotent — also catches
      processes that spawn after AI Mode is already on)
- [x] **Removed** the naive "pin to first half of cores" heuristic — it halved
      CPU-inference throughput. Replaced by hybrid-core awareness in 2.8
- [ ] amd-pstate / EPP → performance; per-core power-save off (see 2.8)

### 2.5 Optimized I/O for models
- [x] Optimize kernel readahead for large GGUF files (sysctl)
- [ ] Pre-cache frequently used models in RAM with `vmtouch`
- [ ] I/O scheduler tuned for large sequential reads on the model NVMe

### 2.6 "AI Mode" widget in Plasma
- [x] Taskbar widget showing AI Mode status
- [x] Display detected AI processes with PIDs
- [x] Show applied optimizations (governor, swappiness, huge pages, priority)
- [x] Auto-refresh every 5 seconds + pulsing animation when active
- [x] Manual ON/OFF toggle (force AI Mode) — wired to the `genesi-ai-mode` CLI
- [ ] Live VRAM / GPU / tokens-per-second metrics (see 2.8)
- [ ] Rewrite for Plasma 6 API (current QML uses Plasma 5 imports)

### 2.7 Integrated MemPalace
[MemPalace](https://github.com/MemPalace/mempalace) is a local-first AI memory
system — it stores conversations and context locally with semantic search,
nothing leaves the machine.

- [ ] Pre-install MemPalace on the system
- [ ] Configure as a background service
- [ ] Local AIs (Ollama, etc.) can use MemPalace for persistent memory
- [ ] Developer project context is automatically indexed
- [ ] Semantic search: "why did we switch to GraphQL?" returns the exact conversation
- [ ] Integrate with the IDE (VS Code/Zed) via MemPalace MCP tools
- [ ] Plasma widget showing MemPalace status (indexed memories, last sync)

Benefit: local AI on Genesi OS gains long-term memory. The dev talks to the AI,
closes everything, and next time the AI still remembers the context.

---

### 2.8 AI Mode 2.0 — Universal Hardware Optimizer 🚀
> Turn AI Mode from a fixed CPU/RAM tweak into an **adaptive optimizer that
> profiles the machine and applies only what that hardware can benefit from** —
> bare-metal desktop, gaming laptop, NVIDIA/AMD/Intel GPU, hybrid CPU, low-RAM
> box, or a VM. Every change stays **fully reversible**.

#### 2.8.0 Foundation — hardware detection & architecture
The brain that makes "optimize for ANY PC" real. The daemon profiles the host
once and gates every optimizer on detected capabilities.

- [ ] `HardwareProfile`: CPU (vendor, physical cores, **hybrid P/E split**,
      virtualized?), GPU (NVIDIA/AMD/Intel + VRAM), total RAM, chassis
      (laptop/desktop), power source (AC/battery)
- [ ] Capability-gated optimizer plugins — each captures original → applies →
      restores, and is skipped when the hardware can't use it
- [ ] **VM awareness**: report "virtualized — limited gains" and skip no-op knobs
      (e.g. CPU governor doesn't exist under VirtualBox)
- [ ] Profiles: **Max Performance / Balanced / Battery-aware** (don't nuke power
      on battery unless the user forces it)
- [ ] Enrich `state.json` with the hardware profile + exactly-what-changed list

#### 2.8.1 🔥 GPU performance mode (biggest missing win)
- [ ] **NVIDIA**: persistence mode (`nvidia-smi -pm 1`), power limit to max
      (`-pl`), lock GPU/mem clocks at max, report VRAM use
- [ ] **AMD**: `power_dpm_force_performance_level=high`, compute power profile
      via sysfs (`pp_power_profile_mode`)
- [ ] **Intel Arc/iGPU**: max GPU frequency via sysfs where supported
- [ ] Restore each GPU to its prior power/clock state on disable

#### 2.8.2 🔥 Power / platform profile
- [ ] `powerprofilesctl set performance` while AI runs (restore prior profile)
- [ ] `/sys/firmware/acpi/platform_profile` → `performance` where available
- [ ] On laptops this unlocks the full CPU+GPU power/thermal budget

#### 2.8.3 🔥 Model in RAM (load fast, never stall)
- [ ] `vmtouch` to preload the active GGUF into page cache
- [ ] `mlock` the model so it can't be evicted mid-inference
- [ ] Optionally pre-cache the most-recently-used models

#### 2.8.4 🔥 Smart CPU threads & core placement
- [ ] Detect **physical** cores and P-core/E-core topology (`/sys` capacity)
- [ ] Set inference thread count to physical P-cores; avoid SMT/E-core contention
- [ ] `cpuset`/cgroup the AI process onto performance cores (system keeps the rest)

#### 2.8.5 ⚡ Quiet the background during inference
- [x] Pause file indexers (`baloo`, `tracker`) with SIGSTOP while AI runs, resume
      with SIGCONT on disable (safe + saves power; package managers untouched)
- [ ] Also pause other CPU/RAM/IO hogs on demand (opt-in list)
- [ ] Compositor effect trimming done **in the user session** (helper reads
      `state.json` and toggles KWin blur/effects)

#### 2.8.6 ⚡ Inference-engine auto-tuning
- [x] Ollama defaults via systemd drop-in: `OLLAMA_FLASH_ATTENTION=1`,
      `OLLAMA_KV_CACHE_TYPE=q8_0`, `OLLAMA_KEEP_ALIVE=15m`
- [ ] `OLLAMA_NUM_PARALLEL` / `OLLAMA_MAX_LOADED_MODELS` tuned to RAM/VRAM
- [ ] Auto-pick `num_gpu` (offload layers) from detected VRAM vs model size
- [ ] Equivalent flags for llama.cpp / llama-server

#### 2.8.7 ⚡ I/O, NUMA & scheduler
- [ ] I/O scheduler (`none`/`mq-deadline`) + readahead on the model's NVMe
- [ ] NUMA pinning (`numactl`) on multi-socket / Threadripper
- [ ] Integrate CachyOS `sched-ext` throughput schedulers while AI Mode is on

#### 2.8.8 🧠 Intelligence, metrics & UX
- [ ] **Live metrics** in the widget: tokens/s (Ollama `/api/ps`), GPU/VRAM/CPU,
      temperature, package power — plus a before/after of what AI Mode changed
- [ ] **Thermal guard**: back off if the CPU/GPU is throttling (so "max perf"
      never becomes net-slower)
- [ ] **`genesi-ai-mode bench`**: run an identical prompt with AI Mode OFF then
      ON and print the tokens/s delta (with VM caveat)
- [ ] **Model advisor**: given model + hardware, recommend quant / offload / context
- [ ] Rewrite the widget for the Plasma 6 API with a richer dashboard

#### 2.8.9 ✨ Advanced / opt-in (with tradeoffs)
- [ ] Explicit huge pages allocated on enable, freed on disable
- [ ] Disable CPU security mitigations (opt-in, clearly flagged) for max throughput
- [ ] Disable PCIe ASPM / USB autosuspend for the inference GPU

---

## PHASE 3: Own Packages & Repository ✅ OPERATIONAL
> Native Genesi packages and a self-hosted pacman repository, so branding and
> features persist **after installation to disk** — not just on the live ISO.

Early Genesi OS rebranded CachyOS packages at build time via
`customize_airootfs.sh`. That worked on the live medium but reverted to CachyOS
once installed. Phase 3 replaces that with **real, conflicting/`provides`
packages** built and published by CI.

### Shipping packages (built by `publish-packages.yml`)
- [x] `genesi-settings` — system branding (`os-release`, hostname, MOTD, sysctl)
- [x] `genesi-kde-settings` — KDE Plasma theme, wallpapers, Klassy 14px corners,
      Darkly glassmorphism, Kickoff sizing, panel layout
- [x] `genesi-ai-mode` — AI Mode daemon (`genesi-aid`), systemd service, plasmoid
- [x] `genesi-update` — interactive update notifier + systray applet
      (fork of CachyOS `cachy-update`)
- [x] `genesi-channel` — switch between **stable** and **testing** update channels
- [x] `genesi-calamares` — Calamares installer build
- [x] `genesi-calamares-branding` — native installer branding (logo, slideshow, colors)
- [x] `genesi-welcome` — first-run welcome app replacing `cachyos-hello`

### Repository & delivery
- [x] In-repo pacman registry at `genesi-arch/repo/x86_64`, generated with `repo-add`
- [x] Stable/testing channels by branch (`main` / `develop`)
- [x] Branding and features **persist after install** (packages, not sed patches)
- [x] Installed systems update via plain `pacman -Syu` or the in-OS notifier
- [x] Reproducible CI build inside a `cachyos-v3` container with CachyOS repos

### Desktop polish (in progress)
- [x] Klassy compiled/configured for rounded window corners (14px)
- [ ] Custom taskbar icon selection style — rounded pill highlight + hover animation
- [ ] Centered taskbar icons (Windows 11 style) — logo left, systray right
- [ ] Custom app launcher (Kickoff replacement) — glassmorphic popup with search,
      pinned grid, recent files, user profile, Genesi green accents

---

## PHASE 4: IDE & Dev Tools ⬜ PENDING
> Developer-focused tools and integrations (secondary differentiator).

### 4.1 Genesi IDE (based on VS Code or Zed)
- [ ] Fork of VS Code or Zed with Genesi branding
- [ ] Pre-installed Genesi theme
- [ ] Pre-configured extensions (Git, Docker, AI, popular languages)
- [ ] Native integration with the local AI daemon
- [ ] Integration with MemPalace (project context)
- [ ] Desktop and menu shortcut

### 4.2 Container widget in Plasma
- [ ] Taskbar widget showing running Docker containers
- [ ] Start/Stop/Restart with one click
- [ ] View container logs and mapped ports
- [ ] CPU/RAM usage per container

### 4.3 Project sandboxes (isolated workspaces)
- [ ] Based on Distrobox/Toolbox
- [ ] GUI to create/manage workspaces
- [ ] Templates: "Java + Spring Boot", "React + Vite", "Python + FastAPI", etc.
- [ ] Each workspace has its own isolated dependencies
- [ ] Integration with Genesi IDE

### 4.4 Network inspection
- [ ] mitmproxy pre-installed and configured
- [ ] Simple GUI to intercept HTTP/HTTPS requests
- [ ] Quick shortcut to enable/disable a debug proxy
- [ ] Integration with the container widget (per-container traffic)

### 4.5 Database explorer
- [ ] Beekeeper Studio or DBeaver pre-installed
- [ ] Dolphin plugin to connect to databases
- [ ] Support for PostgreSQL, MySQL, SQLite, MongoDB
- [ ] Quick table and data visualization

---

## PHASE 5: Polish & Distribution ⬜ PENDING
> Final polish and public release.

- [ ] Custom Calamares slideshow & imagery (branding package already in place)
- [ ] Official Genesi OS website
- [ ] Complete end-user documentation
- [ ] Download page with ISOs
- [ ] Community (Discord/Forum)
- [x] Automatic updates via the self-hosted repository *(delivered in Phase 3)*

### 5.1 Desktop Environment selector in the installer (Calamares)
> Like CachyOS's installer — let the user pick their DE at install time.

- [ ] Add a "Choose your desktop" step to Calamares (similar to the CachyOS
      `packagechooser` module with screenshots + descriptions)
- [ ] **Option 1: KDE Plasma 6 (default)** — current Genesi setup: Klassy 14px
      rounded windows, Darkly glassmorphism, Ant-Dark popups, Kickoff menu
- [ ] **Option 2: Hyprland + caelestia-shell** — Wayland tiling compositor with
      the [caelestia-dots/shell](https://github.com/caelestia-dots/shell) design
      (Quickshell QML widgets, no waybar). Pulls `hyprland`, `caelestia-shell`
      (AUR), `caelestia-cli`, `quickshell-git`, `ddcutil`, `brightnessctl` into a
      netinstall group that installs only when this option is picked
- [ ] (Future) Additional options: GNOME, COSMIC, Sway, etc.
- [ ] SDDM session entries auto-registered for whatever the user picked
- [ ] Wallpapers + branding consistent across all DE choices
- [ ] `genesi-x11-detect.sh` extended to handle the chosen DE (Hyprland needs
      different SDDM session forcing than Plasma)
- [ ] `genesi-welcome` detects the running DE and adjusts its buttons per-DE
- [ ] Doc page explaining the DE choice and when each one shines

---

## Build & Release Infrastructure

Genesi OS keeps **two strictly separate** pipelines so that fixing the live ISO
can never break updates for installed users, and vice-versa.

### 1. Package / Update pipeline — `publish-packages.yml`
- **Trigger:** pushes to `main`/`develop` touching `genesi-arch/packages/**` or
  any package submodule pointer (each package sources `HEAD` of its submodule).
- **Runner:** `cachyos/cachyos-v3` container (CachyOS repos + keyring trusted),
  required because several packages depend on CachyOS-only packages.
- **Flow:** collect all PKGBUILD deps → pre-install them → `makepkg` each package
  as an unprivileged `builder` user → `repo-add` → commit the repo to
  `genesi-arch/repo/x86_64` on the same branch.
- **Result:** installed systems receive updates via `pacman -Syu` / the in-OS
  notifier. `main` → stable, `develop` → testing (selectable with `genesi-channel`).

### 2. ISO pipeline — `iso-pipeline.yml`
- **Trigger:** pushes to `main` touching ISO inputs (`genesi-arch/**`, the
  Calamares config submodule, the workflow) and `v*` tags. Docs-only commits are
  skipped.
- **Job 1 — validate-install:** dependency dry-run + a real `pacstrap` into a
  throwaway root, reproducing the Calamares package set. A broken package set
  fails here, before any 30-minute build.
- **Job 2 — build-iso:** runs only if Job 1 passes; `mkarchiso` → `.iso`,
  uploaded as an artifact (and attached to a GitHub Release on `v*` tags).

### Build/install internals worth knowing
- **ISO build:** `genesi-arch/prepare-and-build.sh` → `buildiso.sh -p desktop` →
  `mkarchiso`. The scripts refuse to run as root, so CI uses a passwordless-sudo
  `builder` user.
- **Calamares config deploy:** `genesi-calamares-config-full/` (submodule) reaches
  the ISO via `customize_airootfs.sh` at build time and is re-copied at install
  launch by `calamares-online.sh`.
- **NVIDIA gotcha:** the netinstall "NVIDIA Drivers" group must use
  `nvidia-open-dkms` (Turing+), **not** `nvidia-dkms` — the only `nvidia-dkms`
  provider hard-pins a `nvidia-utils` version that is unsatisfiable whenever
  CachyOS lags Arch.

---

## Appendix: Running Local AI on Genesi OS

### Method 1 — Ollama (easiest)
```bash
curl -fsSL https://ollama.ai/install.sh | sh   # install
ollama pull llama3.2                            # 2GB, lightweight
ollama pull deepseek-coder                      # 1.3GB, good for code
ollama run llama3.2                             # run (AI Mode activates automatically)
curl http://localhost:11434/api/generate -d '{"model":"llama3.2","prompt":"Hello"}'
```

### Method 2 — llama.cpp (more control)
```bash
sudo pacman -S llama.cpp
wget https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/resolve/main/llama-2-7b-chat.Q4_K_M.gguf
llama-cli   -m llama-2-7b-chat.Q4_K_M.gguf -p "Hello, how are you?"
llama-server -m llama-2-7b-chat.Q4_K_M.gguf --port 8080   # OpenAI-compatible API
```

### Method 3 — LocalAI (OpenAI-compatible API)
```bash
docker run -p 8080:8080 localai/localai
```

### Benchmarking (for documentation)
```bash
ollama run llama3.2 "Explain quantum computing" --verbose   # tokens/sec
llama-bench -m model.gguf                                    # llama.cpp benchmark
watch -n 1 nvidia-smi   # GPU (if NVIDIA)
htop                    # CPU and RAM
```

Run the same prompt on Genesi OS (AI Mode ON) vs. a stock Ubuntu/Fedora and
compare tokens/second, RAM usage, model load time, and VRAM usage.

---

## About MemPalace

[MemPalace](https://github.com/MemPalace/mempalace) is a local-first AI memory
system that:
- Stores conversations verbatim (no summarizing/altering)
- Organizes into "wings" (projects), "rooms" (topics), "drawers" (content)
- Provides local semantic search (96.6% recall without an LLM, 98.4% with heuristics)
- Keeps everything on-device — nothing leaves the machine
- Exposes 29 MCP tools to integrate with any AI

Why it fits Genesi OS: persistent memory for local AI, automatic project-context
indexing, zero cloud, and IDE integration via MCP. License: MIT (compatible with
Genesi OS's GPL-3.0).
