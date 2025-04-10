#include "main.h"
#include "adc.h"
#include <stm32f7xx_ll_adc.h>
#include <stm32f7xx_ll_dma.h>
#include "globals.h"
#include "stm32f722xx.h"
#include "stm32f7xx_hal_rcc_ex.h"

int16_t inbufx[BUF_SIZE]; 
int16_t inbufy[BUF_SIZE]; 


static void ADCx_DMAx_StreamConfig(ADC_TypeDef *ADCx, 
                            uint32_t ADC_Channel,
                            DMA_TypeDef *DMAx, 
                            uint32_t DMA_Stream, 
                            uint32_t DMA_Channel, 
                            IRQn_Type DMA_IRQn,
                            uint32_t *Buffer,
                            GPIO_TypeDef *GPIOx, 
                            uint32_t GPIO_Pin);

void ADC_DMA_Config(void) 
{
  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_ADC2_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  LL_ADC_CommonInitTypeDef adc_commoninit;
  adc_commoninit.Multimode = LL_ADC_MULTI_INDEPENDENT; 
  adc_commoninit.CommonClock = LL_ADC_CLOCK_SYNC_PCLK_DIV4; 
  adc_commoninit.MultiDMATransfer = LL_ADC_MULTI_REG_DMA_EACH_ADC;
  if (LL_ADC_CommonInit(ADC, &adc_commoninit)) {
    // while (1) {
    //
    // }
  }

  // stream 1 
  ADCx_DMAx_StreamConfig(ADC1, 
                         LL_ADC_CHANNEL_3,
                         DMA2, 
                         LL_DMA_STREAM_0, 
                         LL_DMA_CHANNEL_0,
                         DMA2_Stream0_IRQn,
                         (uint32_t*) inbufx,
                         GPIOA, 
                         GPIO_PIN_3
                         );

  // stream 2
  ADCx_DMAx_StreamConfig(ADC2, 
                         LL_ADC_CHANNEL_10, 
                         DMA2, 
                         LL_DMA_STREAM_3, 
                         LL_DMA_CHANNEL_1, 
                         DMA2_Stream3_IRQn, 
                         (uint32_t*) inbufy,
                         GPIOC, 
                         GPIO_PIN_0);



}

static void ADCx_DMAx_StreamConfig(ADC_TypeDef *ADCx, 
                            uint32_t ADC_Channel,
                            DMA_TypeDef *DMAx, 
                            uint32_t DMA_Stream, 
                            uint32_t DMA_Channel, 
                            IRQn_Type DMA_IRQn,
                            uint32_t *Buffer,
                            GPIO_TypeDef *GPIOx, 
                            uint32_t GPIO_Pin)
{
  // configure GPIO
  GPIO_InitTypeDef gpioa_init; 
  gpioa_init.Pin = GPIO_Pin; 
  gpioa_init.Mode = GPIO_MODE_ANALOG; 
  gpioa_init.Pull = GPIO_NOPULL; 
  HAL_GPIO_Init(GPIOx, &gpioa_init);

  // configure DMA
  LL_DMA_InitTypeDef lldma_init; 
  lldma_init.Mode = LL_DMA_MODE_NORMAL; 
  lldma_init.NbData = BUF_SIZE; 
  lldma_init.Channel = DMA_Channel; 
  lldma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE; 
  lldma_init.MemBurst = LL_DMA_MBURST_SINGLE; 
  lldma_init.Priority = LL_DMA_PRIORITY_HIGH;
  lldma_init.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  lldma_init.PeriphBurst = LL_DMA_PBURST_SINGLE;
  lldma_init.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_1_2;
  lldma_init.PeriphOrM2MSrcAddress = LL_ADC_DMA_GetRegAddr(ADCx, LL_ADC_DMA_REG_REGULAR_DATA);
  lldma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  lldma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
  lldma_init.MemoryOrM2MDstAddress = (uint32_t) Buffer;
  lldma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  lldma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
  LL_DMA_DisableStream(DMAx, DMA_Stream);
  while (LL_DMA_IsEnabledStream(DMAx, DMA_Stream));
  if (LL_DMA_Init(DMAx, DMA_Stream, &lldma_init)) { // initialize
    while (1) {

    }
  }
  LL_DMA_EnableIT_TC(DMAx, DMA_Stream);        // enable tranfer complete interrupt
  LL_DMA_EnableIT_HT(DMAx, DMA_Stream);
  LL_DMA_EnableIT_DME(DMAx, DMA_Stream);
  LL_DMA_EnableIT_FE(DMAx, DMA_Stream);
  LL_DMA_EnableIT_TE(DMAx, DMA_Stream);
  // LL_DMA_EnableStream(DMAx, DMA_Stream);       // enable the stream
  // enable interrupt in nvic
  HAL_NVIC_SetPriority(DMA_IRQn, 2, 2);
  HAL_NVIC_EnableIRQ(DMA_IRQn);
  
  // Confgigure ADC for one channel conversion
  LL_ADC_InitTypeDef lladc_init; 
  lladc_init.Resolution = LL_ADC_RESOLUTION_12B; 
  lladc_init.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT; 
  lladc_init.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
  LL_ADC_REG_InitTypeDef lladc_reginit; 
  lladc_reginit.DMATransfer = LL_ADC_REG_DMA_TRANSFER_LIMITED; 
  lladc_reginit.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE; 
  lladc_reginit.ContinuousMode = LL_ADC_REG_CONV_CONTINUOUS; 
  lladc_reginit.SequencerLength = LL_ADC_REG_SEQ_SCAN_DISABLE; 
  lladc_reginit.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_DISABLE;
  LL_ADC_Disable(ADCx);
  while (LL_ADC_IsEnabled(ADCx));
  if (LL_ADC_Init(ADCx, &lladc_init)) {     // initialize basic features
    while (1) {

    }
  }
  if (LL_ADC_REG_Init(ADCx, &lladc_reginit)) { // initialize regular group
    while (1) {

    }
  }
  LL_ADC_REG_SetSequencerRanks(ADCx, LL_ADC_REG_RANK_1, ADC_Channel);        // configure sequencer rank
  LL_ADC_SetChannelSamplingTime(ADCx, ADC_Channel, LL_ADC_SAMPLINGTIME_3CYCLES);  // configure sampling time
  LL_ADC_Enable(ADCx);            // enable ADC
  // HAL_Delay(1);         // delay between enable and start of conversion
  // LL_ADC_REG_StartConversionSWStart(ADCx);    // start conversion 
}
