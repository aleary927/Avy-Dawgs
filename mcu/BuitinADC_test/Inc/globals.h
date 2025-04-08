/*
 * A common header file for global buffers and flags.
 */

#include "main.h"
#include "arm_math.h"
#include <stdint.h>

// input buffer data size
#define BUF_SIZE 2048

// time between bursts (in ms)
#define BURST_PERIOD_MS 10

// configuration complete flag
extern int config_cplt;

// input buffers
extern uint16_t inbufx[BUF_SIZE]; 
extern uint16_t inbufy[BUF_SIZE]; 
// input buffer flags
extern int inbufx_rdy; 
extern int inbufy_rdy;


// goertzel buffer size
#define GOERTZEL_BUF_SIZE 10

// goertzel buffers
extern float32_t goertzelbufx[GOERTZEL_BUF_SIZE];
extern float32_t goertzelbufy[GOERTZEL_BUF_SIZE];
extern uint32_t goertzelbufx_pos;
extern uint32_t goertzelbufy_pos;
