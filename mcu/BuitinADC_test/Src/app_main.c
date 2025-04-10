#include "main.h"
#include "led.h"
#include "stm32f722xx.h"
#include "stm32f7xx_nucleo_144.h"
#include "adc.h"
#include "arm_math.h"
#include <stdint.h>
#include <stm32f7xx_hal.h>
#include "dwt.h"
#include "globals.h"
#include "dsp.h"

float goertzelbufx[GOERTZEL_BUF_SIZE];
float goertzelbufy[GOERTZEL_BUF_SIZE];
uint32_t goertzelbufx_pos = 0;
uint32_t goertzelbufy_pos = 0;

int inbufx_rdy = 0; 
int inbufy_rdy = 0;

int config_cplt = 0;

void process(int16_t *buf, float *gbuf, uint32_t *gbuf_pos);

void app_main(void)
{
  // enable caches
  SCB_EnableICache();
  SCB_EnableDCache();

  LED_Init();
  ADC_DMA_Config();
  DWT_Init();

  // config is now complete
  config_cplt = 1;

  int32_t sum_prev = 0;

  while (1) {
    // poll for buffers ready
    if (inbufx_rdy) {
      int32_t sum = 0;
      for (int i = 0; i < BUF_SIZE; i++) {
        sum += inbufx[i];
      }
      if (sum == sum_prev) {
        LED_Toggle(LED3_PIN);
      }
      sum_prev = sum;
      inbufx_rdy = 0; 
      // uint32_t t1 = DWT_GetCount();
      process((int16_t*) inbufx, goertzelbufx, &goertzelbufx_pos);
      // uint32_t diff = DWT_GetCount() - t1;
      // t1 = 0;
    }
    if (inbufy_rdy) {
      inbufy_rdy = 0; 
      process((int16_t*) inbufy, goertzelbufy, &goertzelbufy_pos);
    }

    LED_Toggle(LED2_PIN);
  }
}


void process(int16_t *buf, float *gbuf, uint32_t *gbuf_pos)
{
  // subtract away dc op point
  for (int i = 0; i < BUF_SIZE; i+=4) {
    buf[i] -= 2048;
    buf[i+1] -= 2048;
    buf[i+2] -= 2048;
    buf[i+3] -= 2048;
  }

  // calc power at 457 kHz
  float gres = goertzel_power(buf);

  // save in buffer 
  gbuf[*gbuf_pos] = gres;
  (*gbuf_pos)++; 
  if ((*gbuf_pos) == GOERTZEL_BUF_SIZE) {
    *gbuf_pos = 0;
  }

  float gsum;
  for (int i = 0; i < GOERTZEL_BUF_SIZE; i++) {
    gsum += gbuf[i];
  }

  float avg = gsum / GOERTZEL_BUF_SIZE;
}
