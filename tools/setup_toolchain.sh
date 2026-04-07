#!/bin/bash
# tools/setup_toolchain.sh
# Sets up the x86_64-elf cross-compiler toolchain for Genesi OS
# Run once inside WSL2: bash tools/setup_toolchain.sh

set -e

BINUTILS_VER="2.41"
GCC_VER="13.2.0"
TARGET="x86_64-elf"
PREFIX="$HOME/opt/cross"
JOBS=$(nproc)
BUILDTMP="/tmp/genesi-toolchain"

export PATH="$PREFIX/bin:$PATH"

echo "========================================"
echo "  Genesi OS Toolchain Setup"
echo "  Target  : $TARGET"
echo "  Prefix  : $PREFIX"
echo "  Threads : $JOBS"
echo "========================================"
echo ""

# ---- System packages ----
echo "[1/5] Installing system packages..."
sudo apt-get update -qq
sudo apt-get install -y \
    build-essential bison flex \
    libgmp-dev libmpc-dev libmpfr-dev libisl-dev \
    texinfo wget \
    nasm \
    xorriso grub-pc-bin grub-common mtools \
    qemu-system-x86 \
    gdb \
    2>/dev/null

echo "      Done."

# ---- Download sources ----
mkdir -p "$BUILDTMP" && cd "$BUILDTMP"

echo "[2/5] Downloading sources (this may take a moment)..."

if [ ! -f "binutils-${BINUTILS_VER}.tar.xz" ]; then
    wget -q "https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VER}.tar.xz"
fi
if [ ! -f "gcc-${GCC_VER}.tar.xz" ]; then
    wget -q "https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VER}/gcc-${GCC_VER}.tar.xz"
fi

echo "      Done."

# ---- Extract ----
echo "[3/5] Extracting..."
[ -d "binutils-${BINUTILS_VER}" ] || tar xf "binutils-${BINUTILS_VER}.tar.xz"
[ -d "gcc-${GCC_VER}" ]           || tar xf "gcc-${GCC_VER}.tar.xz"
echo "      Done."

# ---- Build binutils ----
echo "[4/5] Building binutils (a few minutes)..."
mkdir -p build-binutils && cd build-binutils
"../binutils-${BINUTILS_VER}/configure" \
    --target="$TARGET" \
    --prefix="$PREFIX" \
    --with-sysroot \
    --disable-nls \
    --disable-werror \
    --quiet
make -j"$JOBS" -s
make install -s
cd "$BUILDTMP"
echo "      Done."

# ---- Build GCC ----
echo "[5/5] Building GCC (10-20 minutes, grab a coffee)..."
cd "gcc-${GCC_VER}"
./contrib/download_prerequisites -q 2>/dev/null || true
cd "$BUILDTMP"
mkdir -p build-gcc && cd build-gcc
"../gcc-${GCC_VER}/configure" \
    --target="$TARGET" \
    --prefix="$PREFIX" \
    --disable-nls \
    --enable-languages=c \
    --without-headers \
    --quiet
make -j"$JOBS" -s all-gcc
make -j"$JOBS" -s all-target-libgcc
make install-gcc -s
make install-target-libgcc -s
cd "$BUILDTMP"
echo "      Done."

echo ""
echo "========================================"
echo "  Toolchain installed successfully!"
echo "========================================"
echo ""
echo "  Add to your ~/.bashrc:"
echo "    export PATH=\"$PREFIX/bin:\$PATH\""
echo ""
echo "  Verify:"
echo "    ${TARGET}-gcc --version"
echo "    ${TARGET}-ld  --version"
echo ""
echo "  Then build Genesi:"
echo "    make iso && make run"
