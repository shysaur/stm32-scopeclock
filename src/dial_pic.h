#ifndef DIAL_PIC_H
#define DIAL_PIC_H

#include <stdint.h>
#include "draw.h"

#define DIAL_FACE_CMDBUF_SIZE 591
extern const uint8_t dial_face_cmdbuf[DIAL_FACE_CMDBUF_SIZE];

extern t_plot dial_plot;

void initDialPlot(void);

#endif
