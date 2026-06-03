# genesi-ai-mode

Automatic, **reversible** AI optimization daemon + Plasma widget for Genesi OS.

When a local-AI inference process is detected, the system is tuned for it; when
the workload stops (and there's no manual override) every tweak is restored.

## Components

- `genesi-aid` — Python daemon that detects AI processes and applies tuning
- `genesi-ai-mode` — CLI to force/inspect the mode (`on`/`off`/`auto`/`toggle`/`status`)
- `genesi-aid.service` — systemd service (root)
- `genesi-ai-mode.tmpfiles.conf` — creates `/run/genesi-ai-mode` for IPC
- `99-genesi-ai.conf` — static sysctl tuning (no permanent resource reservation)
- `plasmoid-aimode/` — Plasma 6 widget: live dashboard + on/auto/off
- `monitor/` — **AI Mode Monitor**, a standalone Qt6/Kirigami dashboard app
  (`genesi-ai-monitor`, needs `pyside6`)
- `genesi-ai-kwin-helper` — user-session helper that trims KWin desktop effects
  (blur/contrast) while AI Mode is on (KDE autostart, needs `qdbus`)
- `genesi-ai-turbo` — **speculative decoding** (opt-in): a small same-family draft
  model proposes tokens, the big model verifies several at once → 1.5–3× faster
  generation, identical output. Drives llama.cpp's `llama-server` (ollama doesn't
  expose this), reusing the GGUFs ollama already pulled. Needs `llama.cpp-cuda`.

  ```bash
  genesi-ai-turbo bench llama3.1:8b     # prove the speedup (spec vs no-spec)
  genesi-ai-turbo serve llama3.1:8b     # Turbo server on :11435 (OpenAI API)
  ```

## Detected frameworks

Ollama, llama.cpp (`llama-server`/`llama-cli`), vLLM, LocalAI,
text-generation-webui, KoboldCPP, Oobabooga.

## Optimizations (only while AI Mode is ON)

| Knob | AI Mode | Restored on exit |
|------|---------|------------------|
| CPU governor | `performance` | yes (to captured original) |
| CPU EPP (amd-pstate/intel_pstate) | `performance` (AC/forced) | yes |
| NVIDIA GPU | persistence + max power limit (AC/forced) | yes |
| AMD GPU | `power_dpm_force_performance_level=high` (AC/forced) | yes |
| Intel GPU | GT freq cap lifted to hardware max (AC/forced) | yes |
| `vm.swappiness` | 10 | yes |
| Transparent huge pages | `madvise` (not `always` — avoids khugepaged stalls) | yes |
| AI process priority | `nice -5` | reset to 0 |
| Model weights | preloaded into page cache (RAM) | n/a (pure cache warming) |
| Model-disk I/O scheduler | `none`/`mq-deadline` (AC/forced only) | yes (to captured original) |

Model preload and the I/O-scheduler switch target the actual weight files the
running engine has mmapped (detected via `/proc/<pid>/maps`). Preload is skipped
when the weights already live on a RAM-backed fs (e.g. a live ISO) or when free
RAM is tight; the I/O knob is system-wide so it only engages on AC or when
forced.

### Always-on defaults (not part of the on/off cycle)

- **Ollama concurrency** — at startup `genesi-aid` picks `OLLAMA_NUM_PARALLEL`
  and `OLLAMA_MAX_LOADED_MODELS` from this machine's RAM/VRAM and writes them to
  `/run/genesi-ai-mode/ollama.env`, which the Ollama unit reads via
  `EnvironmentFile=-`. Sane defaults, same spirit as the flash-attention /
  KV-cache drop-in.

### Pausing extra background apps (opt-in)

Beyond the built-in indexers (`baloo`, `tracker`), list extra process names —
one per line, `#` comments allowed — in `/etc/genesi-ai-mode/hogs.conf` and they
are `SIGSTOP`ped while AI Mode is on and `SIGCONT`ed when it ends. The file is
read live, so edits take effect on the next AI Mode transition (no restart).

CPU affinity pinning was intentionally removed — CPU inference scales with all
cores, so limiting affinity hurt the main use case.

## Manual control

```bash
genesi-ai-mode on       # force ON (even with no AI running)
genesi-ai-mode off      # force OFF (even while AI runs)
genesi-ai-mode auto     # follow automatic detection (default)
genesi-ai-mode toggle   # flip forced-ON / automatic
genesi-ai-mode status   # print daemon state (JSON)
```

The override is a file in the daemon's world-writable runtime dir, so no sudo is
needed. The panel launcher and the widget button use the same mechanism.

## How it talks to the UI

| Path | Writer | Readers |
|------|--------|---------|
| `/run/genesi-ai-mode/force` | user / CLI / panel / widget | daemon |
| `/run/genesi-ai-mode/state.json` | daemon | widget, `genesi-ai-mode status` |
| `/run/genesi-aid-originals.json` | daemon (0600) | daemon (restart recovery) |

`/run` is tmpfs: a daemon restart keeps the saved originals (so restore is
correct), but a reboot starts clean. The service keeps `PrivateTmp=true`
hardening because none of this uses `/tmp`.

## Build

```bash
makepkg -f
```

## Usage

```bash
sudo systemctl status genesi-aid     # service state
sudo journalctl -u genesi-aid -f     # logs (journal only, no unbounded file)
genesi-ai-mode status                # current state as JSON
```
