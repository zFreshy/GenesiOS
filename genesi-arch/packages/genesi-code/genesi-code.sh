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
# of the launcher and gets killed the instant the launcher closes.
#
# CRUCIAL second half: also redirect stdio. From the menu the app's stdout/
# stderr are a pipe owned by the launcher's (short-lived) transient scope; once
# that scope exits the pipe's read end is gone, and the detached GUI app is
# killed by SIGPIPE / EIO on its very next write to stdout/stderr. That is the
# exact "opens from a terminal but NOT from the menu" symptom — a terminal keeps
# those fds alive, a menu launcher does not. Point stdin at /dev/null and stdout/
# stderr at a per-user log file so nothing can ever hang up on the app, and we
# also get a crash log if it still fails to show. Running from a terminal is
# unaffected (the redirect just sends its logs to the file too).
LOGDIR="${XDG_CACHE_HOME:-$HOME/.cache}/genesi-code"
mkdir -p "$LOGDIR" 2>/dev/null || true
exec setsid -f /usr/lib/genesi-code/genesi-code "$@" \
    </dev/null >>"$LOGDIR/genesi-code.log" 2>&1
