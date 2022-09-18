#!/usr/bin/env python3

import math

vals = [round(math.sin(i / 256 * (math.pi / 2)) * 255) for i in range(0, 256)]
items = ['0x%02x' % v for v in vals]
grp_items = [items[i:i+8] for i in range(0, 256, 8)]
lines = [', '.join(line)+',' for line in grp_items]
print('\n'.join(lines))
