from PIL import Image, ImageDraw, ImageFont
import sys

# Tamanho desejado da fonte para ficar moderno (16x32)
FONT_WIDTH = 12
FONT_HEIGHT = 24

try:
    # Use any modern font available, let's download Inter or Roboto dynamically if not present
    import urllib.request
    import os
    
    font_file = "RobotoMono-Regular.ttf"
    if not os.path.exists(font_file):
        print("Downloading font...")
        urllib.request.urlretrieve("https://github.com/googlefonts/RobotoMono/raw/main/fonts/ttf/RobotoMono-Regular.ttf", font_file)
        
    # Uma fonte Mono tamanho 20 cabe perfeitamente em 12x24
    font = ImageFont.truetype(font_file, 20)
    
    out = "#ifndef FONT_DATA_H\n#define FONT_DATA_H\n\n"
    out += f"#define FONT_WIDTH {FONT_WIDTH}\n"
    out += f"#define FONT_HEIGHT {FONT_HEIGHT}\n\n"
    out += f"static const unsigned char font_alpha[256][{FONT_WIDTH * FONT_HEIGHT}] = {{\n"
    
    for i in range(256):
        # Cria uma imagem em escala de cinza (L) com fundo preto (0)
        img = Image.new('L', (FONT_WIDTH, FONT_HEIGHT), 0)
        draw = ImageDraw.Draw(img)
        
        char = chr(i) if i >= 32 and i <= 126 else '?'
        if i < 32: char = ' ' # Ignore non-printable
        
        # Render text with white (255)
        # We need to center the text slightly.
        # getbbox or textlength could be used.
        try:
            bbox = font.getbbox(char)
            # centraliza a letra horizontalmente no bloco de 12px
            w = bbox[2] - bbox[0]
            offset_x = (FONT_WIDTH - w) // 2
            # Roboto Mono pode ficar um pouco alta, descemos ela 2 pixels
            draw.text((offset_x, 2), char, font=font, fill=255)
        except:
            draw.text((0, 2), char, font=font, fill=255)
            
        pixels = list(img.getdata())
        
        row_bytes = [str(p) for p in pixels]
        out += "    { " + ", ".join(row_bytes) + " },\n"
        
    out += "};\n\n#endif\n"
    
    with open("kernel/gfx/font_data.h", "w") as f:
        f.write(out)
        
    print("Generated smooth anti-aliased font successfully!")
    
except Exception as e:
    print("Error:", e)
