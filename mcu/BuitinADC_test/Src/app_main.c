#include "main.h"
#include "led.h"
#include "stm32f722xx.h"
#include "stm32f7xx_nucleo_144.h"
#include "adc.h"
#include <stdint.h>
#include <stm32f7xx_hal.h>
#include "dwt.h"
#include "globals.h"
#include "dsp.h"
#include "constants.h"
#include "UART.h"
#include <stdio.h>
#include "circ_buf.h"
#include "guidance.h"

/*********************** 
 * Defines 
 * ********************/

#define POWER_AVG_BUF_SIZE 100
#define POWER_BUF_SIZE 5

/********************* 
 * Globals 
 * ******************/

// power buffers
float powerbufx[POWER_BUF_SIZE];
float powerbufy[POWER_BUF_SIZE];
circ_buf_float powerbufcircx; 
circ_buf_float powerbufcircy;

// avg power buffers
float avgpowerbufx[POWER_AVG_BUF_SIZE];
float avgpowerbufy[POWER_AVG_BUF_SIZE];
circ_buf_float avgpowerbufcircx;
circ_buf_float avgpowerbufcircy;

// flag to tell if configuration complete
volatile int config_cplt;

// count of ADC bursts
int burst_count;

// buffer for usart transmit
char uart_buf[100];

// Guidance state and parameters
static GuidanceState  g_guidance_state;
static GuidanceParams g_guidance_params = 
{
  .buf_size      = POWER_AVG_BUF_SIZE,   // consume the same buffer size you’re averaging over
  .hist_size     = POWER_AVG_BUF_SIZE,   // size of the rolling‐average window
  .drop_steps    = 10,                   // how many drops before a U-turn
  .reverse_cd    = 40,                   // cooldown ticks after a U-turn
  .fwd_thresh    = 3.14159265/8.0f,            // straight‐ahead if angle ≤ 22.5°
  .min_valid_mag =  0.0f                 // ignore magnitudes < 0 (i.e. never ignore)
};

/*************************
 * Function prototypes. 
 * **********************/

void process_step(void);
void power_calc(int16_t *buf, circ_buf_float *pbuf);
void avg_power(float *pbuf, circ_buf_float *avg_pbuf);
void app_init(void);

/*************************** 
 * Functions 
 * *************************/

void app_main(void)
{
  config_cplt = 0;

  // enable caches
  SCB_EnableICache();
  SCB_EnableDCache();

  // configure peripherals
  LED_Init();
  ADC_DMA_Config();
  DWT_Init();
  UART_Config();

  // initialize app
  app_init();

  // config is now complete
  config_cplt = 1;

  // MAIN WHILE LOOP
  while (1) 
  {
    // poll for buffers ready
    if (inbufx_rdy || inbufy_rdy) 
    {
      process_step();
    }

    LED_Toggle(LED2_PIN);
  }
}

/*
 * Init globals.
 */
void app_init(void) 
{
  // initialize burst count
  burst_count = 0;

  // init power circ bufs
  circ_buf_init_float(&powerbufcircx, powerbufx, POWER_BUF_SIZE);
  circ_buf_init_float(&powerbufcircy, powerbufy, POWER_BUF_SIZE);

  // init avg power circ bufs
  circ_buf_init_float(&avgpowerbufcircx, avgpowerbufx, POWER_AVG_BUF_SIZE);
  circ_buf_init_float(&avgpowerbufcircy, avgpowerbufy, POWER_AVG_BUF_SIZE);

  guidance_state_init(&g_guidance_state, &g_guidance_params);
}

void process_step(void) 
{
  // process both buffers
  switch((inbufy_rdy << 1) | inbufx_rdy) {
    // x ready only
    case 0x1:
      inbufx_rdy = 0;
      power_calc((int16_t*) inbufx, &powerbufcircx);

      // wait for y buffer
      while(!inbufy_rdy);
      inbufy_rdy = 0;
      power_calc((int16_t*) inbufy, &powerbufcircy);
    break;
    // y ready only
    case 0x2: 
      inbufy_rdy = 0;
      power_calc((int16_t*) inbufy, &powerbufcircy);

      // wait for x buffer
      while(!inbufx_rdy);
      inbufx_rdy = 0;
      power_calc((int16_t*) inbufx, &powerbufcircx);
    break;
    // both ready
    case 0x3: 
      inbufx_rdy = 0; 
      inbufy_rdy = 0;
      power_calc((int16_t*) inbufx, &powerbufcircx);
      power_calc((int16_t*) inbufy, &powerbufcircy);
    break;
    default: 
      // should never happen
    break;

  }

  // increment burst count, calc the average power if power buf is full
  burst_count++; 
  if (burst_count == POWER_BUF_SIZE) {
    burst_count = 0;

    avg_power(powerbufx, &avgpowerbufcircx); 
    avg_power(powerbufy, &avgpowerbufcircy);

    int avgpower_x = circ_buf_rd_float(&avgpowerbufcircx);
    int avgpower_y = circ_buf_rd_float(&avgpowerbufcircy);

    //Implement Guidance function call here.
    Direction dir = guidance_step(avgpowerbufcircx.buf, avgpowerbufcircy.buf, avgpowerbufcircx.idx, avgpowerbufcircy.idx, &g_guidance_state, &g_guidance_params);
    const char *dir_str = "??";
    switch(dir) 
    {
      case STRAIGHT_AHEAD: dir_str = "FWD";     
        break;
      case TURN_LEFT:      dir_str = "LEFT";    
        break;
      case TURN_RIGHT:     dir_str = "RIGHT";   
        break;
      case TURN_AROUND:    dir_str = "UTURN";   
        break;
    }
    snprintf(uart_buf, 99, "avg x: %d  avg y: %d  dir: %s\n\r", avgpower_x, avgpower_y, dir_str);
    UART_Transmit(uart_buf);
  }
}

/*
* Calculate power of input samples, and save in power buffer.
*/
void power_calc(int16_t *buf, circ_buf_float *pbuf)
{
  // subtract away dc op point and apply window
  for (int i = 0; i < BUF_SIZE; i+=2) {
    int32_t intres0 = (buf[i] - 2048) * flattop_int16_3600[i];
    int32_t intres1 = (buf[i+1] - 2048) * flattop_int16_3600[i+1];

    // shift back into 16 bit range
    buf[i] = intres0 >> 12;
    buf[i+1] = intres1 >> 12;
  }

  // calc power at 457 kHz
  float power = goertzel_power_457k(buf);
  // clamp to 1 (to avoid negative power dB readings)
  if (power < 1) {
    power = 1;
  }

  // convert power to dB
  float power_db = 10 * log10f(power);

  // save in buffer 
  circ_buf_wr_float(pbuf, power_db);
}

/*
* Take the average of the power buffer and save in average power buffer.
*/
void avg_power(float *pbuf, circ_buf_float *avg_pbuf)
{
  // calc average of power buffer
  float sum = 0;

  for (int i = 0; i < POWER_BUF_SIZE; i++) {
    sum += pbuf[i];
  }
  float avg = sum / POWER_BUF_SIZE;

  // save average in average power buffer
  circ_buf_wr_float(avg_pbuf, avg);
}
