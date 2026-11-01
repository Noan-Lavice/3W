// systeme.c
// Horloge à 80 MHz, mesure de la pile,
// et les fonctions appelées par le matériel (interruptions).
#include "stm32l4xx_hal.h"
#include "systeme.h"

// Horloge : HSI 16 MHz -> PLL -> 80 MHz
// Calcul : (16 MHz / M) x N / R = (16 / 1) x 10 / 2 = 80 MHz
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;   // Oscillateur interne 16 MHz
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 1;               // 16 / 1  = 16 MHz
    RCC_OscInitStruct.PLL.PLLN            = 10;              // 16 x 10 = 160 MHz
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV7;   // Sortie pas utilisée
    RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;   // Sortie pas utilisée
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV2;   // 160 / 2 = 80 MHz
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                     | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;   // Le processeur utilise la PLL
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;           // 80 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;             // 80 MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;             // 80 MHz
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);     // 4 cycles d'attente pour la Flash à 80 MHz
}

// Lit le registre SP (le sommet de la pile) avec une instruction en assembleur.
// "MOV %0, SP" copie SP dans la variable sp.
// inline : pas d'appel de fonction, donc la mesure ne change pas la pile.
static inline uint32_t get_sp(void) {
    uint32_t sp;
    __asm volatile ("MOV %0, SP" : "=r" (sp));
    return sp;
}

// La pile part de la fin de la RAM et descend : octets utilisés = SRAM_END - SP
uint32_t pile_utilisee(void) {
    return SRAM_END - get_sp();
}

// Fonctions appelées par le matériel (interruptions)

// Appelée toutes les 1 ms : fait avancer HAL_GetTick(). Sans elle, HAL_Delay() bloque.
void SysTick_Handler(void) {
    HAL_IncTick();
}

void NMI_Handler(void) {
}

// Erreur grave (par exemple la pile qui déborde) : on bloque ici
void HardFault_Handler(void) {
    while (1) {
    }
}
