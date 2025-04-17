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
#include <math.h>
#include <stdio.h>

float goertzelbufx[GOERTZEL_BUF_SIZE];
float goertzelbufy[GOERTZEL_BUF_SIZE];
uint32_t goertzelbufx_pos = 0;
uint32_t goertzelbufy_pos = 0;

volatile int inbufx_rdy = 0; 
volatile int inbufy_rdy = 0;

volatile int config_cplt = 0;

float max_power_y; 
float max_power_x;

char uart_buf[100];

int count; 

void power_calc(int16_t *buf, float *gbuf, uint32_t *gbuf_pos);

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

  // config is now complete
  config_cplt = 1;

  while (1) {
    // poll for buffers ready
    if (inbufx_rdy || inbufy_rdy) {
      switch((inbufy_rdy << 1) | inbufx_rdy) {
        // x ready only
        case 0x1:
          inbufx_rdy = 0;
          power_calc((int16_t*) inbufx, goertzelbufx, &goertzelbufx_pos);

          // wait for y buffer
          while(!inbufy_rdy);
          inbufy_rdy = 0;
          power_calc((int16_t*) inbufy, goertzelbufy, &goertzelbufy_pos);
        break;
        // y ready only
        case 0x2: 
          inbufy_rdy = 0;
          power_calc((int16_t*) inbufy, goertzelbufy, &goertzelbufy_pos);

          // wait for x buffer
          while(!inbufx_rdy);
          inbufx_rdy = 0;
          power_calc((int16_t*) inbufx, goertzelbufx, &goertzelbufx_pos);
        break;
        // both ready
        case 0x3: 
          inbufx_rdy = 0; 
          inbufy_rdy = 0;
          power_calc((int16_t*) inbufx, goertzelbufx, &goertzelbufx_pos);
          power_calc((int16_t*) inbufy, goertzelbufy, &goertzelbufy_pos);
        break;
        default: 
          // error TODO
        break;

      }

      // find highest power
      float new_max_x = 0;
      float new_max_y = 0;
      for (int i = 0; i < GOERTZEL_BUF_SIZE; i++) {
        if (goertzelbufx[i] > new_max_x) {
          new_max_x = goertzelbufx[i];
        }
        if (goertzelbufy[i] > new_max_y) {
          new_max_y = goertzelbufy[i];
        }
      }
      max_power_x = new_max_x;
      max_power_y = new_max_y;


      count++; 
      if (count == 5 && !tx_in_progress) {
        snprintf(uart_buf, 99, "max x: %d     max y: %d\n\r\0", (int) max_power_x, (int) max_power_y);
        UART_Transmit(uart_buf);
        count = 0;
      }
    }


    LED_Toggle(LED2_PIN);
  }
}


void power_calc(int16_t *buf, float *gbuf, uint32_t *gbuf_pos)
{
  // subtract away dc op point and apply window
  for (int i = 0; i < BUF_SIZE; i+=2) {
    int32_t intres0 = (buf[i] - 2048) * flattop_int16_2048[i];
    int32_t intres1 = (buf[i+1] - 2048) * flattop_int16_2048[i+1];
    buf[i] = intres0 >> 12;
    buf[i+1] = intres1 >> 12;
  }

  // calc power at 457 kHz
  float power = 20 * log10(goertzel_power(buf));

  // save in buffer 
  gbuf[*gbuf_pos] = power;
  (*gbuf_pos)++; 
  if ((*gbuf_pos) == GOERTZEL_BUF_SIZE) {
    *gbuf_pos = 0;
  }
}
