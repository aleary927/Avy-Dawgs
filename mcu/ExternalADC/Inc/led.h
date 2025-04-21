#include "main.h"
#include "stm32f722xx.h"
#include <stdint.h>
#include <stm32f7xx_hal_gpio.h>
#include <stm32f7xx_hal_rcc.h>
#include <stm32f7xx_nucleo_144.h>

void LED_Init(void)
{
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitTypeDef gpiob_init; 
  gpiob_init.Pin = LED1_PIN | LED2_PIN | LED3_PIN; 
  gpiob_init.Mode = GPIO_MODE_OUTPUT_PP; 
  gpiob_init.Pull = GPIO_NOPULL;
  gpiob_init.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &gpiob_init);
}

void LED_Toggle(uint16_t Led_Pin)
{
  HAL_GPIO_TogglePin(GPIOB, Led_Pin);
}

void LED_Set(uint16_t Led_Pin)
{
  HAL_GPIO_WritePin(GPIOB, Led_Pin, GPIO_PIN_SET);
}

void LED_Reset(uint16_t Led_Pin)
{
  HAL_GPIO_WritePin(GPIOB, Led_Pin, GPIO_PIN_RESET);
}
