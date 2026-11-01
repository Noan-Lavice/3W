// i2c_bus.c
// Bus I2C1 : PB8 = SCL (horloge), PB9 = SDA (données).
// Plusieurs modules partagent les 2 fils, chacun a son adresse (RTC = 0x68, BME280 = 0x76).
#include "stm32l4xx_hal.h"
#include "i2c_bus.h"

I2C_HandleTypeDef hi2c1;

void i2c_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_I2C1_CLK_ENABLE();

    GPIO_InitStruct.Pin       = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;   // Open-drain : obligatoire pour l'I2C
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hi2c1.Instance             = I2C1;
    hi2c1.Init.Timing          = 0x10909CEC;   // 100 kHz avec une horloge de 80 MHz (valeur donnée par CubeMX)
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    HAL_I2C_Init(&hi2c1);
}

// "adresse << 1" : la HAL veut l'adresse décalée d'un bit vers la gauche
int i2c_lire(uint8_t adresse, uint8_t registre, uint8_t *donnees, int nombre) {
    if (HAL_I2C_Mem_Read(&hi2c1, adresse << 1, registre, I2C_MEMADD_SIZE_8BIT, donnees, nombre, 50) == HAL_OK) {
        return 1;
    }
    return 0;
}

int i2c_ecrire(uint8_t adresse, uint8_t registre, uint8_t *donnees, int nombre) {
    if (HAL_I2C_Mem_Write(&hi2c1, adresse << 1, registre, I2C_MEMADD_SIZE_8BIT, donnees, nombre, 50) == HAL_OK) {
        return 1;
    }
    return 0;
}
