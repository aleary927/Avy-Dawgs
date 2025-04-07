#include "main.h"
#include "adc.h"
#include <stm32f7xx_ll_adc.h>
#include <stm32f7xx_ll_dma.h>
#include "globals.h"

uint16_t buf0[BUF_SIZE]; 
uint16_t buf1[BUF_SIZE]; 

void ADC_DMA_StreamConfig(void) 
{
  // enable clocks
  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  // configure GPIO
  GPIO_InitTypeDef gpioa_init; 
  gpioa_init.Pin = GPIO_PIN_3; 
  gpioa_init.Mode = GPIO_MODE_ANALOG; 
  gpioa_init.Pull = GPIO_NOPULL; 
  HAL_GPIO_Init(GPIOA, &gpioa_init);

  // Configure DMA to double buffer mode
  LL_DMA_InitTypeDef lldma_init; 
  lldma_init.Mode = LL_DMA_MODE_CIRCULAR; 
  lldma_init.NbData = BUF_SIZE; 
  lldma_init.Channel = LL_DMA_CHANNEL_0; 
  lldma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE; 
  lldma_init.MemBurst = LL_DMA_MBURST_SINGLE; 
  lldma_init.Priority = LL_DMA_PRIORITY_HIGH;
  lldma_init.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  lldma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;
  lldma_init.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_1_2;
  lldma_init.PeriphOrM2MSrcAddress = LL_ADC_DMA_GetRegAddr(ADC1, LL_ADC_DMA_REG_REGULAR_DATA);
  lldma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  lldma_init.MemoryOrM2MDstAddress = (uint32_t) buf0;
  lldma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  lldma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
  lldma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
  LL_DMA_Init(DMA2, LL_DMA_STREAM_0, &lldma_init);  // initialize
  LL_DMA_SetMemory1Address(DMA2, LL_DMA_STREAM_0, (uint32_t) buf1); // set address for second buffer
  LL_DMA_EnableDoubleBufferMode(DMA2, LL_DMA_STREAM_0);   // enable double buffer mode
  LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);        // enable tranfer complete interrupt
  LL_DMA_EnableIT_HT(DMA2, LL_DMA_STREAM_0);
  LL_DMA_EnableIT_DME(DMA2, LL_DMA_STREAM_0);
  LL_DMA_EnableIT_FE(DMA2, LL_DMA_STREAM_0);
  LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_0);
  LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);       // enable the stream
  // enable interrupt in nvic
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 2, 2);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  
  // Confgigure ADC for one channel conversion
  LL_ADC_InitTypeDef lladc_init; 
  lladc_init.Resolution = LL_ADC_RESOLUTION_12B; 
  lladc_init.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT; 
  lladc_init.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  LL_ADC_REG_InitTypeDef lladc_reginit; 
  lladc_reginit.DMATransfer = LL_ADC_REG_DMA_TRANSFER_UNLIMITED; 
  lladc_reginit.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE; 
  lladc_reginit.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS; 
  lladc_reginit.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE; 
  lladc_reginit.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  LL_ADC_Init(ADC1, &lladc_init);     // initialize basic features
  LL_ADC_REG_Init(ADC1, &lladc_reginit);  // initialize regular group
  LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, LL_ADC_CHANNEL_3);        // configure sequencer rank
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_3, LL_ADC_SAMPLINGTIME_3CYCLES);  // configure sampling time
  LL_ADC_Enable(ADC1);            // enable ADC
  HAL_Delay(1);         // delay between enable and start of conversion
  LL_ADC_REG_StartConversionSWStart(ADC1);    // start conversion 
}
