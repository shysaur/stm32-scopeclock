#include "utils.h"


static int formatInt(int32_t num, bool s, int base, int plus, int uc, int minw, char *out)
{
  char dlut[] = "0123456789ABCDEF0123456789abcdef";
  uint32_t abs_value;
  int n;
  char *start = out, *swap = out, t;
  
  if (s) {
    if (num >= 0) {
      abs_value = num;
      if (plus) {
        swap++;
        if (plus == PLUS_SPACE)
          *(out++) = ' ';
        else
          *(out++) = '+';
      }
    } else {
      abs_value = -num;
      *(out++) = '-';
      swap++;
    }
  } else
    abs_value = (uint32_t)num;
    
  uc = uc ? 0 : 16;
    
  do {
    *(out++) = dlut[abs_value % base + uc];
    abs_value /= base;
    minw--;
  } while (abs_value > 0 || minw > 0);
  n = (int)(out - start);
  *(out--) = '\0';
  
  for (; out>swap; swap++, out--) {
    t = *swap;
    *swap = *out;
    *out = t;
  }
  
  return n;
}

int formatSignedInt(int32_t n, int base, int plus, bool uppercase, int minimumWidth, char *out)
{
  return formatInt(n, true, base, plus, uppercase, minimumWidth, out);
}

int formatUnsignedInt(uint32_t n, int base, bool uppercase, int minimumWidth, char *out)
{
  return formatInt((uint32_t)n, false, base, PLUS_NONE, uppercase, minimumWidth, out);
}

int formatFixedPoint(t_fixp n, int base, int plus, bool uppercase, int fracWidth, int minimumWidth, char *out)
{
  bool negative = n < 0;
  uint32_t intpart = ABS(n) / FIX_1;
  uint32_t fracpart = ABS(n) % FIX_1;
  char *p = out;

  if (fracWidth < 0)
    fracWidth = 4;
  else if (fracWidth > 4)
    fracWidth = 4;
  minimumWidth -= fracWidth + 1;
  uint32_t power = 1;
  for (int i=0; i<fracWidth; i++)
    power *= base;
  fracpart = fracpart * power / FIX_1;

  if (negative) {
    *p++ = '-';
    minimumWidth--;
  } else {
    if (plus) {
      *p++ = plus == PLUS_SPACE ? ' ' : '+';
      minimumWidth--;
    }
  }
  p += formatUnsignedInt(intpart, base, uppercase, minimumWidth, p);
  *p++ = '.';
  p += formatUnsignedInt(fracpart, base, uppercase, fracWidth, p);
  return (int)(p - out);
}
