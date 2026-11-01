#ifndef CAPTEURS_H
#define CAPTEURS_H

// Une mesure. Quand un capteur ne répond pas, sa valeur vaut NA (voir config.h)
typedef struct {
    int dateOK;          // 1 si l'horloge a répondu
    int annee;
    int mois;
    int jour;
    int heure;
    int minute;
    int seconde;
    float temperature;   // En °C
    float humidite;      // En %
    float pression;      // En hPa
    int lumiere;         // De 0 à 1023
} Mesure;

// Le nom des colonnes (première ligne de chaque fichier)
#define ENTETE   "date;heure;temperature_C;humidite_pct;pression_hPa;luminosite;niveau_lumiere"

extern Mesure mesure;     // La dernière mesure
extern char ligne[100];   // La dernière mesure en texte, ex : "2026-10-23;14:20:00;27.1;38.5;983.2;224;faible"

void capteurs_init(void);
void capteurs_mesurer(void);   // Remplit "mesure"
void capteurs_ligne(void);     // Remplit "ligne" à partir de "mesure"

#endif
