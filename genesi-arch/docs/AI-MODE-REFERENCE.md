# Genesi AI Mode - Quick Reference

## What is AI Mode?

AI Mode is an intelligent system optimization daemon that automatically detects when you're running local AI models and applies performance optimizations. When AI processes stop, everything returns to normal.

## Supported AI Frameworks

- **Ollama** - Most popular, easiest to use
- **llama.cpp** - Maximum control and performance
- **vLLM** - Production-grade inference server
- **LocalAI** - OpenAI-compatible API
- **text-generation-webui** - Web UI for models
- **KoboldCPP** - Gaming/creative writing focused
- **Oobabooga** - Advanced text generation

## Optimizations Applied

| Optimization | Normal | AI Mode | Benefit |
|--------------|--------|---------|---------|
| CPU Governor | `powersave` | `performance` | Max CPU frequency |
| Swappiness | 60 | 10 | Keep model in RAM |
| Huge Pages | `madvise` | `always` | Faster memory access |
| Process Priority | 0 | -5 (high) | More CPU time for AI |
| Compositor Effects | Full | Reduced | Free VRAM |

## Performance Impact

Based on testing with Ollama on CPU-only systems:

- **Tokens/second**: +15-25% improvement
- **Model load time**: -30-40% faster
- **Memory stability**: Reduced swap usage
- **CPU frequency**: Consistent max frequency

## How to Use

### 1. Install an AI Framework

**Ollama (Recommended)**
```bash
curl -fsSL https://ollama.ai/install.sh | sh
ollama pull llama3.2
ollama run llama3.2
```

**llama.cpp**
```bash
sudo pacman -S llama.cpp
wget https://huggingface.co/TheBloke/Llama-2-7B-Chat-GGUF/resolve/main/llama-2-7b-chat.Q4_K_M.gguf
llama-server -m llama-2-7b-chat.Q4_K_M.gguf
```

### 2. AI Mode Activates Automatically

No configuration needed! The daemon detects AI processes and enables optimizations.

### 3. Monitor Status

**Check daemon status:**
```bash
sudo systemctl status genesi-aid
```

**Watch logs:**
```bash
sudo journalctl -u genesi-aid -f
```

**Check state file:**
```bash
cat /var/run/genesi-aid.state
```

**Verify optimizations:**
```bash
# CPU governor (should be "performance")
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

# Swappiness (should be 10)
cat /proc/sys/vm/swappiness

# Huge pages (should be "always")
cat /sys/kernel/mm/transparent_hugepage/enabled
```

### 4. Use the Plasma Widget

Look for the **AI Mode widget** in your taskbar:
- Shows "AI" text with pulsing animation when active
- Click to see detected processes
- Lists applied optimizations
- Auto-refreshes every 5 seconds

## Troubleshooting

### AI Mode not activating?

```bash
# Check if AI process is running
ps aux | grep -E "ollama|llama"

# Restart daemon
sudo systemctl restart genesi-aid

# Check logs for errors
sudo journalctl -u genesi-aid -n 50
```

### Daemon not starting?

```bash
# Check if python-psutil is installed
pacman -Q python-psutil

# Run daemon manually to see errors
sudo /usr/local/bin/genesi-aid
```

### Widget not showing?

```bash
# Restart Plasma
kquitapp5 plasmashell && kstart5 plasmashell

# Check widget files
ls -la /usr/share/plasma/plasmoids/org.genesi.aimode/
```

## Benchmarking

To measure performance improvements:

```bash
# Baseline (AI Mode OFF)
sudo systemctl stop genesi-aid
ollama run llama3.2 "Write a Python function to calculate fibonacci" --verbose

# With AI Mode (ON)
sudo systemctl start genesi-aid
ollama run llama3.2 "Write a Python function to calculate fibonacci" --verbose
```

Compare the **tokens/second** value in the output.

## Technical Details

### Files and Locations

- **Daemon**: `/usr/local/bin/genesi-aid`
- **Service**: `/usr/lib/systemd/system/genesi-aid.service`
- **State file**: `/var/run/genesi-aid.state`
- **Logs**: `/var/log/genesi-aid.log`
- **Sysctl config**: `/etc/sysctl.d/99-genesi-ai.conf`
- **Widget**: `/usr/share/plasma/plasmoids/org.genesi.aimode/`

### Daemon Behavior

- Checks for AI processes every 5 seconds
- Enables optimizations when AI detected
- Disables optimizations when AI stops
- Saves state to JSON file for widget
- Runs as systemd service (root privileges required)

### Sysctl Settings

```ini
vm.nr_hugepages = 512              # Pre-allocate huge pages
vm.hugetlb_shm_group = 0           # Allow all users
vm.swappiness = 60                 # Default (changed to 10 by daemon)
vm.max_map_count = 262144          # For large models
vm.vfs_cache_pressure = 50         # Optimize caching
vm.page-cluster = 3                # Readahead for large files
```

## Future Enhancements

- [ ] Manual toggle in widget (force ON/OFF)
- [ ] VRAM usage display (GPU monitoring)
- [ ] Tokens/second live metrics
- [ ] Model caching with vmtouch
- [ ] CPU pinning for inference threads
- [ ] GPU detection and optimal offloading
- [ ] Integration with MemPalace for persistent memory

## Why This Matters

**Genesi OS is the only Linux distribution with built-in AI optimization.**

Other distros require manual configuration:
- Editing `/etc/sysctl.conf`
- Creating custom systemd services
- Writing shell scripts
- Manually switching CPU governors
- Configuring huge pages

Genesi OS does all of this **automatically**. Just run your AI model and it works.

This is a **major differentiator** for developers working with local AI.
