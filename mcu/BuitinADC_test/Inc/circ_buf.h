#include <stdint.h>

typedef struct {
  float *buf; 
  uint32_t idx;
  uint32_t size;
} circ_buf_float;

typedef struct {
  uint32_t *buf; 
  uint32_t idx; 
  uint32_t size;
} circ_buf_uint32;

/***** FLOAT *****/

void circ_buf_init_float(circ_buf_float *circ, float *buf, uint32_t size)
{
  circ->idx = 0; 
  circ->buf = buf;
  circ->size = size;
}

void circ_buf_wr_float(circ_buf_float *circ, float val)
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

float circ_buf_rd_float(circ_buf_float *circ) 
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

/***** UINT32_T *****/

void circ_buf_init_uint32(circ_buf_uint32 *circ, uint32_t *buf, uint32_t size)
{
  circ->idx = 0; 
  circ->buf = buf;
  circ->size = size;
}

void circ_buf_wr_uint32(circ_buf_uint32 *circ, uint32_t val) 
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

uint32_t circ_buf_rd_uint32(circ_buf_uint32 *circ) 
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
