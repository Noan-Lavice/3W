// boutons.h - Bouton rouge (D5 = PB4) et bouton vert (D6 = PB10)
#ifndef BOUTONS_H
#define BOUTONS_H

#define BOUTON_ROUGE   0
#define BOUTON_VERT    1

void boutons_init(void);
int  bouton_enfonce(int bouton);       // Lecture de la broche : 1 = appuyé
int  bouton_rouge_au_demarrage(void);  // 1 = rouge tenu au démarrage (mode configuration)
void boutons_mise_a_jour(void);        // À appeler à chaque tour de la boucle principale
int  bouton_appui_long(int bouton);    // 1 une seule fois quand le bouton a été tenu 5 s
void boutons_reinitialiser(void);      // Oublie les appuis en cours (changement de mode)

#endif
