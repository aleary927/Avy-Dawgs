#include "main.h"
#include "stm32f722xx.h"
#include <stm32f7xx_ll_usart.h>

void UART_Config(USART_TypeDef *USARTx);

void UART_Test(void){

    while(1){

    }
}

void UART_Config(USART_TypeDef *USARTx){
    LL_USART_InitTypeDef usart_init; 
    usart_init.BaudRate = 0;
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
    usart_clock.ClockPolarity = 0;

    LL_USART_Disable(USARTx);
    while(LL_USART_IsEnabled(USARTx));

    LL_USART_Init(USARTx, &usart_init);
    LL_USART_ClockInit(USARTx, &usart_clock);
}
