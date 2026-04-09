#!/bin/bash
set -e

ICONS_DIR="public/icons"
OUT_DIR="kernel/gui/icons"

mkdir -p $OUT_DIR

# 32x32 para a barra de tarefas
for icon in icon_grid4 icon_grid9 icon_search icon_folder icon_cmd icon_power; do
    echo "Converting $icon to 32x32..."
    rsvg-convert -w 32 -h 32 $ICONS_DIR/$icon.svg -o /tmp/$icon.png
    python3 img2c.py /tmp/$icon.png $OUT_DIR/$icon.h $icon
done

# 64x64 para o File Explorer
for icon in icon_doc icon_image icon_video icon_add; do
    echo "Converting $icon to 64x64..."
    rsvg-convert -w 64 -h 64 $ICONS_DIR/$icon.svg -o /tmp/$icon.png
    python3 img2c.py /tmp/$icon.png $OUT_DIR/$icon.h $icon
done

echo "All icons converted."