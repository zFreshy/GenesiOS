# Genesi OS - show a branded fastfetch logo + system info when a terminal opens.
# Installed to /usr/share/fish/vendor_conf.d/, which fish auto-sources on every
# shell start. We guard on `status is-interactive` so it never runs in scripts,
# and on a session flag so split panes / subshells of the same terminal don't
# repeat it (fish exports the flag to children; each NEW terminal starts fresh).
if status is-interactive
    if not set -q GENESI_FASTFETCH_SHOWN
        set -gx GENESI_FASTFETCH_SHOWN 1
        if type -q fastfetch
            fastfetch --config /usr/share/genesi/fastfetch/genesi.jsonc
        end
    end
end
