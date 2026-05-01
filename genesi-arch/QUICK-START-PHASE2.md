# Quick Start - Testing Phase 2 (AI Mode)

## 🚀 Build the ISO

On your CachyOS VM:

```bash
cd genesi-arch
sudo rm -rf build/ out/
sudo ./buildiso.sh -p desktop 2>&1 | tee build.log
```

Wait ~10-15 minutes for the build to complete.

## 📀 Boot the ISO

1. Copy the ISO from `out/` to your host machine
2. Create a new VirtualBox VM:
   - **RAM**: 8GB minimum (16GB recommended)
   - **CPU**: 4+ cores
   - **Disk**: 30GB+
3. Boot from the ISO

## ✅ Verify AI Mode is Running

Once booted to the desktop:

```bash
# Check if genesi-aid service is running
sudo systemctl status genesi-aid

# Should show:
# ● genesi-aid.service - Genesi AI Daemon
#    Active: active (running)
```

## 🤖 Install and Test Ollama

```bash
# Install Ollama
curl -fsSL https://ollama.ai/install.sh | sh

# Download a lightweight model (2GB)
ollama pull llama3.2

# Open a second terminal to watch AI Mode activate
sudo journalctl -u genesi-aid -f

# In the first terminal, run the model
ollama run llama3.2
```

## 👀 What to Look For

### In the daemon logs (second terminal):
```
INFO - Detected AI processes: ['ollama']
INFO - Enabling AI Mode...
INFO - CPU governor set to: performance
INFO - Swappiness set to: 10
INFO - Transparent huge pages enabled
INFO - Prioritized process: ollama (PID 1234)
INFO - AI Mode enabled
```

### In the taskbar:
- Look for the **AI Mode widget** (should show "AI" with pulsing animation)
- Click it to see detected processes

### Verify optimizations:
```bash
# CPU governor (should be "performance")
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

# Swappiness (should be 10)
cat /proc/sys/vm/swappiness

# Huge pages (should be "always")
cat /sys/kernel/mm/transparent_hugepage/enabled
```

## 📊 Benchmark Performance

### Test 1: With AI Mode (default)
```bash
ollama run llama3.2 "Write a Python function to calculate fibonacci numbers up to n" --verbose
```

Note the **tokens/second** value at the end.

### Test 2: Without AI Mode
```bash
# Stop the daemon
sudo systemctl stop genesi-aid

# Run same prompt
ollama run llama3.2 "Write a Python function to calculate fibonacci numbers up to n" --verbose
```

Note the **tokens/second** value.

### Expected Result
AI Mode should be **15-25% faster** on CPU-only systems.

## 🎯 Test Auto-Disable

```bash
# Start daemon again
sudo systemctl start genesi-aid

# Watch logs
sudo journalctl -u genesi-aid -f

# Exit Ollama (type /bye or Ctrl+D)
```

You should see:
```
INFO - No AI processes detected
INFO - Disabling AI Mode...
INFO - CPU governor set to: powersave
INFO - Swappiness set to: 60
INFO - AI Mode disabled
```

## 🐛 Troubleshooting

### Daemon not running?
```bash
# Check logs
sudo journalctl -u genesi-aid -n 50

# Check if python-psutil is installed
pacman -Q python-psutil

# Manually run to see errors
sudo /usr/local/bin/genesi-aid
```

### AI Mode not activating?
```bash
# Check if Ollama is running
ps aux | grep ollama

# Check state file
cat /var/run/genesi-aid.state

# Restart daemon
sudo systemctl restart genesi-aid
```

### Widget not showing?
```bash
# Check widget files
ls -la /usr/share/plasma/plasmoids/org.genesi.aimode/

# Restart Plasma
kquitapp5 plasmashell && kstart5 plasmashell
```

## 📸 Screenshots to Take

For documentation:

1. **Desktop with AI Mode widget** (before running AI)
2. **Ollama running** with widget showing "AI Mode: ON"
3. **Widget expanded** showing detected processes
4. **Terminal with daemon logs** showing optimizations
5. **Performance comparison** (tokens/second with and without AI Mode)

## 📝 Document Your Results

Create a file with your findings:

```bash
# System info
uname -a
cat /proc/cpuinfo | grep "model name" | head -1
free -h

# Performance results
echo "With AI Mode: X tokens/second"
echo "Without AI Mode: Y tokens/second"
echo "Improvement: Z%"
```

## 🎉 Success Criteria

Phase 2 is successful if:

- ✅ `genesi-aid` service is running
- ✅ AI Mode activates when Ollama runs
- ✅ CPU governor switches to "performance"
- ✅ Swappiness changes to 10
- ✅ Huge pages enabled
- ✅ Widget shows "AI Mode: ON"
- ✅ Performance improvement of 10%+ on CPU-only
- ✅ AI Mode disables when Ollama stops

## 🚀 Next Steps After Testing

1. **Document performance** with real numbers
2. **Take screenshots** for README
3. **Create demo video** (optional)
4. **Share results** with the team
5. **Move to Phase 3** (IDE and Dev Tools) or **Phase 5** (Own Packages)

---

**Need help?** Check:
- `docs/PHASE2-AI-MODE.md` - Detailed testing guide
- `docs/AI-MODE-REFERENCE.md` - Quick reference
- `PHASE2-COMPLETE.md` - Implementation details

**Ready to build?** Run:
```bash
bash test-phase2.sh  # Verify files
sudo ./buildiso.sh -p desktop  # Build ISO
```
