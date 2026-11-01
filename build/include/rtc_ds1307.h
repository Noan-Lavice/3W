#ifndef RTC_DS1307_H
#define RTC_DS1307_H

int rtc_init(void);
// On donne l'adresse de 6 variables (comme avec scanf) et la fonction les remplit
int rtc_lire(int *annee, int *mois, int *jour, int *heure, int *minute, int *seconde);
int rtc_regler_heure(int heure, int minute, int seconde);
int rtc_regler_date(int mois, int jour, int annee);
int rtc_regler_jour(int jour);   // 1 = lundi ... 7 = dimanche

#endif
