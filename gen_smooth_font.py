from PIL import Image, ImageDraw, ImageFont
import sys

# Aumentando a resolução da fonte base para ficar bem mais legível (16x32)
FONT_WIDTH = 16
FONT_HEIGHT = 32

try:
    # Use any modern font available, let's download Inter or Roboto dynamically if not present
    import urllib.request
    import os
    
    font_file = "RobotoMono-Regular.ttf"
    if not os.path.exists(font_file):
        print("Downloading font...")
        urllib.request.urlretrieve("https://github.com/googlefonts/RobotoMono/raw/main/fonts/ttf/RobotoMono-Regular.ttf", font_file)
        
    # Fonte maior, renderizada nativamente com bom anti-aliasing
    font = ImageFont.truetype(font_file, 26)
    
    out = "#ifndef FONT_DATA_H\n#define FONT_DATA_H\n\n"
    out += f"#define FONT_WIDTH {FONT_WIDTH}\n"
    out += f"#define FONT_HEIGHT {FONT_HEIGHT}\n\n"
    out += f"static const unsigned char font_alpha[256][{FONT_WIDTH * FONT_HEIGHT}] = {{\n"
    
    for i in range(256):
        # Renderização super amostrada para melhor anti-aliasing
        # Renderizamos 2x maior e depois damos downscale (Supersampling)
        img_large = Image.new('L', (FONT_WIDTH * 2, FONT_HEIGHT * 2), 0)
        draw = ImageDraw.Draw(img_large)
        font_large = ImageFont.truetype(font_file, 52)
        
        char = chr(i) if i >= 32 and i <= 126 else '?'
        if i < 32: char = ' ' # Ignore non-printable
        
        try:
            bbox = font_large.getbbox(char)
            w = bbox[2] - bbox[0]
            offset_x = (FONT_WIDTH * 2 - w) // 2
            draw.text((offset_x, 4), char, font=font_large, fill=255)
        except:
            draw.text((0, 4), char, font=font_large, fill=255)
            
        # Downscale com filtro LANCZOS para anti-aliasing perfeito
        img = img_large.resize((FONT_WIDTH, FONT_HEIGHT), Image.Resampling.LANCZOS)
        pixels = list(img.getdata())
        
        row_bytes = [str(p) for p in pixels]
        out += "    { " + ", ".join(row_bytes) + " },\n"
        
    out += "};\n\n#endif\n"
    
    with open("kernel/gfx/font_data.h", "w") as f:
        f.write(out)
        
    print("Generated smooth anti-aliased font successfully!")
    
except Exception as e:
    print("Error:", e)
