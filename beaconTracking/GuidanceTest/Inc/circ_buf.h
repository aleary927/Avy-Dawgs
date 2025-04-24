#ifndef CIRC_BUF_H
#define CIRC_BUF_H

#include <stdint.h>

typedef struct {
    float   *buf;
    uint32_t idx;
    uint32_t size;
} circ_buf_float;

void  circ_buf_wr_float(circ_buf_float *circ, float val);
float circ_buf_rd_float(circ_buf_float *circ);

#endif /* CIRC_BUF_H */
