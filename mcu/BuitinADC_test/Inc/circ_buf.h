#include <stdint.h>

typedef struct {
  float *buf; 
  uint32_t idx;
  uint32_t size;
} circ_buf_float;

void circ_buf_write(circ_buf_float *circ, float val)
{
  uint32_t idx = circ->idx;

  // update 
  circ->buf[idx] = val;

  // increment pointer
  idx++;
  if (idx == circ->size) {
    circ->idx = 0;
  }
  else {
    circ->idx = idx;
  }
}

float circ_buf_read(circ_buf_float *circ) 
{
  uint32_t idx = circ->idx;

  // get location to read
  if (idx == 0) {
    idx = circ->size - 1;
  }
  else {
    idx--;
  }

  return circ->buf[idx];
}
