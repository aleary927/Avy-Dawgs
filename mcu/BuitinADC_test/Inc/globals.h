/*
 * A common header file for global buffers and flags.
 */

#include "main.h"
#include <stdint.h>

// input buffer data size
#define BUF_SIZE 4096

// time between bursts (in ms)
#define BURST_PERIOD_MS 10

// configuration complete flag
extern volatile int config_cplt;

// input buffers
extern volatile int16_t inbufx[BUF_SIZE]; 
extern volatile int16_t inbufy[BUF_SIZE]; 
// input buffer flags
extern volatile int inbufx_rdy; 
extern volatile int inbufy_rdy;
