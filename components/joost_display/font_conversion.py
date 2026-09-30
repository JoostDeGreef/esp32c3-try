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
    char["ox"] = ox
    char["oy"] = oy
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
    f.write("extern std::map<char,font_data_struct> font_data;");
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
    #
    # Write the meta data
    #
    f.write("std::map<char,font_data_struct> font_data\n{\n");
    for char in chars:
        c = char["char"]
        g = ord(c)
        c = c.replace("\\","\\\\").replace('"',"\\\"").replace("\'","\\\'")
        ox = char["xoffset"]
        oy = char["yoffset"]
        a = char["xadvance"]
        f.write("  {{\'{}\',{{{},{},{},{},{},glyph_{}}}}},\n".format(c,w,h,ox,oy,a,g))
    f.write("};\n")


