// stockage.h - Enregistrement des mesures dans les fichiers AAMMJJ_R.LOG
// Notre carte SD n'est pas câblée : l'enregistrement est simulé dans la console.
#ifndef STOCKAGE_H
#define STOCKAGE_H

void stockage_init(void);
void stockage_ajouter(void);   // Ajoute la dernière mesure ("ligne" de capteurs.c) au fichier du jour

#endif
