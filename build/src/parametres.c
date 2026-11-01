
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"
#include "parametres.h"
#include "rtc_ds1307.h"
#include "systeme.h"

// La Flash fait 1 Mo, découpée en pages de 2 Ko. On utilise la dernière page :
// la page 255 de la banque 2, qui commence à l'adresse 0x080FF800.
#define ADRESSE_FLASH   0x080FF800
#define PAGE_FLASH      255

// Les valeurs actuelles des paramètres
int parametre[NB_PARAMETRES];

// Le nom de chaque paramètre (dans le même ordre que dans parametres.h) :
// un tableau de 15 textes de 20 caractères maximum
static char nom[NB_PARAMETRES][20] = {
    "LOG_INTERVALL", "FILE_MAX_SIZE", "TIMEOUT",
    "LUMIN", "LUMIN_LOW", "LUMIN_HIGH",
    "TEMP_AIR", "MIN_TEMP_AIR", "MAX_TEMP_AIR",
    "HYGR", "HYGR_MINT", "HYGR_MAXT",
    "PRESSURE", "PRESSURE_MIN", "PRESSURE_MAX"
};

// Les valeurs par défaut du sujet
static int valeurParDefaut[NB_PARAMETRES] = {
    10, 2048, 30,
    1, 255, 768,
    1, -10, 60,
    1, 0, 50,
    1, 850, 1080
};

// Les bornes : valeur minimum et valeur maximum autorisées
static int minimum[NB_PARAMETRES] = {
    1, 512, 1,
    0, 0, 0,
    0, -40, -40,
    0, -40, -40,
    0, 300, 300
};
static int maximum[NB_PARAMETRES] = {
    1440, 32768, 300,
    1, 1023, 1023,
    1, 85, 85,
    1, 85, 85,
    1, 1100, 1100
};

// Lecture et écriture dans la Flash

// Lit le nombre rangé dans la Flash à l'adresse donnée
static int lire_flash(uint32_t adresse) {
    int *case_memoire = (int *)adresse;   // "À cette adresse, il y a un int"
    return *case_memoire;                 // On lit ce int
}

// Enregistre les paramètres dans la Flash. Deux règles de la Flash :
//  1) il faut effacer la page avant d'écrire dedans
//  2) on écrit par blocs de 8 octets (ici, un bloc pour chaque nombre)
static void SauvegardeEEPROM(void) {
    FLASH_EraseInitTypeDef effacement = {0};
    uint32_t erreur;

    HAL_FLASH_Unlock();   // La Flash est protégée contre l'écriture : on la déverrouille
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);   // On efface d'anciennes erreurs qui bloqueraient l'écriture

    // 1) On efface la page
    effacement.TypeErase = FLASH_TYPEERASE_PAGES;
    effacement.Banks     = FLASH_BANK_2;
    effacement.Page      = PAGE_FLASH;
    effacement.NbPages   = 1;
    HAL_FLASHEx_Erase(&effacement, &erreur);

    // 2) On écrit le numéro de version, puis chaque paramètre, dans des blocs de 8 octets
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ADRESSE_FLASH, EEPROM_VERSION);
    for (int i = 0; i < NB_PARAMETRES; i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ADRESSE_FLASH + 8 + i * 8, parametre[i]);
    }

    HAL_FLASH_Lock();   // On reverrouille la Flash
}

void RecuperationValeur(void) {
    for (int i = 0; i < NB_PARAMETRES; i++) {
        parametre[i] = lire_flash(ADRESSE_FLASH + 8 + i * 8);
    }
}

void InitialisationEEPROM(void) {
    for (int i = 0; i < NB_PARAMETRES; i++) {
        parametre[i] = valeurParDefaut[i];
    }
    SauvegardeEEPROM();
}

void VerificationEEPROM(void) {
    // Une Flash jamais écrite ne contient que des 0xFF : le numéro de version n'est pas le bon
    if (lire_flash(ADRESSE_FLASH) != EEPROM_VERSION) {
        printf("[PARAM] Premiere utilisation : valeurs par defaut\r\n");
        InitialisationEEPROM();
    } else {
        RecuperationValeur();
        printf("[PARAM] Parametres recuperes\r\n");
    }
}

void AffichageData(void) {
    printf("---- Parametres ----\r\n");
    for (int i = 0; i < NB_PARAMETRES; i++) {
        printf("%s = %d\r\n", nom[i], parametre[i]);
    }
    printf("--------------------\r\n");
}

// Vérifie que le texte est un nombre entier, par exemple "20" ou "-10"
static int est_un_nombre(char texte[]) {
    int i = 0;

    if (texte[0] == '-') {
        i = 1;   
    }
    if (texte[i] == '\0') {
        return 0;   // Texte vide
    }
    while (texte[i] != '\0') {
        if (texte[i] < '0' || texte[i] > '9') {
            return 0;   // Ce n'est pas un chiffre
        }
        i++;
    }
    return 1;
}

// Commandes de date et d'heure (horloge RTC)

// CLOCK=HH:MM:SS
static void commande_clock(char texte[]) {
    int heure, minute, seconde;

    if (sscanf(texte, "%d:%d:%d", &heure, &minute, &seconde) != 3) {
        printf("Format : CLOCK=HH:MM:SS\r\n");
        return;
    }
    if (heure < 0 || heure > 23 || minute < 0 || minute > 59 || seconde < 0 || seconde > 59) {
        printf("Erreur : heure impossible\r\n");
        return;
    }
    if (rtc_regler_heure(heure, minute, seconde) == 1) {
        printf("Heure reglee\r\n");
    } else {
        printf("Erreur : horloge RTC absente\r\n");
    }
}

// DATE=MOIS,JOUR,ANNEE
static void commande_date(char texte[]) {
    int mois, jour, annee;

    if (sscanf(texte, "%d,%d,%d", &mois, &jour, &annee) != 3) {
        printf("Format : DATE=MOIS,JOUR,ANNEE\r\n");
        return;
    }
    if (mois < 1 || mois > 12 || jour < 1 || jour > 31 || annee < 2000 || annee > 2099) {
        printf("Erreur : date impossible\r\n");
        return;
    }
    if (rtc_regler_date(mois, jour, annee) == 1) {
        printf("Date reglee\r\n");
    } else {
        printf("Erreur : horloge RTC absente\r\n");
    }
}

// DAY=MON (ou TUE, WED, THU, FRI, SAT, SUN)
static void commande_day(char texte[]) {
    char jours[7][4] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};   // 7 textes de 3 lettres (+ le '\0')

    for (int i = 0; i < 7; i++) {
        if (strcmp(texte, jours[i]) == 0) {
            if (rtc_regler_jour(i + 1) == 1) {   // 1 = lundi ... 7 = dimanche
                printf("Jour regle\r\n");
            } else {
                printf("Erreur : horloge RTC absente\r\n");
            }
            return;
        }
    }
    printf("Format : DAY=MON, TUE, WED, THU, FRI, SAT ou SUN\r\n");
}

// Traite une commande tapée en mode configuration
void TraiterCommande(char commande[]) {
    char nomCommande[32];   // Ce qui est avant le '=' (ex : "TIMEOUT")
    char texteValeur[32];   // Ce qui est après le '=' (ex : "20")
    int i = 0;
    int j = 0;

    // 1) On met tout en majuscules : "timeout=20" -> "TIMEOUT=20"
    for (i = 0; commande[i] != '\0'; i++) {
        commande[i] = toupper(commande[i]);
    }

    // 2) Les commandes sans '='
    if (strcmp(commande, "RESET") == 0) {
        InitialisationEEPROM();
        printf("Parametres remis aux valeurs par defaut\r\n");
        return;
    }
    if (strcmp(commande, "VERSION") == 0) {
        printf("Version : %s, lot : %s\r\n", VERSION_PROGRAMME, NUMERO_LOT);
        return;
    }
    if (strcmp(commande, "PILE") == 0) {   // Taille de la pile utilisée en ce moment
        printf("Pile utilisee : %lu octets\r\n", pile_utilisee());
        return;
    }
    if (strcmp(commande, "HELP") == 0) {
        AffichageData();
        printf("Autres commandes : RESET, VERSION, PILE, CLOCK=HH:MM:SS, DATE=MOIS,JOUR,ANNEE, DAY=MON, QUITTER\r\n");
        return;
    }

    // 3) On coupe "NOM=VALEUR" en deux textes : nomCommande et texteValeur
    i = 0;
    while (commande[i] != '=' && commande[i] != '\0' && i < 31) {
        nomCommande[i] = commande[i];
        i++;
    }
    nomCommande[i] = '\0';

    if (commande[i] != '=') {
        printf("Commande inconnue (tapez HELP)\r\n");
        return;
    }
    i++;   // On saute le '='

    j = 0;
    while (commande[i] != '\0' && j < 31) {
        texteValeur[j] = commande[i];
        i++;
        j++;
    }
    texteValeur[j] = '\0';

    // 4) La date et l'heure
    if (strcmp(nomCommande, "CLOCK") == 0) {
        commande_clock(texteValeur);
        return;
    }
    if (strcmp(nomCommande, "DATE") == 0) {
        commande_date(texteValeur);
        return;
    }
    if (strcmp(nomCommande, "DAY") == 0) {
        commande_day(texteValeur);
        return;
    }

    // Le sujet écrit aussi LOG_INTERVAL avec un seul L
    if (strcmp(nomCommande, "LOG_INTERVAL") == 0) {
        strcpy(nomCommande, "LOG_INTERVALL");
    }

    // 5) Les paramètres : on cherche le nom dans le tableau
    for (i = 0; i < NB_PARAMETRES; i++) {
        if (strcmp(nomCommande, nom[i]) == 0) {
            if (est_un_nombre(texteValeur) == 0) {
                printf("Erreur : la valeur doit etre un nombre entier\r\n");
                return;
            }
            int valeur = atoi(texteValeur);   // atoi transforme le texte en nombre : "20" -> 20
            if (valeur < minimum[i] || valeur > maximum[i]) {
                printf("Erreur : %s doit etre entre %d et %d\r\n", nom[i], minimum[i], maximum[i]);
                return;
            }
            parametre[i] = valeur;
            SauvegardeEEPROM();
            printf("%s = %d (enregistre)\r\n", nom[i], valeur);
            return;
        }
    }
    printf("Commande inconnue (tapez HELP)\r\n");
}
