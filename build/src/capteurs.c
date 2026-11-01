// capteurs.c
// Fait une mesure complète de tous les capteurs, avec les règles du sujet :
//  - un capteur qui ne répond pas avant TIMEOUT donne "NA"
//  - après 2 mesures de suite sans réponse, la LED signale l'erreur capteur
//  - une valeur en dehors des seuils MIN / MAX est signalée comme incohérente
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "capteurs.h"
#include "parametres.h"
#include "rtc_ds1307.h"
#include "bme280.h"
#include "luminosite.h"
#include "led.h"

// Variables globales (et pas locales) : elles ne prennent pas de place dans la pile
Mesure mesure;
char ligne[100];

static int echecsBME = 0;   // Nombre de mesures de suite où le BME280 n'a pas répondu

void capteurs_init(void) {
    if (rtc_init() == 1) {
        printf("[RTC] OK\r\n");
    } else {
        printf("[RTC] Horloge non detectee\r\n");
        led_erreur(ERREUR_RTC, 1);
    }

    if (bme280_init() == 1) {
        printf("[BME] OK\r\n");
    } else {
        printf("[BME] Capteur non detecte\r\n");
    }

    luminosite_init();
    printf("[LUM] OK\r\n");
}

// La date et l'heure
static void mesurer_date(void) {
    mesure.dateOK = rtc_lire(&mesure.annee, &mesure.mois, &mesure.jour,
                             &mesure.heure, &mesure.minute, &mesure.seconde);
    if (mesure.dateOK == 1) {
        led_erreur(ERREUR_RTC, 0);
    } else {
        led_erreur(ERREUR_RTC, 1);
    }
}

// Température, humidité, pression
static void mesurer_bme(void) {
    float t, h, p;
    int reponse = 0;
    uint32_t debut = HAL_GetTick();
    uint32_t timeout = parametre[TIMEOUT] * 1000;   // TIMEOUT est en secondes

    mesure.temperature = NA;
    mesure.humidite = NA;
    mesure.pression = NA;

    // On réessaie tant que le capteur n'a pas répondu, pendant au maximum TIMEOUT secondes
    while (reponse == 0 && HAL_GetTick() - debut < timeout) {
        if (bme280_lire(&t, &h, &p) == 1) {
            reponse = 1;
        } else {
            bme280_init();       // Le capteur a peut-être été rebranché : on le réveille
            led_mise_a_jour();   // Pendant l'attente, la LED continue de clignoter
            HAL_Delay(100);
        }
    }

    if (reponse == 0) {
        echecsBME++;
        if (echecsBME >= 2) {
            led_erreur(ERREUR_CAPTEUR, 1);
        }
        return;   // Les 3 valeurs restent à NA
    }
    echecsBME = 0;
    led_erreur(ERREUR_CAPTEUR, 0);

    // On garde seulement les mesures activées, et on vérifie les seuils
    int incoherent = 0;

    if (parametre[TEMP_AIR] == 1) {
        mesure.temperature = t;
        if (t < parametre[MIN_TEMP_AIR] || t > parametre[MAX_TEMP_AIR]) {
            incoherent = 1;
        }
    }
    if (parametre[HYGR] == 1) {
        // Le sujet : l'humidité ne compte que si la température est entre HYGR_MINT et HYGR_MAXT
        if (t >= parametre[HYGR_MINT] && t <= parametre[HYGR_MAXT]) {
            mesure.humidite = h;
        }
    }
    if (parametre[PRESSURE] == 1) {
        mesure.pression = p;
        if (p < parametre[PRESSURE_MIN] || p > parametre[PRESSURE_MAX]) {
            incoherent = 1;
        }
    }
    led_erreur(ERREUR_INCOHERENT, incoherent);
}

// La luminosité
static void mesurer_lumiere(void) {
    mesure.lumiere = NA;
    if (parametre[LUMIN] == 1) {
        int valeur = luminosite_lire();
        if (valeur >= 0) {
            mesure.lumiere = valeur;
        }
    }
}

void capteurs_mesurer(void) {
    mesurer_date();
    mesurer_bme();
    mesurer_lumiere();
}

// Ajoute une valeur à la fin de la ligne, avec un chiffre après la virgule, puis un ";"
static void ajouter_valeur(float valeur) {
    char texte[30];

    if (valeur == NA) {
        sprintf(texte, "NA;");
    } else {
        sprintf(texte, "%.1f;", valeur);
    }
    strcat(ligne, texte);   // strcat colle "texte" à la fin de "ligne"
}

// Transforme la mesure en texte : "2026-10-23;14:20:00;27.1;38.5;983.2;224;faible"
void capteurs_ligne(void) {
    char texte[30];

    // La date et l'heure
    if (mesure.dateOK == 1) {
        sprintf(ligne, "%04d-%02d-%02d;%02d:%02d:%02d;",
                mesure.annee, mesure.mois, mesure.jour, mesure.heure, mesure.minute, mesure.seconde);
    } else {
        sprintf(ligne, "NA;NA;");
    }

    // Température, humidité, pression
    ajouter_valeur(mesure.temperature);
    ajouter_valeur(mesure.humidite);
    ajouter_valeur(mesure.pression);

    // Luminosité : la valeur, puis faible / moyenne / forte selon les seuils
    if (mesure.lumiere == NA) {
        strcat(ligne, "NA;NA");
    } else {
        sprintf(texte, "%d;", mesure.lumiere);
        strcat(ligne, texte);
        if (mesure.lumiere < parametre[LUMIN_LOW]) {
            strcat(ligne, "faible");
        } else if (mesure.lumiere > parametre[LUMIN_HIGH]) {
            strcat(ligne, "forte");
        } else {
            strcat(ligne, "moyenne");
        }
    }
}
