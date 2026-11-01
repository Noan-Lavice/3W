// systeme.h - Horloge à 80 MHz et mesure de la pile
#ifndef SYSTEME_H
#define SYSTEME_H

#include <stdint.h>

// Fin de la RAM = endroit où la pile commence (valeur de départ de SP)
// SRAM1 (64 Ko) + SRAM2 (32 Ko) = 96 Ko depuis 0x20000000 : 0x20000000 + 0x18000 = 0x20018000
#define SRAM_END   0x20018000UL

void     SystemClock_Config(void);   // Horloge à 80 MHz
uint32_t pile_utilisee(void);        // Nombre d'octets utilisés dans la pile

#endif
