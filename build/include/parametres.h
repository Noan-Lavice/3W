// parametres.h - Les paramètres du sujet, réglables en mode configuration.
// Ils sont gardés dans la Flash pour ne pas être perdus quand on éteint la carte
// (le STM32L476 n'a pas d'EEPROM).
#ifndef PARAMETRES_H
#define PARAMETRES_H

// Numéro enregistré dans la Flash avec les paramètres.
// À changer si on ajoute ou enlève un paramètre : les valeurs par défaut seront remises.
#define EEPROM_VERSION   4

// Le numéro de chaque paramètre dans le tableau "parametre"
enum {
    LOG_INTERVALL,   // Durée entre 2 mesures, en minute (en seconde si MODE_DEMO)
    FILE_MAX_SIZE,   // Taille max d'un fichier, en octets
    TIMEOUT,         // Temps d'attente max d'un capteur, en seconde
    LUMIN,           // Capteur de lumière : 0 ou 1
    LUMIN_LOW,       // En dessous : lumière "faible"
    LUMIN_HIGH,      // Au-dessus : lumière "forte"
    TEMP_AIR,        // Température : 0 ou 1
    MIN_TEMP_AIR,    // Min -40
    MAX_TEMP_AIR,    // Max 85
    HYGR,            // Humidité : 0 ou 1
    HYGR_MINT,       // L'humidité compte seulement au-dessus de cette température...
    HYGR_MAXT,       // ... et en dessous de celle-ci
    PRESSURE,        // Pression : 0 ou 1
    PRESSURE_MIN,    // Min 300
    PRESSURE_MAX,    // Max 1100
    NB_PARAMETRES    // Le nombre de paramètres (15)
};

// Table qui stocke les paramètres (gardée dans l'EEPROM, en vrai la Flash)
// Ex : parametre[TIMEOUT] vaut 30
extern int parametre[NB_PARAMETRES];

void VerificationEEPROM(void);     // Au démarrage : relit la Flash, ou met les valeurs par défaut
void InitialisationEEPROM(void);   // Valeurs par défaut + sauvegarde
void RecuperationValeur(void);     // Relit les paramètres dans la Flash
void AffichageData(void);          // Affiche tous les paramètres
void TraiterCommande(char commande[]);   // Ex : "LOG_INTERVALL=10", "RESET", "CLOCK=14:30:00"

#endif
