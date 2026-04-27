#!/bin/bash
#
# Genesi OS Arch Edition - Build Script
# Based on CachyOS buildiso.sh
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORK_DIR="${SCRIPT_DIR}/work"
OUT_DIR="${SCRIPT_DIR}/out"
PROFILE_DIR="${SCRIPT_DIR}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Options
CLEAN_FIRST=true
VERBOSE=false
BUILD_IN_RAM=false
REMOVE_BUILD_DIR=false

print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

usage() {
    echo "Usage: ${0##*/} [options]"
    echo '    -c                 Disable clean work dir'
    echo '    -r                 Enable building in RAM on systems with more than 23GB RAM'
    echo '    -w                 Remove build directory (not the ISO) after ISO file is built'
    echo '    -v                 Verbose output'
    echo '    -h                 This help'
    echo ''
    exit $1
}

# Parse arguments
while getopts "crvwh" opt; do
    case "${opt}" in
        c) CLEAN_FIRST=false ;;
        r) BUILD_IN_RAM=true ;;
        v) VERBOSE=true ;;
        w) REMOVE_BUILD_DIR=true ;;
        h) usage 0 ;;
        *) usage 1 ;;
    esac
done

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    print_error "This script must be run as root (use sudo)"
    exit 1
fi

# Check dependencies
print_info "Checking dependencies..."
DEPS=("archiso" "mkinitcpio-archiso" "git" "squashfs-tools" "grub")
MISSING_DEPS=()

for dep in "${DEPS[@]}"; do
    if ! pacman -Qi "$dep" &> /dev/null; then
        MISSING_DEPS+=("$dep")
    fi
done

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    print_error "Missing dependencies: ${MISSING_DEPS[*]}"
    print_info "Install with: sudo pacman -S ${MISSING_DEPS[*]}"
    exit 1
fi

print_success "All dependencies are installed"

# Build in RAM if enabled and enough RAM available
if [[ "$BUILD_IN_RAM" == "true" ]] && [[ $(grep MemTotal /proc/meminfo | awk '{print int($2/1024/1024)}') -gt 23 ]]; then
    print_info "Building in RAM (system has >23GB RAM)"
    WORK_DIR="$(mktemp -d --suffix="-genesi-iso")"
fi

# Clean work directory if enabled
if [[ "$CLEAN_FIRST" == "true" ]] && [[ -d "${WORK_DIR}" ]]; then
    print_info "Cleaning previous build..."
    rm -rf "${WORK_DIR}"
fi

# Create directories
mkdir -p "${OUT_DIR}"
mkdir -p "${WORK_DIR}"

# Verify profiledef.sh exists
if [ ! -f "${PROFILE_DIR}/profiledef.sh" ]; then
    print_error "profiledef.sh not found in ${PROFILE_DIR}"
    exit 1
fi

# Build the ISO
print_info "Starting ISO build..."
print_info "Profile: ${PROFILE_DIR}"
print_info "Work dir: ${WORK_DIR}"
print_info "Output dir: ${OUT_DIR}"
echo ""

if [[ "$VERBOSE" == "true" ]]; then
    mkarchiso -v -w "${WORK_DIR}" -o "${OUT_DIR}" "${PROFILE_DIR}"
else
    mkarchiso -w "${WORK_DIR}" -o "${OUT_DIR}" "${PROFILE_DIR}"
fi

BUILD_STATUS=$?

if [ $BUILD_STATUS -eq 0 ]; then
    echo ""
    print_success "ISO built successfully!"
    print_info "ISO location: ${OUT_DIR}"
    ls -lh "${OUT_DIR}"/*.iso 2>/dev/null || print_warning "No ISO file found in output directory"
else
    print_error "Failed to build ISO (exit code: $BUILD_STATUS)"
    exit 1
fi

# Remove work directory if requested
if [[ "$REMOVE_BUILD_DIR" == "true" ]]; then
    print_info "Removing work directory..."
    rm -rf "${WORK_DIR}"
    print_success "Work directory removed"
fi

echo ""
print_success "Build complete!"
print_info "You can now test the ISO in VirtualBox or write it to a USB drive"
