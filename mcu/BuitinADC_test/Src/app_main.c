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

#define POWER_AVG_BUF_SIZE 100

volatile int inbufx_rdy = 0; 
volatile int inbufy_rdy = 0;

float powerbufx[POWER_BUF_SIZE];
float powerbufy[POWER_BUF_SIZE];
uint32_t powerbufx_pos = 0;
uint32_t powerbufy_pos = 0;

float avgpowerbufx[POWER_AVG_BUF_SIZE];
float avgpowerbufy[POWER_AVG_BUF_SIZE];

circ_buf_float avgpowerbufcircx;
circ_buf_float avgpowerbufcircy;

volatile int config_cplt = 0;

float max_power_y; 
float max_power_x;

int count;

char uart_buf[100];

void power_calc(int16_t *buf, float *pbuf, uint32_t *pbuf_pos);
void avg_power(float *pbuf, circ_buf_float *avg_pbuf);

void app_main(void)
{
  // enable caches
  SCB_EnableICache();
  SCB_EnableDCache();

  // configure peripherals
  LED_Init();
  ADC_DMA_Config();
  DWT_Init();
  UART_Config();

  count = 0;

  avgpowerbufcircx.idx = 0; 
  avgpowerbufcircx.size = POWER_AVG_BUF_SIZE; 
  avgpowerbufcircx.buf = avgpowerbufx; 

  avgpowerbufcircy.idx = 0; 
  avgpowerbufcircy.size = POWER_AVG_BUF_SIZE; 
  avgpowerbufcircy.buf = avgpowerbufy; 

  // config is now complete
  config_cplt = 1;

  while (1) {
    // poll for buffers ready
    if (inbufx_rdy || inbufy_rdy) {
      switch((inbufy_rdy << 1) | inbufx_rdy) {
        // x ready only
        case 0x1:
          inbufx_rdy = 0;
          power_calc((int16_t*) inbufx, powerbufx, &powerbufx_pos);

          // wait for y buffer
          while(!inbufy_rdy);
          inbufy_rdy = 0;
          power_calc((int16_t*) inbufy, powerbufy, &powerbufy_pos);
        break;
        // y ready only
        case 0x2: 
          inbufy_rdy = 0;
          power_calc((int16_t*) inbufy, powerbufy, &powerbufy_pos);

          // wait for x buffer
          while(!inbufx_rdy);
          inbufx_rdy = 0;
          power_calc((int16_t*) inbufx, powerbufx, &powerbufx_pos);
        break;
        // both ready
        case 0x3: 
          inbufx_rdy = 0; 
          inbufy_rdy = 0;
          power_calc((int16_t*) inbufx, powerbufx, &powerbufx_pos);
          power_calc((int16_t*) inbufy, powerbufy, &powerbufy_pos);
        break;
        default: 
          // error TODO
        break;

      }

      count++; 
      if (count == POWER_BUF_SIZE) {
        count = 0;

        float new_max_x = 0;
        float new_max_y = 0;
        for (int i = 0; i < POWER_BUF_SIZE; i++) {
          if (powerbufx[i] > new_max_x) {
            new_max_x = powerbufx[i];
          }
          if (powerbufy[i] > new_max_y) {
            new_max_y = powerbufy[i];
          }
        }
        max_power_x = new_max_x;
        max_power_y = new_max_y;

        avg_power(powerbufx, &avgpowerbufcircx); 
        avg_power(powerbufy, &avgpowerbufcircy);

        int avgpower_x = circ_buf_read(&avgpowerbufcircx);
        int avgpower_y = circ_buf_read(&avgpowerbufcircy);

        snprintf(uart_buf, 99, "avg x: %d     avg y: %d\n\r", avgpower_x, avgpower_y);
        // snprintf(uart_buf, 99, "avg x: %d     avg y: %d\n\r", (int) max_power_x, (int) max_power_y);
        UART_Transmit(uart_buf);
       }
    }

    LED_Toggle(LED2_PIN);
  }
}


void power_calc(int16_t *buf, float *pbuf, uint32_t *pbuf_pos)
{
  // subtract away dc op point and apply window
  for (int i = 0; i < BUF_SIZE; i+=2) {
    int32_t intres0 = (buf[i] - 2048) * flattop_int16_2048[i];
    int32_t intres1 = (buf[i+1] - 2048) * flattop_int16_2048[i+1];
    buf[i] = intres0 >> 12;
    buf[i+1] = intres1 >> 12;
  }

  // calc power at 457 kHz
  float power = goertzel_power(buf);
  // clamp to 10 
  if (power < 1) {
    power = 1;
  }

  float power_db = 20 * log10f(power);

  // save in buffer 
  pbuf[*pbuf_pos] = power_db;
  (*pbuf_pos)++; 
  if ((*pbuf_pos) == POWER_BUF_SIZE) {
    *pbuf_pos = 0;
  }
}


void avg_power(float *pbuf, circ_buf_float *avg_pbuf)
{
  // calc average of power buffer
  float sum = 0;
  for (int i = 0; i < POWER_BUF_SIZE; i++) {
    sum += pbuf[i];
  }
  float avg = sum / POWER_BUF_SIZE;

  // save average in average power buffer
  circ_buf_write(avg_pbuf, avg);
}
