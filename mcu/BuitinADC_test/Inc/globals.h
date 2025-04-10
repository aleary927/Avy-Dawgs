/*
 * A common header file for global buffers and flags.
 */

#include "main.h"
// #include "arm_math.h"
#include <stdint.h>

// input buffer data size
#define BUF_SIZE 2048

// time between bursts (in ms)
#define BURST_PERIOD_MS 10

// configuration complete flag
extern int config_cplt;

// input buffers
extern int16_t inbufx[BUF_SIZE]; 
extern int16_t inbufy[BUF_SIZE]; 
// input buffer flags
extern int inbufx_rdy; 
extern int inbufy_rdy;


// goertzel buffer size
#define GOERTZEL_BUF_SIZE 10

// goertzel buffers
extern float goertzelbufx[GOERTZEL_BUF_SIZE];
extern float goertzelbufy[GOERTZEL_BUF_SIZE];
extern uint32_t goertzelbufx_pos;
extern uint32_t goertzelbufy_pos;
