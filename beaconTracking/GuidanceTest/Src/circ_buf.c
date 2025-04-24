// Src/circ_buf.c
#include "circ_buf.h"
#include <stdint.h>
#include <stdlib.h>

void circ_buf_wr_float(circ_buf_float *circ, float val)
{
    uint32_t idx = circ->idx;
    circ->buf[idx] = val;
    idx++;
    circ->idx = (idx == circ->size) ? 0 : idx;
}

float circ_buf_rd_float(circ_buf_float *circ)
{
    uint32_t idx = circ->idx;
    if (idx == 0)
        idx = circ->size - 1;
    else
        idx--;
    return circ->buf[idx];
}
