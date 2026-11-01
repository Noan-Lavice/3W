// systeme.h - Horloge à 80 MHz et mesure de la pile
#ifndef SYSTEME_H
#define SYSTEME_H

#include <stdint.h>


#define SRAM_END   0x20018000UL

void     SystemClock_Config(void);   // Horloge à 80 MHz
uint32_t pile_utilisee(void);        // Nombre d'octets utilisés dans la pile

#endif
