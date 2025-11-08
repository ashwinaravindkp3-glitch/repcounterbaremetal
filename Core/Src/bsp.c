#include "bsp.h"
#include "stm32f4xx.h"

/**
 * @brief Configure system clock to 84 MHz from the 16 MHz HSI.
 *
 * Steps:
 *  - Ensure HSI oscillator is enabled and ready (RCC->CR HSION/HSIRDY).
 *  - Configure Flash wait states to 2 WS to support 84 MHz (FLASH->ACR LATENCY).
 *  - Disable PLL before configuration (RCC->CR PLLON).
 *  - Program PLL factors: PLLM=16, PLLN=336, PLLP=4 (RCC->PLLCFGR).
 *  - Set bus prescalers: AHB=1, APB1=2, APB2=1 (RCC->CFGR HPRE/PPRE1/PPRE2).
 *  - Enable PLL and wait for lock (RCC->CR PLLRDY).
 *  - Switch SYSCLK source to PLL (RCC->CFGR SW) and wait for confirmation (SWS).
 */
void BSP_SystemClockConfig(void)
{
    /* 1. Ensure HSI is ON (it is by default, but set to be safe). */
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0U)
    {
        /* Wait until the internal oscillator is stable. */
    }

    /* 2. Configure Flash latency and enable instruction/data cache. */
    FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_2WS;

    /* 3. Disable PLL before reconfiguration. */
    if ((RCC->CR & RCC_CR_PLLON) != 0U)
    {
        RCC->CR &= ~RCC_CR_PLLON;
        while ((RCC->CR & RCC_CR_PLLRDY) != 0U)
        {
            /* Wait until PLL is fully stopped. */
        }
    }

    /* 4. Configure PLL: VCO = (HSI / PLLM) * PLLN = (16MHz /16)*336 = 336 MHz. */
    /*    SYSCLK = VCO / PLLP = 336 / 4 = 84 MHz. */
    /*    Set PLL source to HSI, PLLM=16, PLLN=336, PLLP=4 (bits 17:16 = 01 for /4). */
    const uint32_t pll_m = 16U;
    const uint32_t pll_n = 336U;
    const uint32_t pll_q = 7U;  /* 336 / 7 = 48 MHz for USB-style peripherals. */

    RCC->PLLCFGR = (pll_m << RCC_PLLCFGR_PLLM_Pos) |
                   (pll_n << RCC_PLLCFGR_PLLN_Pos) |
                   (1U << RCC_PLLCFGR_PLLP_Pos) |
                   (pll_q << RCC_PLLCFGR_PLLQ_Pos) |
                   RCC_PLLCFGR_PLLSRC_HSI;

    /* 5. Configure AHB and APB prescalers: APB1 must be <=42 MHz. */
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2; /* APB1 = 84/2 = 42 MHz. */
    /* HPRE defaults to 1, PPRE2 defaults to 1, so no extra bits needed. */

    /* 6. Enable PLL and wait for lock. */
    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0U)
    {
        /* Wait until PLL locks. */
    }

    /* 7. Select PLL as system clock source. */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
    {
        /* Wait until PLL is used as the system clock. */
    }

    /* Core now runs at 84 MHz (AHB), APB1 at 42 MHz, APB2 at 84 MHz. */

}
