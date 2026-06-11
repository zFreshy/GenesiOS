#!/bin/sh
# Genesi Code launcher.
#
# Inside a VM the GPU presentation path is unreliable: VirtualBox's SVGA3D GL
# adapter creates a window but renders it invisibly, and lavapipe Vulkan fails
# surface creation. Forcing software GL (llvmpipe) renders correctly there. On
# bare metal we leave the environment alone so the real GPU (Vulkan/GL) is used.
#
# Override: if the user already set WGPU_BACKEND or LIBGL_ALWAYS_SOFTWARE, we
# respect it and don't touch anything. Use an absolute path for
# systemd-detect-virt so detection works even under a launcher's reduced PATH.
if [ -z "${WGPU_BACKEND:-}" ] && [ -z "${LIBGL_ALWAYS_SOFTWARE:-}" ]; then
    if /usr/bin/systemd-detect-virt --quiet 2>/dev/null; then
        export LIBGL_ALWAYS_SOFTWARE=1
        export WGPU_BACKEND=gl
    fi
fi

# Launch DETACHED (new session, reparented to init) with `setsid -f`. When
# started from the KDE menu / KRunner / a plasmoid, the app is otherwise a child
# of the launcher and gets killed the instant the launcher closes — the window
# would load and immediately disappear (it works fine from a terminal because
# the terminal stays alive). This is the same fix genesi-ai-mode uses for its
# plasmoid. Running from a terminal is unaffected.
exec setsid -f /usr/lib/genesi-code/genesi-code "$@"
