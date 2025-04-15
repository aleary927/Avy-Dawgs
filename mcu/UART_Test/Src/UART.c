#include "main.h"
#include "stm32f722xx.h"
#include "stm32f7xx_hal_gpio_ex.h"
#include "stm32f7xx_hal_rcc_ex.h"
#include <stm32f7xx_ll_usart.h>
#include <stm32f7xx_ll_gpio.h>
#include <stm32f7xx_ll_bus.h>
#include <stm32f7xx_hal_gpio.h>

#define UART USART2

volatile uint8_t tx_in_progress = 0;
volatile uint8_t *tx_buffer = NULL;
volatile uint32_t tx_len = 0;
volatile uint32_t tx_index = 0;

void UART_Config(USART_TypeDef *USARTx);
int UART_Transmit(const char *data);
uint32_t simple_strlen(const char *s);
void USART2_IRQHandler(void);

// global flag (for interrupt based) to say whether transmit is in progress
// function to check if transmit is in progress
// interrupt handler to deal with the end of a transmit
// function to start a transmit
// interrupt handler clear a flag once transmit is done return error code if you call transmit while one is in progress

// Test function
void UART_Test(void){
    UART_Config(UART);

    while(1){
        if (!tx_in_progress) {
            UART_Transmit("Distance: 123, Direction: N\n");
            for (volatile int i = 0; i < 1000000; i++); // crude delay
        }
    }
}

// Uart config function
void UART_Config(USART_TypeDef *USARTx){

  __HAL_RCC_USART2_CLK_ENABLE(); 
  __HAL_RCC_GPIOD_CLK_ENABLE(); 
    
    GPIO_InitTypeDef gpio_init; 
    gpio_init.Pin = GPIO_PIN_5; 
    gpio_init.Mode = GPIO_MODE_AF_OD; 
    gpio_init.Pull = GPIO_NOPULL; 
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH; 
    gpio_init.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOD, &gpio_init);

    LL_USART_InitTypeDef usart_init; 
    usart_init.BaudRate = 115200;
    usart_init.DataWidth = LL_USART_DATAWIDTH_8B;
    usart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    usart_init.OverSampling = LL_USART_OVERSAMPLING_16;
    usart_init.Parity = LL_USART_PARITY_NONE;
    usart_init.StopBits = LL_USART_STOPBITS_1;
    usart_init.TransferDirection = LL_USART_DIRECTION_TX;

    LL_USART_ClockInitTypeDef usart_clock;
    usart_clock.ClockOutput = LL_USART_CLOCK_DISABLE;
    usart_clock.ClockPhase = 0;
    usart_clock.ClockPolarity = 0;

    LL_USART_Disable(USARTx);
    while(LL_USART_IsEnabled(USARTx));

    LL_USART_Init(USARTx, &usart_init);
    LL_USART_ClockInit(USARTx, &usart_clock);
}

// String length function
static uint32_t strlen(const char *s) {
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

    LL_USART_TransmitData8(UART, tx_buffer[tx_index++]);  // Start first byte
    return 0;
}

// Interrupt Handler
void USART2_IRQHandler(void){
    if (LL_USART_IsActiveFlag_TC(UART)) {
        LL_USART_ClearFlag_TC(UART);

        if (tx_index < tx_len) {
            LL_USART_TransmitData8(UART, tx_buffer[tx_index++]);
        } else {
            tx_in_progress = 0;
        }
    }
}

