#include "main.h" 
#include <stdint.h>

extern volatile uint8_t tx_in_progress;

void UART_Config(void);
int UART_Transmit(const char *data);
