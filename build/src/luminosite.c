
#include "stm32l4xx_hal.h"
#include "luminosite.h"

ADC_HandleTypeDef hadc1;

void luminosite_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    ADC_ChannelConfTypeDef canal = {0};

    // PA0 en mode analogique (on lit une tension, pas 0 ou 1)
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin  = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Réglages de l'ADC1 (ceux qu'on ne met pas restent à 0 = désactivés)
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_SYSCLK);
    hadc1.Instance                   = ADC1;
    hadc1.Init.ClockPrescaler        = ADC_CLOCK_ASYNC_DIV4;   // 80 MHz / 4 = 20 MHz
    hadc1.Init.Resolution            = ADC_RESOLUTION_10B;     // Résultat de 0 à 1023
    hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
    hadc1.Init.NbrOfConversion       = 1;
    hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;     // Lancée par le programme
    hadc1.Init.Overrun               = ADC_OVR_DATA_OVERWRITTEN;
    HAL_ADC_Init(&hadc1);

    // PA0 = entrée n°5 de l'ADC1
    canal.Channel      = ADC_CHANNEL_5;
    canal.Rank         = ADC_REGULAR_RANK_1;
    canal.SamplingTime = ADC_SAMPLETIME_92CYCLES_5;
    canal.SingleDiff   = ADC_SINGLE_ENDED;
    canal.OffsetNumber = ADC_OFFSET_NONE;
    HAL_ADC_ConfigChannel(&hadc1, &canal);

    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);   // L'ADC corrige ses petites erreurs
}

int luminosite_lire(void) {
    int valeur = -1;

    HAL_ADC_Start(&hadc1);   // Lance une conversion
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {   // Attend la fin (10 ms max)
        valeur = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    return valeur;
}
