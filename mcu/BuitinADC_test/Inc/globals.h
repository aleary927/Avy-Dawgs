/*
 * A common header file for global buffers and flags.
 */

#include "main.h"
#include "arm_math.h"
#include <stdint.h>

// input buffer data size
#define BUF_SIZE 2048

// input buffers
extern uint16_t buf0[BUF_SIZE]; 
extern uint16_t buf1[BUF_SIZE]; 
// input buffer flags
extern int buf0_rdy; 
extern int buf1_rdy;


// goertzel buffer size
#define NWINDOWS 10

// goertzel buffer
extern float32_t goertzelbuf[NWINDOWS];
extern int32_t goertzelbuf_pos;
