#include "main.h" 
#include "ExtADC.h"
#include "stm32f722xx.h"
#include "stm32f7xx_hal_cortex.h"
#include "stm32f7xx_hal_gpio_ex.h"
#include "stm32f7xx_hal_rcc_ex.h"
#include "stm32f7xx_ll_spi.h"
#include "stm32f7xx_ll_dma.h"
#include "stm32f7xx_hal_gpio.h"
#include "globals.h"

#define SPIA SPI1
#define SPIA_DMA DMA2
#define SPIA_DMA_STREAM LL_DMA_STREAM_0
#define SPIA_DMA_CHANNEL LL_DMA_CHANNEL_3
#define SPIB SPI3
#define SPIB_DMA DMA1 
#define SPIB_DMA_STREAM LL_DMA_STREAM_0
#define SPIB_DMA_CHANNEL LL_DMA_CHANNEL_0

// config for two config registers
const uint16_t extadc_config[2] = {
  (0x1 << 15) | (0x1 << 12) | 0x0, // all default
  (0x1 << 15) | (0x2 << 12) | 0x0}; // all default

/* 
 * Pin mappings: 
 *
 * SPIA SCK: PA5 (CN7 10)
 * SPIA MISO: PA6 (CN7 12)
 * SPIA MOSI: PA7 (CN7 14)
 * SPIA NSS: PA15 (CN11 17)
 *
 * SPIB SCK: PB3 (CN7 15)
 * SPIB MISO: PB4 (CN7 19)
 * SPIB MOSI: PB5 (CN7 13)
 * SPIB NSS: PA4 (CN7 17)
 */

/*
* Configure SPI for communicating with external ADC.
*/
void ExtADC_Config(void) 
{
  // enable clocks
  __HAL_RCC_SPI1_CLK_ENABLE(); 
  __HAL_RCC_SPI3_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  // config one spi as master and one as slave
  // config gpio and dma for each

  /*****************************
   * SPI master (ADC A output and input) 
   * ****************************/

  // gpio for spia
  GPIO_InitTypeDef spia_gpio_init; 
  spia_gpio_init.Mode = GPIO_MODE_AF_PP; 
  spia_gpio_init.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_15; 
  spia_gpio_init.Pull = GPIO_NOPULL; 
  spia_gpio_init.Speed = GPIO_SPEED_FREQ_HIGH; 
  spia_gpio_init.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &spia_gpio_init);

  // dma for spia
  LL_DMA_InitTypeDef spia_dma_init; 
  spia_dma_init.Mode = LL_DMA_MODE_NORMAL;
  spia_dma_init.NbData = BUF_SIZE; 
  spia_dma_init.Channel = SPIA_DMA_CHANNEL; 
  spia_dma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE; 
  spia_dma_init.MemBurst = LL_DMA_MBURST_SINGLE; 
  spia_dma_init.Priority = LL_DMA_PRIORITY_HIGH; 
  spia_dma_init.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY; 
  spia_dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE; 
  spia_dma_init.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_1_2; 
  spia_dma_init.PeriphOrM2MSrcAddress = LL_SPI_DMA_GetRegAddr(SPIA); 
  spia_dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT; 
  spia_dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD; 
  spia_dma_init.MemoryOrM2MDstAddress = (uint32_t) extadc_config; 
  spia_dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT; 
  spia_dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
  LL_DMA_DisableStream(SPIA_DMA, SPIA_DMA_STREAM);
  while(LL_DMA_IsEnabledStream(SPIA_DMA, SPIA_DMA_STREAM));
  LL_DMA_Init(SPIA_DMA, SPIA_DMA_STREAM, &spia_dma_init);

  // spia init
  LL_SPI_InitTypeDef spia_init; 
  spia_init.Mode = LL_SPI_MODE_MASTER; 
  spia_init.NSS = LL_SPI_NSS_HARD_INPUT; 
  spia_init.CRCPoly = 0; 
  spia_init.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE; 
  spia_init.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2; 
  spia_init.BitOrder = LL_SPI_MSB_FIRST; 
  spia_init.DataWidth = LL_SPI_DATAWIDTH_16BIT; 
  spia_init.ClockPhase = LL_SPI_PHASE_1EDGE; 
  spia_init.ClockPolarity = LL_SPI_POLARITY_LOW; 
  spia_init.TransferDirection = LL_SPI_FULL_DUPLEX;
  LL_SPI_Disable(SPIA);
  while(LL_SPI_IsEnabled(SPIA));
  LL_SPI_Init(SPIA, &spia_init);

  /******************************  
   * SPI slave (ADC B output) 
   * ****************************/

  // gpio for spib
  GPIO_InitTypeDef spib_gpio_init; 
  spib_gpio_init.Mode = GPIO_MODE_AF_PP; 
  spib_gpio_init.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5; 
  spib_gpio_init.Pull = GPIO_NOPULL; 
  spib_gpio_init.Speed = GPIO_SPEED_FREQ_HIGH; 
  spib_gpio_init.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOB, &spib_gpio_init);
  spib_gpio_init.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOA, &spib_gpio_init);

  // dma for spib
  LL_DMA_InitTypeDef spib_dma_init; 
  spib_dma_init.Mode = LL_DMA_MODE_NORMAL;
  spib_dma_init.NbData = BUF_SIZE; 
  spib_dma_init.Channel = SPIA_DMA_CHANNEL; 
  spib_dma_init.FIFOMode = LL_DMA_FIFOMODE_DISABLE; 
  spib_dma_init.MemBurst = LL_DMA_MBURST_SINGLE; 
  spib_dma_init.Priority = LL_DMA_PRIORITY_HIGH; 
  spib_dma_init.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY; 
  spib_dma_init.PeriphBurst = LL_DMA_PBURST_SINGLE; 
  spib_dma_init.FIFOThreshold = LL_DMA_FIFOTHRESHOLD_1_2; 
  spib_dma_init.PeriphOrM2MSrcAddress = LL_SPI_DMA_GetRegAddr(SPIB); 
  spib_dma_init.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT; 
  spib_dma_init.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD; 
  spib_dma_init.MemoryOrM2MDstAddress = (uint32_t) inbufy; 
  spib_dma_init.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT; 
  spib_dma_init.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
  LL_DMA_DisableStream(SPIB_DMA, SPIB_DMA_STREAM);
  while(LL_DMA_IsEnabledStream(SPIB_DMA, SPIB_DMA_STREAM));
  LL_DMA_Init(SPIB_DMA, SPIB_DMA_STREAM, &spib_dma_init);

  // spib init
  LL_SPI_InitTypeDef spib_init; 
  spib_init.Mode = LL_SPI_MODE_SLAVE; 
  spib_init.NSS = LL_SPI_NSS_HARD_OUTPUT; 
  spib_init.CRCPoly = 0; 
  spib_init.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE; 
  spib_init.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2; 
  spib_init.BitOrder = LL_SPI_MSB_FIRST; 
  spib_init.DataWidth = LL_SPI_DATAWIDTH_14BIT; 
  spib_init.ClockPhase = LL_SPI_PHASE_1EDGE; 
  spib_init.ClockPolarity = LL_SPI_POLARITY_LOW; 
  spib_init.TransferDirection = LL_SPI_FULL_DUPLEX;
  LL_SPI_Disable(SPIB);
  while(LL_SPI_IsEnabled(SPIB));
  LL_SPI_Init(SPIB, &spib_init);

  // enable master, send configuration code 
  LL_SPI_Enable(SPIA);
  LL_SPI_WriteReg(SPIA, DR, 0);
  LL_SPI_TransmitData16(SPIA, extadc_config[0]);
  while(!LL_SPI_IsActiveFlag_TXE(SPIA));
  LL_SPI_TransmitData16(SPIA, extadc_config[1]);
  while(LL_SPI_IsActiveFlag_BSY(SPIA));
  
  // set SPIA to 14 bit
  LL_SPI_Disable(SPIA);
  while(LL_SPI_IsEnabled(SPIA));
  spia_init.DataWidth = LL_SPI_DATAWIDTH_14BIT;

  LL_DMA_EnableStream(SPIA_DMA, SPIA_DMA_STREAM);
  LL_DMA_EnableStream(SPIB_DMA, SPIB_DMA_STREAM);
  LL_SPI_EnableDMAReq_RX(SPIA); 
  LL_SPI_EnableDMAReq_RX(SPIB); 
  LL_SPI_Enable(SPIA); 
  LL_SPI_Enable(SPIB);

  LL_DMA_EnableIT_TC(SPIA_DMA, SPIA_DMA_STREAM);
  LL_DMA_EnableIT_TC(SPIB_DMA, SPIB_DMA_STREAM);

  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 1, 1);
  HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 1, 1);
  HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

/*
* Starts a transfer to fill input buffers.
*/
void ExtADC_StartTransfer(void) 
{
  // set DMA transfer size, memore address
  LL_DMA_SetDataLength(SPIA_DMA, SPIA_DMA_STREAM, BUF_SIZE);
  LL_DMA_SetDataLength(SPIB_DMA, SPIB_DMA_STREAM, BUF_SIZE);
  LL_DMA_SetMemoryAddress(SPIA_DMA, SPIA_DMA_STREAM, (uint32_t) inbufx);
  LL_DMA_SetMemoryAddress(SPIB_DMA, SPIB_DMA_STREAM, (uint32_t) inbufy);

  // enable dma 
  LL_DMA_EnableStream(SPIA_DMA, SPIA_DMA_STREAM);
  LL_DMA_EnableStream(SPIB_DMA, SPIB_DMA_STREAM);

  // enable spi
  LL_SPI_Enable(SPIA); 
  LL_SPI_Enable(SPIB); 

}

/*
* Ends transfer.
*/
void ExtADC_SPIA_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TC0(SPIA_DMA)) {

    // clear dma flags 
    LL_DMA_ClearFlag_TC0(SPIA_DMA);

    // clear SPIA flags

    // disable dma stream
    LL_DMA_DisableStream(SPIA_DMA, SPIA_DMA_STREAM);
    while(LL_DMA_IsEnabledStream(SPIA_DMA, SPIA_DMA_STREAM));
    // disable SPIA
    LL_SPI_Disable(SPIA);
    while(LL_SPI_IsEnabled(SPIA));

  }
}

/*
* Ends transfer.
*/
void ExtADC_SPIB_IRQHandler(void)
{
  if (LL_DMA_IsActiveFlag_TC0(SPIB_DMA)) {

    // clear dma flags 
    LL_DMA_ClearFlag_TC0(SPIB_DMA);
    // clear SPIB flags

    // disable dma stream 
    LL_DMA_DisableStream(SPIB_DMA, SPIB_DMA_STREAM);
    while(LL_DMA_IsEnabledStream(SPIB_DMA, SPIB_DMA_STREAM));
    // disable SPIB
    LL_SPI_Disable(SPIB); 
    while(LL_SPI_IsEnabled(SPIB));
  }

}
