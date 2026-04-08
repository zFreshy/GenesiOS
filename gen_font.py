import sys

try:
    with open("font.psf", "rb") as f:
        data = f.read()

    magic = data[:2]
    if magic != b"\x36\x04":
        print("Not a valid PSF1 file!")
        sys.exit(1)

    charsize = data[3]
    print(f"Char size: {charsize}")

    glyphs = data[4:4 + 256 * charsize]
    out = "#ifndef FONT_DATA_H\n#define FONT_DATA_H\n\n"
    out += "static const unsigned char font16x8[256][" + str(charsize) + "] = {\n"
    for i in range(256):
        row_bytes = [hex(b) for b in glyphs[i*charsize:(i+1)*charsize]]
        out += "    { " + ", ".join(row_bytes) + " },\n"
    out += "};\n\n#endif\n"

    with open("kernel/gfx/font_data.h", "w") as f:
        f.write(out)

    print("Generated kernel/gfx/font_data.h successfully!")
except Exception as e:
    print(e)
