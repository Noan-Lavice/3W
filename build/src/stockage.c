// stockage.c
// Enregistrement des mesures, avec les règles du sujet :
//  - nom du fichier AAMMJJ_R.LOG (ex : 261023_0.LOG pour le 23/10/2026)
//  - on écrit toujours dans la révision 0
//  - quand elle dépasse FILE_MAX_SIZE, elle est archivée en révision 1, 2, ...
//    et la révision 0 recommence vide
//
// Notre carte SD n'est pas câblée (il manque les 3 fils ICSP) : on fait "comme si".
// Le programme retient le nom et la taille du fichier, et affiche ce qu'il écrirait.
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "stockage.h"
#include "capteurs.h"
#include "parametres.h"
#include "led.h"

#define REVISION_MAX   9   // Nom limité à 8 caractères : "AAMMJJ_R", donc R va de 0 à 9

static char jourActuel[20] = "";   // Jour du fichier en cours, ex : "261023"
static int tailleFichier = 0;      // Taille qu'il aurait sur la carte, en octets
static int revision = 0;           // Dernière révision archivée

void stockage_init(void) {
    printf("[SD] Carte SD simulee : les lignes sont affichees dans la console\r\n");
}

void stockage_ajouter(void) {
    char jour[20];
    int longueur = strlen(ligne) + 2;   // + 2 pour le retour à la ligne "\r\n"

    // Le jour, pour le nom du fichier : 2026-10-23 -> "261023"
    if (mesure.dateOK == 1) {
        sprintf(jour, "%02d%02d%02d", mesure.annee % 100, mesure.mois, mesure.jour);
    } else {
        sprintf(jour, "000000");   // Pas de date
    }

    // Nouveau jour = nouveau fichier
    if (strcmp(jour, jourActuel) != 0) {
        strcpy(jourActuel, jour);
        tailleFichier = strlen(ENTETE) + 2;   // Un fichier commence par la ligne des titres
        revision = 0;
    }

    // Fichier plein : on l'archive sous la révision suivante et la révision 0 recommence
    if (tailleFichier + longueur > parametre[FILE_MAX_SIZE]) {
        if (revision == REVISION_MAX) {
            printf("[SD] Plus de revision disponible aujourd'hui\r\n");
            led_erreur(ERREUR_SD_PLEINE, 1);
            return;
        }
        revision++;
        printf("[SD] Fichier plein, archive sous %s_%d.LOG\r\n", jour, revision);
        tailleFichier = strlen(ENTETE) + 2;
    }

    tailleFichier = tailleFichier + longueur;
    led_erreur(ERREUR_SD_PLEINE, 0);   // La ligne a pu être ajoutée : la carte n'est pas pleine
    printf("  -> [SD simulee] ajoute a %s_0.LOG (%d / %d octets)\r\n", jour, tailleFichier, parametre[FILE_MAX_SIZE]);
}
