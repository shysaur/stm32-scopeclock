#include "utils.h"


static int formatInt(uint32_t num, bool s, int base, int plus, bool uc, int minw, char *out)
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
  return formatInt((uint32_t)n, true, base, plus, uppercase, minimumWidth, out);
}

int formatUnsignedInt(uint32_t n, int base, bool uppercase, int minimumWidth, char *out)
{
  return formatInt(n, false, base, PLUS_NONE, uppercase, minimumWidth, out);
}
