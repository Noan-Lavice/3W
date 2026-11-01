// main.c
// Projet Worldwide Weather Watcher - station météo embarquée
// Carte NUCLEO-L476RG, framework STM32Cube (HAL)
//
// Les 4 modes :
//   STANDARD      (LED verte)  : démarrage normal, mesures enregistrées
//   CONFIGURATION (LED jaune)  : bouton rouge tenu au démarrage, réglages par la console
//   MAINTENANCE   (LED orange) : rouge 5 s depuis standard, mesures affichées sans enregistrer
//   ECONOMIQUE    (LED bleue)  : vert 5 s depuis standard, mesures 2 fois moins souvent
//
// Ordre au démarrage : HAL_Init() -> SystemClock_Config() -> init des périphériques -> while (1)

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "systeme.h"
#include "console.h"
#include "i2c_bus.h"
#include "parametres.h"
#include "capteurs.h"
#include "stockage.h"
#include "led.h"
#include "boutons.h"

enum { STANDARD, CONFIGURATION, MAINTENANCE, ECONOMIQUE };

// Variables globales plutôt que locales : elles ne prennent pas de place dans la pile
int mode = STANDARD;
uint32_t derniereMesure = 0;   // Moment de la dernière mesure (en ms)
int mesureImmediate = 1;       // 1 = faire une mesure tout de suite (en arrivant dans un mode)
uint32_t derniereTouche = 0;   // Moment de la dernière touche tapée en mode configuration

char commande[64];             // Ce qui est tapé dans la console
int longueurCommande = 0;      // Nombre de lettres déjà tapées

void changer_mode(int nouveauMode) {
    mode = nouveauMode;

    if (mode == STANDARD) {
        led_couleur_mode(VERT);
        printf("\r\n===== Mode STANDARD =====\r\n");
    }
    if (mode == CONFIGURATION) {
        led_couleur_mode(JAUNE);
        printf("\r\n===== Mode CONFIGURATION =====\r\n");
        AffichageData();
        printf("Tapez une commande (ex : LOG_INTERVALL=10) ou QUITTER\r\n");
        derniereTouche = HAL_GetTick();
    }
    if (mode == MAINTENANCE) {
        led_couleur_mode(ORANGE);
        printf("\r\n===== Mode MAINTENANCE =====\r\n");
        printf("La carte SD peut etre retiree\r\n");
    }
    if (mode == ECONOMIQUE) {
        led_couleur_mode(BLEU);
        printf("\r\n===== Mode ECONOMIQUE =====\r\n");
    }

    mesureImmediate = 1;       // Une mesure tout de suite dans le nouveau mode
    boutons_reinitialiser();   // Le bouton qu'on tient encore ne doit pas rechanger le mode
}

// Temps entre deux mesures, en ms
uint32_t intervalle_mesure(void) {
    uint32_t intervalle;

    if (MODE_DEMO == 1) {
        intervalle = parametre[LOG_INTERVALL] * 1000;    // En secondes pour la démo
    } else {
        intervalle = parametre[LOG_INTERVALL] * 60000;   // En minutes, comme le sujet
    }
    if (mode == ECONOMIQUE) {
        intervalle = intervalle * 2;   // 2 fois moins souvent
    }
    return intervalle;
}

// Renvoie 1 si c'est l'heure de faire une mesure
int temps_de_mesurer(uint32_t intervalle) {
    if (mesureImmediate == 1 || HAL_GetTick() - derniereMesure >= intervalle) {
        mesureImmediate = 0;
        derniereMesure = HAL_GetTick();
        return 1;
    }
    return 0;
}

// Fait une mesure, l'affiche, et l'enregistre si enregistrer = 1
void mesurer(int enregistrer) {
    capteurs_mesurer();   // Remplit la variable globale "mesure" (capteurs.c)
    capteurs_ligne();     // Remplit la variable globale "ligne" (capteurs.c)
    printf("%s\r\n", ligne);

    if (enregistrer == 1) {
        stockage_ajouter();
    }
}

// Mode configuration : on lit ce qui est tapé dans la console, lettre par lettre
void gerer_configuration(void) {
    char c;

    while (console_lire_caractere(&c) == 1) {
        derniereTouche = HAL_GetTick();

        if (c == '\r' || c == '\n') {
            // Touche Entrée : la commande est finie (une ligne vide est ignorée)
            if (longueurCommande > 0) {
                commande[longueurCommande] = '\0';   // Fin du texte
                longueurCommande = 0;
                printf("> %s\r\n", commande);

                if (strcmp(commande, "QUITTER") == 0 || strcmp(commande, "quitter") == 0) {
                    changer_mode(STANDARD);
                    return;
                }
                TraiterCommande(commande);
            }
        } else if (longueurCommande < 63) {
            // Une lettre de plus (on garde une case pour le '\0')
            commande[longueurCommande] = c;
            longueurCommande++;
        }
    }

    // 30 minutes sans rien taper : retour au mode standard
    if (HAL_GetTick() - derniereTouche >= DUREE_INACTIVITE) {
        printf("30 minutes sans activite : retour au mode standard\r\n");
        changer_mode(STANDARD);
    }
}

int main(void) {
    HAL_Init();              // En premier : prépare la HAL et le tic de 1 ms (SysTick)
    SystemClock_Config();    // Horloge à 80 MHz

    boutons_init();
    int rougeAppuye = bouton_rouge_au_demarrage();   // Rouge tenu au démarrage ?
    led_init();
    console_init();
    HAL_Delay(500);          // Laisse le temps au moniteur série du PC de se connecter

    printf("\r\n===== WORLDWIDE WEATHER WATCHER - version %s =====\r\n", VERSION_PROGRAMME);

    i2c_init();              // Avant les capteurs, qui l'utilisent
    VerificationEEPROM();    // Charge les paramètres
    capteurs_init();
    stockage_init();
    printf("Pile utilisee apres l'initialisation : %lu octets\r\n", pile_utilisee());
    printf("%s\r\n", ENTETE);

    if (rougeAppuye == 1) {
        changer_mode(CONFIGURATION);
    } else {
        changer_mode(STANDARD);
    }

    // Boucle principale : elle ne s'arrête jamais
    while (1) {
        boutons_mise_a_jour();
        led_mise_a_jour();

        if (mode == CONFIGURATION) {
            // Les boutons ne servent pas dans ce mode : on oublie leurs appuis longs
            bouton_appui_long(BOUTON_ROUGE);
            bouton_appui_long(BOUTON_VERT);
            gerer_configuration();   // Pas de mesure dans ce mode
        }
        else if (mode == STANDARD) {
            if (bouton_appui_long(BOUTON_ROUGE) == 1) {
                changer_mode(MAINTENANCE);
            } else if (bouton_appui_long(BOUTON_VERT) == 1) {
                changer_mode(ECONOMIQUE);
            } else if (temps_de_mesurer(intervalle_mesure()) == 1) {
                mesurer(1);
            }
        }
        else if (mode == ECONOMIQUE) {
            if (bouton_appui_long(BOUTON_ROUGE) == 1) {
                changer_mode(STANDARD);
            } else {
                bouton_appui_long(BOUTON_VERT);   // Le vert ne fait rien ici
                if (temps_de_mesurer(intervalle_mesure()) == 1) {
                    mesurer(1);
                }
            }
        }
        else if (mode == MAINTENANCE) {
            if (bouton_appui_long(BOUTON_ROUGE) == 1) {
                changer_mode(STANDARD);
            } else {
                bouton_appui_long(BOUTON_VERT);   // Le vert ne fait rien ici
                if (temps_de_mesurer(PERIODE_MAINTENANCE) == 1) {
                    mesurer(0);   // Pas d'enregistrement : la carte SD peut être retirée
                }
            }
        }
    }
}
