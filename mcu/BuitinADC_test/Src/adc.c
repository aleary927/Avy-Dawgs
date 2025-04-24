#include "main.h"
#include "adc.h"
#include <stm32f7xx_ll_adc.h>
#include <stm32f7xx_ll_dma.h>
#include "globals.h"
#include "stm32f722xx.h"
#include "stm32f7xx_hal_rcc_ex.h"

volatile int16_t inbufx[BUF_SIZE]; 
volatile int16_t inbufy[BUF_SIZE]; 
volatile int inbufx_rdy = 0; 
volatile int inbufy_rdy = 0;


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
  // configure common features of ADCs
  if (LL_ADC_CommonInit(ADC, &adc_commoninit)) {
    // error
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
    // error
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
    // error
  }
  if (LL_ADC_REG_Init(ADCx, &lladc_reginit)) { // initialize regular group
    // error
  }
  LL_ADC_REG_SetSequencerRanks(ADCx, LL_ADC_REG_RANK_1, ADC_Channel);        // configure sequencer rank
  LL_ADC_SetChannelSamplingTime(ADCx, ADC_Channel, LL_ADC_SAMPLINGTIME_3CYCLES);  // configure sampling time
  LL_ADC_Enable(ADCx);            // enable ADC
}

void ADC_SysTick_Handler(void)
{
  static uint32_t count_ms = 0;
  count_ms++; 
  // init ADC burst every period
  if (count_ms == BURST_PERIOD_MS) {
    count_ms = 0;
    // make sure configuration is complete
    if (config_cplt) {
      // it is necessary to invalidate cache here so that changes in cache are not written 
      // through at a later time
      SCB_InvalidateDCache_by_Addr((uint32_t*)(((uint32_t)inbufx) & ~(uint32_t)0x1F), BUF_SIZE*2+32);
      SCB_InvalidateDCache_by_Addr((uint32_t*)(((uint32_t)inbufy) & ~(uint32_t)0x1F), BUF_SIZE*2+32);

      // recover from overrun errors in ADCs
      if (LL_ADC_IsActiveFlag_OVR(ADC1)) {
        LL_ADC_ClearFlag_OVR(ADC1);
      }
      if (LL_ADC_IsActiveFlag_OVR(ADC2)) {
        LL_ADC_ClearFlag_OVR(ADC2);
      }

      // streams must be diabled to load in number of data to transfer
      LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
      LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_3);
      while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_0) || LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_3));
      // enable x stream
      LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_0, (uint32_t) inbufx);
      LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, BUF_SIZE);
      LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);
      // enable y stream
      LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_3, (uint32_t) inbufy);
      LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_3, BUF_SIZE);
      LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_3);
      // start DMA in ADCs
      ADC1->CR2 |= (1 << 8); 
      ADC2->CR2 |= (1 << 8);
      // start ADC conversion
      LL_ADC_REG_StartConversionSWStart(ADC1);    
      LL_ADC_REG_StartConversionSWStart(ADC2);   
    }
  }
}

void ADC_DMA_Stream1_Handler(void)
{
  // if transfer complete
  if (LL_DMA_IsActiveFlag_TC0(DMA2)) {
    LL_DMA_ClearFlag_TC0(DMA2);

    // mark as ready
    inbufx_rdy = 1;
    ADC1->SR &= ~(1 << 4);  // stop ADC conversion
    ADC1->CR2 &= ~(1 << 8); // diable DMA in ADC
  }
  // clear half transfer complete flag
  if (LL_DMA_IsActiveFlag_HT0(DMA2)) {
    LL_DMA_ClearFlag_HT0(DMA2);
  }
}

void ADC_DMA_Stream2_Handler(void)
{
  // if transfer complete
  if (LL_DMA_IsActiveFlag_TC3(DMA2)) {
    LL_DMA_ClearFlag_TC3(DMA2);

    // mark as ready
    inbufy_rdy = 1;
    ADC2->SR &= ~(1 << 4); // stop ADC conversion
    ADC2->CR2 &= ~(1 << 8); // disable DMA in ADC
  }
  // clear half transfer complete flag
  if (LL_DMA_IsActiveFlag_HT3(DMA2)) {
    LL_DMA_ClearFlag_HT3(DMA2);
  }
}
