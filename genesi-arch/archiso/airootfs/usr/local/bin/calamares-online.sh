#!/bin/bash

main() {
    # Remove current keyring first, to complete initiate it
    sudo rm -rf /etc/pacman.d/gnupg
    # We are using this, because archlinux is signing the keyring often with a newly created keyring
    # This results into a failed installation for the user.
    # Installing archlinux-keyring fails due not being correctly signed
    # Mitigate this by installing the latest archlinux-keyring on the ISO, before starting the installation
    # The issue could also happen, when the installation does rank the mirrors and then a "faulty" mirror gets used
    sudo pacman -Sy --noconfirm archlinux-keyring cachyos-keyring
    # Also populate the keys, before starting the Installer, to avoid above issue
    sudo pacman-key --init
    sudo pacman-key --populate archlinux cachyos
    # Also use timedatectl to sync the time with the hardware clock
    # There has been a bunch of reports, that the keyring was created in the future
    # Syncing appears to fix it
    timedatectl set-ntp true

    local progname="$(basename "$0")"
    local log="/home/liveuser/cachy-install.log"
    local mode="online"  # TODO: keep this line for now

    local SYSTEM=""

    if [ -d /sys/firmware/efi ]; then
        SYSTEM="UEFI SYSTEM"
    else
        SYSTEM="BIOS/MBR SYSTEM"
    fi

    local ISO_VERSION="$(cat /etc/version-tag)"
    echo "USING ISO VERSION: ${ISO_VERSION}"

    sudo pacman -Sy --noconfirm cachyos-calamares-next

    # Rebrand Calamares from CachyOS to Genesi OS
    if [ -d /usr/share/calamares/branding/cachyos ]; then
        # Copy cachyos branding to genesi folder, then patch
        sudo mkdir -p /usr/share/calamares/branding/genesi
        sudo cp -rf /usr/share/calamares/branding/cachyos/* /usr/share/calamares/branding/genesi/

        # Copy Genesi OS custom images over CachyOS ones (if they exist in our branding dir)
        for img in logo.png icon.png welcome.png; do
            if [ -f /usr/share/calamares/branding/genesi/$img ]; then
                : # Already there from airootfs
            fi
        done

        sudo sed -i \
            -e 's/productName:.*CachyOS/productName:       Genesi OS/' \
            -e 's/shortProductName:.*CachyOS/shortProductName:  Genesi OS/' \
            -e 's/versionedName:.*CachyOS/versionedName:     Genesi OS/' \
            -e 's/shortVersionedName:.*CachyOS/shortVersionedName: Genesi OS/' \
            -e 's/bootLoaderEntryName:.*CachyOS/bootLoaderEntryName: Genesi OS/' \
            -e 's/componentName:.*cachyos/componentName:     genesi/' \
            -e 's|https://cachyos.org|https://github.com/zFreshy/GenesiOS|g' \
            /usr/share/calamares/branding/genesi/branding.desc
    fi
    # Also copy to /etc path
    sudo mkdir -p /etc/calamares/branding/genesi
    if [ -f /usr/share/calamares/branding/genesi/branding.desc ]; then
        sudo cp -rf /usr/share/calamares/branding/genesi/* /etc/calamares/branding/genesi/
    fi
    # Update settings to use genesi branding
    sudo find /usr/share/calamares /etc/calamares -type f -name "settings*.conf" -exec sed -i \
        -e 's/branding:.*cachyos/branding: genesi/' \
        -e 's/CachyOS/Genesi OS/g' \
        {} + 2>/dev/null || true
    # Patch all Calamares text files
    sudo find /usr/share/calamares /etc/calamares -type f \( -name "*.conf" -o -name "*.qml" -o -name "*.yml" -o -name "*.yaml" -o -name "*.desc" \) \
        -exec sed -i -e 's/CachyOS/Genesi OS/g' {} + 2>/dev/null || true

    # Get Hardware Informations
    inxi -F > "$log"

    cat <<EOF >> "$log"
########## $log by $progname
########## Started (UTC): $(date -u "+%x %X")
########## ISO version: $ISO_VERSION
########## System: $SYSTEM
EOF

    sudo cp "/usr/share/calamares/settings_${mode}.conf" /etc/calamares/settings.conf
    exec pkexec-wrapper calamares -D6 >> $log
}

main "$@"
