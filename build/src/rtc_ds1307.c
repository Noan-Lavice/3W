// rtc_ds1307.c
// Horloge RTC DS1307 (I2C, adresse 0x68). Elle a une pile : elle compte même carte éteinte.
// La date est rangée dans 7 registres :
//   0 secondes, 1 minutes, 2 heures, 3 jour de la semaine, 4 jour, 5 mois, 6 année (2 chiffres)
// Les nombres sont en BCD : 45 est rangé 0x45 (le 4 à gauche, le 5 à droite).
#include "config.h"
#include "rtc_ds1307.h"
#include "i2c_bus.h"

// BCD -> nombre normal : 0x45 -> 4 x 10 + 5 = 45
static int bcd_vers_decimal(uint8_t valeur) {
    return (valeur / 16) * 10 + (valeur % 16);
}

// Nombre normal -> BCD : 45 -> 4 x 16 + 5 = 0x45
static uint8_t decimal_vers_bcd(int valeur) {
    return (valeur / 10) * 16 + (valeur % 10);
}

int rtc_lire(int *annee, int *mois, int *jour, int *heure, int *minute, int *seconde) {
    uint8_t registres[7];

    if (i2c_lire(ADRESSE_RTC, 0x00, registres, 7) == 0) {
        return 0;   // L'horloge ne répond pas
    }
    *seconde = bcd_vers_decimal(registres[0] & 0x7F);   // On enlève le bit 7 (horloge arrêtée)
    *minute  = bcd_vers_decimal(registres[1]);
    *heure   = bcd_vers_decimal(registres[2] & 0x3F);   // On enlève les bits du format 12 h
    *jour    = bcd_vers_decimal(registres[4]);
    *mois    = bcd_vers_decimal(registres[5]);
    *annee   = 2000 + bcd_vers_decimal(registres[6]);
    return 1;
}

// Écrit l'heure dans les registres 0, 1 et 2 (secondes, minutes, heures)
// Le bit 7 des secondes est à 0 : l'horloge tourne.
int rtc_regler_heure(int heure, int minute, int seconde) {
    uint8_t registres[3];

    registres[0] = decimal_vers_bcd(seconde);
    registres[1] = decimal_vers_bcd(minute);
    registres[2] = decimal_vers_bcd(heure);   // Format 24 h
    return i2c_ecrire(ADRESSE_RTC, 0x00, registres, 3);
}

// Écrit la date dans les registres 4, 5 et 6 (jour, mois, année)
int rtc_regler_date(int mois, int jour, int annee) {
    uint8_t registres[3];

    registres[0] = decimal_vers_bcd(jour);
    registres[1] = decimal_vers_bcd(mois);
    registres[2] = decimal_vers_bcd(annee - 2000);   // L'horloge ne garde que 2 chiffres
    return i2c_ecrire(ADRESSE_RTC, 0x04, registres, 3);
}

// Écrit le jour de la semaine dans le registre 3
int rtc_regler_jour(int jour) {
    uint8_t registre = jour;
    return i2c_ecrire(ADRESSE_RTC, 0x03, &registre, 1);
}

int rtc_init(void) {
    uint8_t secondes;

    if (i2c_lire(ADRESSE_RTC, 0x00, &secondes, 1) == 0) {
        return 0;   // L'horloge ne répond pas
    }
    // Bit 7 des secondes à 1 = horloge arrêtée (jamais réglée ou pile vide) :
    // on la met au jeudi 1er janvier 2026 à 00:00:00. On la règle ensuite avec CLOCK, DATE et DAY.
    if (secondes & 0x80) {
        rtc_regler_date(1, 1, 2026);
        rtc_regler_jour(4);
        rtc_regler_heure(0, 0, 0);   // Écrire les secondes remet le bit 7 à 0 : l'horloge démarre
    }
    return 1;
}
