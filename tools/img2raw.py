import sys
from PIL import Image

def main():
    if len(sys.argv) < 3:
        print("Usage: py img2raw.py <input.png> <output.raw>")
        sys.exit(1)
        
    input_path = sys.argv[1]
    output_path = sys.argv[2]
    
    img = Image.open(input_path).convert("RGBA")
    # For now, we assume we want exactly 1920x1080 to match the screen
    if img.size != (1920, 1080):
        img = img.resize((1920, 1080), Image.Resampling.LANCZOS)
    
    pixels = img.getdata()
    
    with open(output_path, "wb") as f:
        for r, g, b, a in pixels:
            # Output in ARGB little-endian format (which matches our uint32_t fb format: 0xAARRGGBB)
            # Actually, framebuffer format is usually B G R A in memory for little endian
            # Let's write B, G, R, A bytes
            f.write(bytes([b, g, r, a]))
            
    print(f"Saved {output_path} ({img.width}x{img.height})")

if __name__ == "__main__":
    main()