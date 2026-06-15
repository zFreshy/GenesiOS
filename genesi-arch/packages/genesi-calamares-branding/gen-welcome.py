#!/usr/bin/env python3
"""Generate the Genesi OS Calamares welcome.png from the canonical leaf logo.

Re-run to regenerate the branding asset. Matches the slideshow aesthetic:
dark-green gradient, mint glow, leaf + "Genesi OS" wordmark + tagline.
"""
import sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter

LEAF = sys.argv[1] if len(sys.argv) > 1 else r"D:\Desenvolvimento\Genesi\wallpapers\logo\GenesiOSLogoNoBg.png"
OUT = sys.argv[2] if len(sys.argv) > 2 else r"D:\Desenvolvimento\Genesi\genesi-calamares-config-full\etc\calamares\branding\genesi\welcome.png"

W, H = 760, 300

# --- brand palette ---
TOP = (7, 20, 15)        # #07140F
BOTTOM = (10, 30, 26)    # #0A1E1A
MINT = (0, 255, 159)     # #00ff9f accent
WORD = (234, 251, 243)   # near-white
TAG = (90, 214, 170)     # softened mint for the tagline

# --- vertical gradient background ---
bg = Image.new("RGB", (W, H))
px = bg.load()
for y in range(H):
    t = y / (H - 1)
    px_row = tuple(int(TOP[i] + (BOTTOM[i] - TOP[i]) * t) for i in range(3))
    for x in range(W):
        px[x, y] = px_row
img = bg.convert("RGBA")

# --- soft mint glow behind the leaf ---
glow = Image.new("RGBA", (W, H), (0, 0, 0, 0))
gd = ImageDraw.Draw(glow)
cx, cy = 150, H // 2
gd.ellipse([cx - 150, cy - 150, cx + 150, cy + 150], fill=(0, 255, 159, 70))
glow = glow.filter(ImageFilter.GaussianBlur(60))
img = Image.alpha_composite(img, glow)

# --- leaf logo ---
leaf = Image.open(LEAF).convert("RGBA")
lh = 170
lw = int(leaf.width * (lh / leaf.height))
leaf = leaf.resize((lw, lh), Image.LANCZOS)
img.alpha_composite(leaf, (cx - lw // 2, cy - lh // 2))

# --- text ---
draw = ImageDraw.Draw(img)


def font(path, size):
    try:
        return ImageFont.truetype(path, size)
    except Exception:
        return ImageFont.load_default()


f_word = font(r"C:\Windows\Fonts\segoeuib.ttf", 70)
f_tag = font(r"C:\Windows\Fonts\segoeui.ttf", 26)

tx = cx + lw // 2 + 36
# wordmark
word = "Genesi OS"
wb = draw.textbbox((0, 0), word, font=f_word)
wh = wb[3] - wb[1]
tag = "Where creations begin"
tb = draw.textbbox((0, 0), tag, font=f_tag)
th = tb[3] - tb[1]

gap = 14
block_h = wh + gap + th
top = cy - block_h // 2 - wb[1]
draw.text((tx, top), word, font=f_word, fill=WORD)

tag_y = top + wb[1] + wh + gap - tb[1]
draw.text((tx, tag_y), tag, font=f_tag, fill=TAG)

# thin mint accent under the wordmark
underline_y = top + wb[1] + wh + 6
draw.rectangle([tx, underline_y, tx + 54, underline_y + 4], fill=MINT)

img.convert("RGBA").save(OUT)
print("wrote", OUT, img.size)
