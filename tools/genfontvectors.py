#!/usr/bin/env python3

from HersheyFonts import HersheyFonts
import turtle

def label_for_vectors(c):
    return 'char_%02x_vec' % ord(c)

def begin_char(c):
    print('static const uint8_t ' + label_for_vectors(c) + '[] = {')
    turtle.clear()

def move_to(x, y):
    print('  \'M\', %2d, %2d,' % (x, y))
    turtle.penup()
    turtle.goto(x*2, y*2)

def line_to(x, y):
    print('  \'L\', %2d, %2d,' % (x, y))
    turtle.pendown()
    turtle.goto(x*2, y*2)

def end_char(c):
    print('  \'E\'')
    print('};')
    print()

all_chars = " 0123456789:.!?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
widths = [0 for i in range(0, 128)]

turtle.mode('logo')
turtle.speed(10)
turtle.setup(500, 500)

font = HersheyFonts()
font.load_default_font()
for char in all_chars:
    glyph = next(font.glyphs_for_text(char))
    curPenPos = (0, 0)
    begin_char(char)
    baseline = glyph.base_line
    left = -glyph.char_box[0][0]
    for p0, p1 in glyph.lines:
        x0, y0 = p0
        x1, y1 = p1
        x0 += left; x1 += left
        y0 = baseline - y0; y1 = baseline - y1
        if (x0, y0) != curPenPos:
            move_to(x0, y0)
        line_to(x1, y1)
        curPenPos = (x1, y1)
    widths[ord(char)] = left + glyph.char_box[1][0]
    end_char(char)

print('const uint8_t *glyph_vectors[128] = {')
vecs = []
for i in range(0, 128):
    if chr(i) in all_chars:
        s = label_for_vectors(chr(i))
    else:
        if chr(i).upper() in all_chars:
            s = label_for_vectors(chr(i).upper())
        else:
            s = 'NULL'
    vecs += ['%12s' % s]
print(',\n'.join(['  ' + ','.join(vecs[i:i+4]) for i in range(0, 128, 4)]))
print('};')
print()

print('const uint8_t glyph_width[128] = {')
str_widths = list(map(lambda w: '%2d' % w, widths))
print(',\n'.join(['  ' + ','.join(str_widths[i:i+16]) for i in range(0, 128, 16)]))
print('};')
