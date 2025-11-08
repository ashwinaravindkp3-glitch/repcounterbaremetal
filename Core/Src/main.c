#include "bsp.h"
#include "uart_bm.h"

int main(void)
{
    BSP_SystemClockConfig();
    UART2_Init(115200U);

    UART2_SendString("\r\n=== Stage 1: UART2 Bare-Metal Test ===\r\n");
    UART2_SendString("Type characters to echo them back.\r\n");

    while (1)
    {
        if (UART2_IsRxReady())
        {
            char c = UART2_ReadChar();
            UART2_SendChar(c);
        }
    }
}
