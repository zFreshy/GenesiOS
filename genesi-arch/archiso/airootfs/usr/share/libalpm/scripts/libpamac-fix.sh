#!/bin/bash
# Fix libpamac install script to prevent arithmetic syntax error
# This script patches the .INSTALL file before it runs

INSTALL_FILE="$1"

if [[ -f "$INSTALL_FILE" ]] && grep -q "libpamac" "$INSTALL_FILE" 2>/dev/null; then
    echo "Patching libpamac install script..."
    # Replace the buggy line with a safe version
    sed -i 's/((.*< 0.*/true # Patched by Genesi OS/' "$INSTALL_FILE"
fi
