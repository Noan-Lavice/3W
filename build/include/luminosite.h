// luminosite.h - Capteur de luminosité analogique sur A0 (PA0)
#ifndef LUMINOSITE_H
#define LUMINOSITE_H

void luminosite_init(void);
int  luminosite_lire(void);   // 0 (noir) à 1023 (très lumineux), -1 si erreur

#endif
