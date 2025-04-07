#include "main.h"
#include <stm32f7xx.h>

void DWT_Init(void)
{
  DWT->LAR = 0xC5ACCE55; 
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

uint32_t DWT_GetCount(void)
{
  return DWT->CYCCNT;
}
