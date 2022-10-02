#!/usr/bin/env python3

from HersheyFonts import HersheyFonts
import turtle
import math

GLYPH_ASCENDER = 21
TURTLE = False
SCALE = 100/GLYPH_ASCENDER
FONT = 'futural'

def label_for_vectors(c):
    return 'char_%02x_vec' % ord(c)

class Plotter:
    def __init__(self, c, scale=SCALE):
        self.c = c
        self.t = 0
        self.x, self.y = 0, 0
        self.scale = scale
        self.points = []
        if TURTLE:
            turtle.clear()

    def move_to(self, x, y):
        x, y = x*self.scale, y*self.scale
        self.x, self.y = x, y
        if TURTLE:
            turtle.penup()
            turtle.goto(x, y)

    def line_to(self, x, y):
        if TURTLE:
            turtle.pendown()
        x, y = x*self.scale, y*self.scale
        dx, dy = x - self.x, y - self.y
        dist = math.sqrt(dx**2 + dy**2)
        end_t = self.t + dist
        for t in range(math.ceil(self.t), math.floor(end_t), 1):
            px = (t - self.t) / dist * dx + self.x
            py = (t - self.t) / dist * dy + self.y
            if TURTLE:
                turtle.goto(px, py)
            self.points += [(px, py)]
        self.t = end_t
        self.x, self.y = x, y
    
    def end(self):
        print('static const uint8_t ' + label_for_vectors(self.c) + '[] = {')
        if len(self.points) > 65535:
            raise Exception('there are too many points in character ' + self.c)
        print('  %4d,%4d,' % (len(self.points) % 256, len(self.points) / 256))
        data = ['%4d,%4d' % (x, y) for x, y in self.points]
        lines = ',\n'.join(['  '+','.join(data[i:i+8]) for i in range(0, len(data), 8)])
        print(lines)
        print('};')
        print()


all_chars = " 0123456789:.!?()ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
widths = [0 for i in range(0, 128)]

if TURTLE:
    turtle.mode('logo')
    turtle.speed(10)
    turtle.setup(SCALE*GLYPH_ASCENDER*3/2, SCALE*GLYPH_ASCENDER*3/2, 0, 0)

font = HersheyFonts()
font.load_default_font(FONT)
for char in all_chars:
    glyph = next(font.glyphs_for_text(char))
    curPenPos = (0, 0)
    plotter = Plotter(char)
    baseline = glyph.base_line
    left = -glyph.char_box[0][0]
    for p0, p1 in glyph.lines:
        x0, y0 = p0
        x1, y1 = p1
        x0 += left; x1 += left
        y0 = baseline - y0; y1 = baseline - y1
        if (x0, y0) != curPenPos:
            plotter.move_to(x0, y0)
        plotter.line_to(x1, y1)
        curPenPos = (x1, y1)
    widths[ord(char)] = left + glyph.char_box[1][0]
    plotter.end()

print('const uint8_t * const '+FONT+'_glyph_vectors[128] = {')
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

print('const uint8_t '+FONT+'_glyph_width[128] = {')
str_widths = list(map(lambda w: '%4d' % (w*SCALE), widths))
print(',\n'.join(['  ' + ','.join(str_widths[i:i+16]) for i in range(0, 128, 16)]))
print('};')
