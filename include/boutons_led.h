#ifndef BOUTONS_LED_H
#define BOUTONS_LED_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    COULEUR_VERTE,
    COULEUR_JAUNE,
    COULEUR_BLEUE,
    COULEUR_ORANGE,
    COULEUR_ROUGE,
    COULEUR_BLANCHE,
    COULEUR_ETEINTE
} CouleurLed;

void BoutonsLed_Init(void);

bool BoutonRouge_Appuye(void);
bool BoutonVert_Appuye(void);

bool BoutonRouge_AppuiLong(void);
bool BoutonVert_AppuiLong(void);

void Led_Couleur(CouleurLed couleur);

#endif
