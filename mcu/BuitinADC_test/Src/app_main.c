#include "main.h"
#include "led.h"
#include "stm32f7xx_nucleo_144.h"
#include "adc.h"

void app_main(void)
{
  LED_Init();

  ADC_StreamConfig();

  while (1) {
    HAL_Delay(500);
    LED_Toggle(LED2_PIN);
  }

}
