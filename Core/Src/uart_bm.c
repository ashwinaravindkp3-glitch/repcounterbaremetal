#include <stddef.h>

#include "uart_bm.h"
#include "stm32f4xx.h"

/**
 * @brief Initialize USART2 on PA2 (TX) and PA3 (RX) for asynchronous communication.
 *
 * Register configuration summary:
 *  - RCC->AHB1ENR: enable GPIOA clock so we can program PA2/PA3.
 *  - RCC->APB1ENR: enable USART2 peripheral clock (USART2 resides on APB1 = 42 MHz).
 *  - GPIOA->MODER: set PA2/PA3 to alternate function mode (10b) to route USART signals.
 *  - GPIOA->AFR[0]: select AF7 for PA2/PA3, which corresponds to USART2.
 *  - GPIOA->OSPEEDR: set high speed for clean edges on TX, OTYPER: push-pull outputs.
 *  - GPIOA->PUPDR: apply pull-up on RX to guarantee a defined idle state.
 *  - USART2->BRR: program baud rate divider using APB1 clock / baud (oversampling by 16).
 *  - USART2->CR1/CR2/CR3: reset to defaults, then enable transmitter, receiver, and USART (8N1).
 */
void UART2_Init(uint32_t baudrate)
{
    /* Enable GPIOA clock. */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* Enable USART2 clock on APB1. */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* Configure PA2/PA3 as alternate function AF7 (USART2). */
    GPIOA->MODER &= ~((3U << (2U * 2U)) | (3U << (3U * 2U))); /* Clear mode bits. */
    GPIOA->MODER |=  (2U << (2U * 2U)) | (2U << (3U * 2U));   /* 10b = alternate function. */

    GPIOA->AFR[0] &= ~((0xFU << (2U * 4U)) | (0xFU << (3U * 4U)));
    GPIOA->AFR[0] |=  (7U << (2U * 4U)) | (7U << (3U * 4U));      /* AF7 = USART2. */

    GPIOA->OSPEEDR |= (3U << (2U * 2U)) | (3U << (3U * 2U));      /* Very high speed. */
    GPIOA->OTYPER  &= ~((1U << 2U) | (1U << 3U));                 /* Push-pull for TX/RX. */
    GPIOA->PUPDR   &= ~((3U << (2U * 2U)) | (3U << (3U * 2U)));
    GPIOA->PUPDR   |=  (1U << (3U * 2U));                         /* Enable pull-up on RX (PA3). */

    /* Make sure USART is disabled before (re-)configuring. */
    USART2->CR1 &= ~USART_CR1_UE;

    /* Reset configuration registers to known state (8N1, no flow control). */
    USART2->CR1 = 0U;
    USART2->CR2 = 0U;
    USART2->CR3 = 0U;

    /* Compute USARTDIV for oversampling by 16: BRR = fCK / baud. */
    const uint32_t apb1_clk = 42000000UL; /* From Stage 1 clock configuration. */
    USART2->BRR = (apb1_clk + (baudrate / 2U)) / baudrate; /* Rounded for better accuracy. */

    /* Configure control register 1: enable RX, TX, and USART module. */
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART2->CR1 |= USART_CR1_UE;

    /* Optional: clear status register bits by reading SR then DR. */
    (void)USART2->SR;
    (void)USART2->DR;
}

/**
 * @brief Blocking send of a single character.
 *
 * Uses USART2->SR TXE (bit 7) to ensure the transmit data register is empty before writing DR.
 */
void UART2_SendChar(char c)
{
    while ((USART2->SR & USART_SR_TXE) == 0U)
    {
        /* Wait until transmit data register is empty. */
    }
    USART2->DR = (uint8_t)c;
}

/**
 * @brief Send a null-terminated string.
 */
void UART2_SendString(const char *str)
{
    if (str == NULL)
    {
        return;
    }

    while (*str != '\0')
    {
        UART2_SendChar(*str++);
    }
}

/**
 * @brief Blocking receive of a single character.
 *
 * Waits on USART2->SR RXNE (bit 5) before reading USART2->DR.
 */
char UART2_ReadChar(void)
{
    while ((USART2->SR & USART_SR_RXNE) == 0U)
    {
        /* Wait until a byte has been received. */
    }
    return (char)(USART2->DR & 0xFFU);
}

/**
 * @brief Check if RX buffer has unread data.
 *
 * @return 1 if RXNE is set, else 0.
 */
uint8_t UART2_IsRxReady(void)
{
    return (USART2->SR & USART_SR_RXNE) ? 1U : 0U;
}
