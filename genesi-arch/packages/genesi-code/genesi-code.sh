#!/bin/sh
# Genesi Code launcher.
#
# Inside a VM the GPU presentation path is unreliable: VirtualBox's SVGA3D GL
# adapter creates a window but renders it invisibly, and lavapipe Vulkan fails
# surface creation. Forcing software GL (llvmpipe) renders correctly there. On
# bare metal we leave the environment alone so the real GPU (Vulkan/GL) is used.
#
# Override: if the user already set WGPU_BACKEND or LIBGL_ALWAYS_SOFTWARE, we
# respect it and don't touch anything.
if [ -z "${WGPU_BACKEND:-}" ] && [ -z "${LIBGL_ALWAYS_SOFTWARE:-}" ]; then
    if command -v systemd-detect-virt >/dev/null 2>&1 && systemd-detect-virt --quiet; then
        export LIBGL_ALWAYS_SOFTWARE=1
        export WGPU_BACKEND=gl
    fi
fi

exec /usr/lib/genesi-code/genesi-code "$@"
