#ifndef LED_H
#define LED_H

// Les couleurs
enum { ETEINT, ROUGE, VERT, BLEU, JAUNE, ORANGE, BLANC };

// Les erreurs, de la plus importante à la moins importante (ordre du sujet).
// Pas d'erreur GPS : on n'a pas de GPS.
enum { ERREUR_RTC, ERREUR_CAPTEUR, ERREUR_INCOHERENT, ERREUR_SD_PLEINE, ERREUR_SD_ACCES, NB_ERREURS };

void led_init(void);
void led_couleur_mode(int couleur);
void led_erreur(int erreur, int active);   // active = 1 : l'erreur commence, 0 : elle est finie
void led_mise_a_jour(void);                // À appeler à chaque tour de la boucle principale

#endif
