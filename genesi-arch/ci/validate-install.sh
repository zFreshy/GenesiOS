#!/usr/bin/env bash
# Genesi OS - install validation (CI + local).
#
# Reproduces what the Calamares installer would do to the TARGET, WITHOUT
# building an ISO or booting a VM, so a broken package set fails here in
# minutes instead of during a real install. Run inside a CachyOS/Arch
# environment (the CI uses the cachyos/cachyos-v3 container).
#
# netinstall.yaml is a SELECTION UI: the user gets the selected:true groups by
# default and can opt into selected:false ones (nvidia, alternative kernels,
# ...). So we validate:
#
#   * DEFAULT set  = pacstrap base + every package in selected:true groups
#                    (and selected:true subgroups). This is what EVERY install
#                    pulls, so it is a HARD gate:
#       Level 1 - `pacman -Sp` dependency dry-run (fast)
#       Level 2 - real `pacstrap` into a throwaway root + install the set the
#                 same way packages@online does (catches file/scriptlet errors)
#
#   * Each OPT-IN group (selected:false) is dry-run INDIVIDUALLY (base+group),
#     never all together (they are mutually exclusive - e.g. one kernel).
#       - a real dependency CONFLICT (like the nvidia-open-dkms 595-vs-610
#         abort) => FAIL the build
#       - merely missing/removed packages (a stale netinstall entry, e.g. an
#         old kernel pruned from the repos) => WARN only, don't block the ISO
#
# Exit non-zero if any HARD check fails. Set SKIP_LEVEL2=1 for Level 1 only.
set -uo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
CALA="$REPO_ROOT/genesi-calamares-config-full/etc/calamares/modules"
PACSTRAP_CONF="$CALA/pacstrap.conf"
NETINSTALL="$CALA/netinstall.yaml"
LIVE_PKGS="$REPO_ROOT/genesi-arch/archiso/packages_desktop.x86_64"
LOCALE_SUB="${LOCALE_SUB:-en-us}"   # expands firefox-i18n-$LOCALE etc.

FAIL=0
note() { printf '\n\033[1;34m==> %s\033[0m\n' "$*"; }
ok()   { printf '\033[1;32m  ✓ %s\033[0m\n' "$*"; }
warn() { printf '\033[1;33m  ! %s\033[0m\n' "$*"; }
bad()  { printf '\033[1;31m  ✗ %s\033[0m\n' "$*"; FAIL=1; }

# ---------------------------------------------------------------------------
# Same repos the installed target sees: [genesi] + CachyOS (already present) +
# core/extra + multilib (for lib32-*).
# ---------------------------------------------------------------------------
note "Configuring repositories ([genesi] + multilib)"
grep -q '^\[genesi\]' /etc/pacman.conf || cat >> /etc/pacman.conf <<'EOF'

[genesi]
SigLevel = Optional TrustAll
Server = https://raw.githubusercontent.com/zFreshy/GenesiOS/main/genesi-arch/repo/x86_64
EOF
grep -q '^\[multilib\]' /etc/pacman.conf || cat >> /etc/pacman.conf <<'EOF'

[multilib]
Include = /etc/pacman.d/mirrorlist
EOF

note "Refreshing package databases (pacman -Sy)"
pacman -Sy --noconfirm >/dev/null || { bad "pacman -Sy failed"; exit 1; }

# ---------------------------------------------------------------------------
# Parse package sets (selection-aware). Writes:
#   /tmp/_base.txt     pacstrap basePackages
#   /tmp/_default.txt  netinstall packages from selected:true nodes
#   /tmp/_optin.tsv    one opt-in (selected:false) group per line: "name\tpkgs"
# ---------------------------------------------------------------------------
note "Parsing package lists (selection-aware)"
python3 - "$PACSTRAP_CONF" "$NETINSTALL" "$LOCALE_SUB" \
         /tmp/_base.txt /tmp/_default.txt /tmp/_optin.tsv <<'PY'
import sys, yaml
pacstrap_conf, netinstall, locale, f_base, f_def, f_opt = sys.argv[1:7]

def clean(p):
    p = str(p).replace("$LOCALE", locale)
    return None if "$" in p else p   # drop any other unexpanded placeholder

def pkgs_of(node):
    out = []
    for p in node.get("packages", []) or []:
        c = clean(p)
        if c:
            out.append(c)
    return out

# base
base = yaml.safe_load(open(pacstrap_conf))
with open(f_base, "w") as fh:
    for p in base.get("basePackages", []) or []:
        c = clean(p)
        if c:
            fh.write(c + "\n")

net = yaml.safe_load(open(netinstall)) or []

def selected(node):           # default-checked unless explicitly false
    return node.get("selected", True) is not False

default_pkgs, optin = [], []   # optin: list of (name, [pkgs...]) for whole subtree

def collect_default(node):     # packages under an active (selected) subtree
    out = list(pkgs_of(node))
    for sg in node.get("subgroups", []) or []:
        if selected(sg):
            out += collect_default(sg)
    return out

def all_pkgs(node):            # every package in a subtree, ignoring selection
    out = list(pkgs_of(node))
    for sg in node.get("subgroups", []) or []:
        out += all_pkgs(sg)
    return out

for grp in net:
    name = grp.get("name", "?")
    if selected(grp):
        default_pkgs += collect_default(grp)
        # selected:false subgroups inside a selected group are opt-in too
        for sg in grp.get("subgroups", []) or []:
            if not selected(sg):
                optin.append((f"{name} / {sg.get('name','?')}", all_pkgs(sg)))
    else:
        optin.append((name, all_pkgs(grp)))

# dedupe default, keep order
seen = set(); ded = []
for p in default_pkgs:
    if p not in seen:
        seen.add(p); ded.append(p)

with open(f_def, "w") as fh:
    fh.write("\n".join(ded) + "\n")
with open(f_opt, "w") as fh:
    for name, ps in optin:
        ps = sorted(set(ps))
        if ps:
            fh.write(name + "\t" + " ".join(ps) + "\n")
PY

mapfile -t BASE    < /tmp/_base.txt
mapfile -t DEFAULT < /tmp/_default.txt
mapfile -t LIVE    < <(grep -vE '^\s*(#|$)' "$LIVE_PKGS" | tr -d ' \t\r')
ok "base packages:            ${#BASE[@]}"
ok "default netinstall pkgs:  ${#DEFAULT[@]}"
ok "live ISO packages:        ${#LIVE[@]}"
ok "opt-in groups:            $(wc -l < /tmp/_optin.tsv)"
[ "${#BASE[@]}" -gt 0 ] && [ "${#DEFAULT[@]}" -gt 0 ] || { bad "empty set - parse failed"; exit 1; }

# ---------------------------------------------------------------------------
# LEVEL 1 - dependency dry-run of the HARD (always-installed) sets.
# ---------------------------------------------------------------------------
dryrun_hard() { # <label> <pkgs...>
  local label="$1"; shift
  if pacman -Sp --needed --noconfirm "$@" >/dev/null 2>/tmp/_err </dev/null; then
    ok "Level 1: $label resolves"
  else
    bad "Level 1: $label FAILED to resolve"
    grep -iE 'error|unable to satisfy|cannot resolve|target not found|conflict' /tmp/_err \
      | sed 's/^/      /' | head -20
  fi
}

note "LEVEL 1 - dependency dry-run (hard gate)"
dryrun_hard "live ISO airootfs"        "${LIVE[@]}"
dryrun_hard "target base (pacstrap)"   "${BASE[@]}"
dryrun_hard "base + default netinstall" "${BASE[@]}" "${DEFAULT[@]}"

# ---------------------------------------------------------------------------
# Opt-in groups: dry-run each on top of base. Distinguish a real dependency
# conflict (fail) from merely-missing packages (warn).
# ---------------------------------------------------------------------------
note "Opt-in groups - dependency dry-run (conflict=fail, missing=warn)"
while IFS=$'\t' read -r gname gpkgs; do
  [ -n "$gname" ] || continue
  # shellcheck disable=SC2086
  if pacman -Sp --needed --noconfirm "${BASE[@]}" $gpkgs >/dev/null 2>/tmp/_err </dev/null; then
    ok "opt-in: $gname resolves"
  elif grep -qiE 'unable to satisfy|could not satisfy|in conflict|cannot resolve.*dependency' /tmp/_err; then
    bad "opt-in: $gname has a DEPENDENCY CONFLICT (install-breaker)"
    grep -iE 'unable to satisfy|could not satisfy|conflict|cannot resolve' /tmp/_err \
      | sed 's/^/      /' | head -10
  else
    warn "opt-in: $gname has missing/removed packages (stale netinstall entry, not blocking)"
    grep -iE 'target not found' /tmp/_err | sed 's/^/      /' | head -10
  fi
done < /tmp/_optin.tsv

# ---------------------------------------------------------------------------
# LEVEL 2 - real install of the DEFAULT path into a throwaway root.
# ---------------------------------------------------------------------------
if [ "${SKIP_LEVEL2:-0}" = "1" ]; then
  note "LEVEL 2 skipped (SKIP_LEVEL2=1)"
else
  note "LEVEL 2 - real pacstrap base + install default netinstall set"
  ROOT="$(mktemp -d /tmp/genesi-root.XXXXXX)"
  trap 'rm -rf "$ROOT"' EXIT
  if pacstrap -c "$ROOT" "${BASE[@]}" </dev/null; then
    ok "Level 2: pacstrap base OK"
    pacman --root "$ROOT" -Sy --noconfirm >/dev/null 2>&1 </dev/null || true
    if pacman --root "$ROOT" -S --noconfirm --needed --overwrite='*' \
              --disable-download-timeout "${DEFAULT[@]}" </dev/null; then
      ok "Level 2: default netinstall set installed OK"
    else
      bad "Level 2: default netinstall install FAILED (file conflict / scriptlet / dep)"
    fi
  else
    bad "Level 2: pacstrap base FAILED"
  fi
fi

# ---------------------------------------------------------------------------
note "RESULT"
if [ "$FAIL" -eq 0 ]; then
  ok "All hard install validations passed - safe to build the ISO"
  exit 0
else
  bad "Install validation FAILED - NOT safe to build the ISO"
  exit 1
fi
