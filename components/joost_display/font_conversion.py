#
# this is a small tool to convert font2bitmap files to c++ structures
# there are many assumptions, and no error checking
#

import json
from PIL import Image

meta = json.load(open("tmp/font2bitmap.json"))
img = Image.open('tmp/font2bitmap.png').convert('RGB')
data = img.tobytes()
stride = img.size[0]*3

def getBit(x,y):
    i = y * stride + x * 3;
    s = data[i+0]+data[i+1]+data[i+2]
    if s>0:
        return 1
    return 0

chars = meta["chars"]

#
# Reduce the bounding boxes
#
for char in chars:
    c = char["char"]
    x = char["x"]
    y = char["y"]
    w = char["width"]
    h = char["height"]
    ox = char["xoffset"]
    oy = char["yoffset"]
    # horizontal bounding box - Right side
    for b in range(w):
        v = 0
        for l in range(h):
            v += getBit(x+w-b-1,y+l)
        if v == 0:
            w -= 1;
        else:
            break
    # horizontal bounding box - Left side
    for b in range(w):
        v = 0
        for l in range(h):
            v += getBit(x+b,y+l)
        if v == 0:
            x += 1;
            w -= 1;
            ox += 1;
        else:
            break
    # vertical bounding box - Bottom side
    for l in range(h):
        v = 0
        for b in range(w):
            v += getBit(x+b,y+h-l-1)
        if v == 0:
            h -= 1;
        else:
            break
    # vertical bounding box - Top side
    for l in range(h):
        v = 0
        for b in range(w):
            v += getBit(x+b,y+l)
        if v == 0:
            y += 1;
            h -= 1;
            oy += 1
        else:
            break
    # update the meta data
    char["x"] = x
    char["y"] = y
    char["xoffset"] = ox
    char["yoffset"] = oy
    char["width"] = w
    char["height"] = h
    
#
# Create a header file
#
with open("font_data.h","w") as f:
    f.write("#pragma once\n")
    f.write("#include <array>\n")
    f.write("#include <map>\n")
    f.write("#include <stdint.h>\n")
    f.write("struct font_data_struct\n")
    f.write("{\n")
    f.write("  uint8_t width;\n")
    f.write("  uint8_t height;\n")
    f.write("  int8_t offset_x;\n")
    f.write("  int8_t offset_y;\n")
    f.write("  int8_t advance;\n")
    f.write("  const uint8_t * data;\n")
    f.write("};\n")
    f.write("extern std::map<char,font_data_struct> font_data;\n")
    f.write("extern std::map<std::pair<char,char>,int> font_kernings;\n")
    f.write("#define ASCII_HEART 255\n")
#
# Create the cpp file
#
with open("font_data.cpp","w") as f:
    f.write("#include \"font_data.h\"\n")
    #
    # write the glyphs
    #
    for char in chars:
        c = char["char"]
        x = char["x"]
        y = char["y"]
        w = char["width"]
        h = char["height"]
        f.write("static const uint8_t glyph_{}[] = {{".format(ord(c)))
        i = int(1);
        v = int(0);
        for l in range(h):
            for b in range(w):
                if getBit(x+b,y+l) == 1:
                    v += i 
                i = i << 1;
                if i == 256:
                    f.write("{},".format(v))
                    i = int(1)
                    v = int(0)
        f.write("{}}};\n".format(v))
    f.write("// predefined\n")
    f.write(r"static const uint8_t glyph_255[] = {0,0,0,0,0,0,0,0,0,0,0,248,15,254,129,255,243,127,248,255,255,31,255,255,255,243,255,255,255,254,255,255,223,255,255,255,251,255,255,127,255,255,255,239,255,255,255,253,255,255,63,255,255,255,227,255,255,127,248,255,255,7,254,255,127,192,255,255,15,240,255,255,0,252,255,15,0,255,255,0,192,255,15,0,240,255,0,0,252,15,0,0,255,0,0,192,15,0,0,96,0,0};")
    #
    # Write the glyph meta data
    #
    f.write("\nstd::map<char,font_data_struct> font_data\n{\n");
    for char in chars:
        c = char["char"]
        g = ord(c)
        c = c.replace("\\","\\\\").replace('"',"\\\"").replace("\'","\\\'")
        w = char["width"]
        h = char["height"]
        ox = char["xoffset"]
        oy = char["yoffset"]
        a = char["xadvance"] + 2 # the advance for this font is very tight for some characters. fix that
        f.write("  {{\'{}\',{{{},{},{},{},{},glyph_{}}}}},\n".format(c,w,h,ox,oy,a,g))
    f.write("  // predefined\n")
    f.write(r"  {ASCII_HEART,{29,28,2,3,26,glyph_255}}," + "\n")
    f.write("};\n")
    #
    # Write the kerning (offsets for specific sequences of characters)
    #
    f.write("std::map<std::pair<char,char>,int> font_kernings\n{\n");
    kernings = meta["kernings"]
    for kerning in kernings:
        a = kerning["first"]
        b = kerning["second"]
        c = kerning["amount"]
        f.write("  {{{{{},{}}},{}}},\n".format(a,b,c))
    f.write("};\n")


