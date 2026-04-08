from PIL import Image, ImageDraw, ImageFont
import sys

# Tamanho desejado da fonte para ficar moderno (16x32)
FONT_WIDTH = 12
FONT_HEIGHT = 24

try:
    # Use any modern font available, let's download Inter or Roboto dynamically if not present
    import urllib.request
    import os
    
    font_file = "Inter-Regular.ttf"
    if not os.path.exists(font_file):
        print("Downloading font...")
        urllib.request.urlretrieve("https://github.com/googlefonts/roboto/raw/main/src/hinted/Roboto-Regular.ttf", font_file)
        
    font = ImageFont.truetype(font_file, 15)
    
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
            # bbox is (left, top, right, bottom)
            # just draw it centered-ish
            draw.text((0, 0), char, font=font, fill=255)
        except:
            draw.text((0, 0), char, font=font, fill=255)
            
        pixels = list(img.getdata())
        
        row_bytes = [str(p) for p in pixels]
        out += "    { " + ", ".join(row_bytes) + " },\n"
        
    out += "};\n\n#endif\n"
    
    with open("kernel/gfx/font_data.h", "w") as f:
        f.write(out)
        
    print("Generated smooth anti-aliased font successfully!")
    
except Exception as e:
    print("Error:", e)
