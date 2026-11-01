// console.c
// Console série : l'UART2 (PA2 = TX, PA3 = RX) est reliée au ST-Link,
// qui la transmet au PC par le câble USB.
#include <stdio.h>
#include "stm32l4xx_hal.h"
#include "console.h"

UART_HandleTypeDef huart2;   // L'UART2 (utilisée par toutes les fonctions de ce fichier)

void console_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PA2 et PA3 sont confiées à l'UART2 (fonction alternative n°7)
    GPIO_InitStruct.Pin       = GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 115200 bauds, 8 bits, pas de parité, 1 bit de stop
    huart2.Instance        = USART2;
    huart2.Init.BaudRate   = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits   = UART_STOPBITS_1;
    huart2.Init.Parity     = UART_PARITY_NONE;
    huart2.Init.Mode       = UART_MODE_TX_RX;
    HAL_UART_Init(&huart2);

    setvbuf(stdout, NULL, _IONBF, 0);   // printf envoie le texte tout de suite (pas de mémoire tampon)
}

// printf() prépare le texte, puis appelle cette fonction pour l'envoyer : on l'envoie sur l'UART2
int _write(int fichier, char *texte, int longueur) {
    HAL_UART_Transmit(&huart2, (uint8_t *)texte, longueur, 100);
    return longueur;
}

// Regarde si une touche a été tapée sur le PC, sans attendre
int console_lire_caractere(char *c) {
    __HAL_UART_CLEAR_OREFLAG(&huart2);   // Efface l'erreur "lettre perdue", sinon l'UART se bloque
    if (HAL_UART_Receive(&huart2, (uint8_t *)c, 1, 0) == HAL_OK) {   // 0 ms : on n'attend pas
        return 1;
    }
    return 0;
}
