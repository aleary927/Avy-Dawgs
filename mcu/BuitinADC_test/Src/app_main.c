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

float32_t goertzelbuf[NWINDOWS];
int32_t goertzelbuf_pos = 0;

int buf0_rdy; 
int buf1_rdy;

void process(int16_t *buf);

void app_main(void)
{
  SCB_EnableICache();
  SCB_EnableDCache();
  LED_Init();

  ADC_DMA_StreamConfig();
  DWT_Init();

  arm_rfft_instance_q15 S;
  arm_rfft_init_q15(&S, BUF_SIZE, 0, 0);

  while (1) {
    // poll for buffers ready
    if (buf0_rdy) {
      buf0_rdy = 0; 
      // uint32_t cnt = HAL_GetTick();
      uint32_t t1, t2; 
      t1 = DWT_GetCount();
      process((int16_t*) buf0);
      // uint32_t newcnt = HAL_GetTick();
      // uint32_t diff = newcnt - cnt;
      t2 = DWT_GetCount() - t1;
      t1 = 0;
    }
    if (buf1_rdy) {
      buf1_rdy = 0; 
      process((int16_t*) buf1);
    }

    // HAL_Delay(500);
    LED_Toggle(LED2_PIN);
  }
}



void process(int16_t *buf)
{
  uint32_t t1, t2;  
  t1 = DWT_GetCount();
  // subtract away dc op point
  for (int i = 0; i < BUF_SIZE; i+=4) {
    buf[i] -= 2048;
    buf[i+1] -= 2048;
    buf[i+2] -= 2048;
    buf[i+3] -= 2048;
  }
  t2 = DWT_GetCount();
  uint32_t diff = t2 - t1;

  // calc power at 457 kHz
  float32_t gres = goertzel(buf);

  // save in buffer 
  goertzelbuf[goertzelbuf_pos] = gres;
  goertzelbuf_pos++; 
  if (goertzelbuf_pos == NWINDOWS) {
    goertzelbuf_pos = 0;
  }
}

