#include "main.h"
#include "stm32f722xx.h"
#include "stm32f7xx_hal_cortex.h"
#include "stm32f7xx_hal_gpio_ex.h"
#include "stm32f7xx_hal_rcc_ex.h"
#include <stm32f7xx_ll_usart.h>
#include <stm32f7xx_ll_gpio.h>
#include <stm32f7xx_ll_bus.h>
#include <stm32f7xx_ll_rcc.h>
#include <stm32f7xx_hal_gpio.h>
#include "UART.h"

#define UART USART6

volatile uint8_t tx_in_progress = 0;
volatile uint8_t *tx_buffer = NULL;
volatile uint32_t tx_len = 0;
volatile uint32_t tx_index = 0;

void UART_Init(USART_TypeDef *USARTx);
uint32_t strlen(const char *s);


/*
* Confiug UART and GPIO.
*/
void UART_Config(void) 
  {
    __HAL_RCC_USART6_CLK_ENABLE(); 
    __HAL_RCC_GPIOG_CLK_ENABLE(); 
    
    GPIO_InitTypeDef gpio_init; 
    gpio_init.Pin = GPIO_PIN_14; 
    gpio_init.Mode = GPIO_MODE_AF_OD; 
    gpio_init.Pull = GPIO_NOPULL; 
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH; 
    gpio_init.Alternate = GPIO_AF8_USART6;
    HAL_GPIO_Init(GPIOG, &gpio_init);
    UART_Init(UART);

    HAL_NVIC_SetPriority(USART6_IRQn, 3, 3);
    HAL_NVIC_EnableIRQ(USART6_IRQn); 
}

/*
* Initialize a UART.
*/
void UART_Init(USART_TypeDef *USARTx){

    LL_USART_InitTypeDef usart_init; 
    usart_init.BaudRate = 115200;
    usart_init.DataWidth = LL_USART_DATAWIDTH_8B;
    usart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    usart_init.OverSampling = LL_USART_OVERSAMPLING_16;
    usart_init.Parity = LL_USART_PARITY_NONE;
    usart_init.StopBits = LL_USART_STOPBITS_1;
    usart_init.TransferDirection = LL_USART_DIRECTION_TX_RX;

    LL_USART_Disable(USARTx);
    while(LL_USART_IsEnabled(USARTx));

    LL_USART_ConfigAsyncMode(USARTx);
    LL_USART_SetBaudRate(USARTx, 108000000, LL_USART_OVERSAMPLING_16, 115200);
    LL_USART_SetDataWidth(USARTx, LL_USART_DATAWIDTH_8B);
    LL_USART_SetStopBitsLength(USARTx, LL_USART_STOPBITS_1);

    LL_USART_EnableIT_TC(USARTx); 

    LL_USART_EnableDirectionTx(USARTx);
    LL_USART_Enable(USARTx);
}

// String length function
uint32_t strlen(const char *s) {
    uint32_t len = 0;
    while (s[len] != '\0') len++;
    return len;
}

// Transmit Function
int UART_Transmit(const char *data){
    if (tx_in_progress) return -1; // Return error if busy

    tx_buffer = (uint8_t *)data;
    tx_len = strlen(data);
    tx_index = 0;
    tx_in_progress = 1;

    LL_USART_TransmitData8(UART, tx_buffer[0]);  // Start first byte
    tx_index++;
    return 0;
}

// Interrupt Handler
void USART6_IRQHandler(void){
    if (LL_USART_IsActiveFlag_TC(UART)) {
        LL_USART_ClearFlag_TC(UART);

        if (tx_index < tx_len) {
            LL_USART_TransmitData8(UART, tx_buffer[tx_index]);
            tx_index++;
        } else {
            tx_in_progress = 0;
        }
    }
}
