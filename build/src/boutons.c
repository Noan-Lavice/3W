// boutons.c
// Les 2 boutons : rouge (PB4) et vert (PB10).
// Grâce à la résistance de pull-up, la broche vaut 1 au repos et 0 quand on appuie.
//
// Problèmes rencontrés pendant les tests avec nos boutons :
//  - le contact n'est pas très franc : quand on maintient le bouton, la carte lit
//    parfois "relâché" pendant quelques ms -> on ignore les relâchements < 300 ms
//  - au moment de l'appui, le contact rebondit -> un appui doit durer 40 ms pour compter
//  - quand on appuie sur un bouton, la ligne de l'autre descend aussi (même module,
//    même câble) -> un seul bouton peut être appuyé à la fois : le premier enfoncé

#include "config.h"
#include "boutons.h"

#define DELAI_APPUI     40    // ms pour valider un appui (anti-rebond)
#define DELAI_RELACHE   300   // ms pour valider un relâchement

// Une case par bouton : [0] = rouge, [1] = vert
static int etat[2];                  // État après filtrage : 1 = appuyé
static uint32_t debutChangement[2];  // Depuis quand la lecture est différente de etat[]
static uint32_t debutAppui[2];       // Moment où l'appui a commencé
static int dejaTraite[2];            // 1 = l'appui long de cet appui a déjà été signalé
static int appuiLong[2];             // 1 = un appui long attend d'être lu par main.c

// Lecture de la broche, sans filtrage : 1 = appuyé, 0 = relâché
int bouton_enfonce(int bouton) {
    uint16_t broche;

    if (bouton == BOUTON_ROUGE) {
        broche = BOUTON_ROUGE_PIN;
    } else {
        broche = BOUTON_VERT_PIN;
    }

    if (HAL_GPIO_ReadPin(GPIOB, broche) == GPIO_PIN_RESET) {
        return 1;   // 0 V sur la broche = appuyé
    }
    return 0;
}

void boutons_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin  = BOUTON_ROUGE_PIN | BOUTON_VERT_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;   // En entrée : on lit les boutons
    GPIO_InitStruct.Pull = GPIO_PULLUP;       // Résistance de pull-up : 1 au repos
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_Delay(5);   // On laisse la tension se stabiliser
    etat[BOUTON_ROUGE] = bouton_enfonce(BOUTON_ROUGE);
    etat[BOUTON_VERT]  = bouton_enfonce(BOUTON_VERT);
}

// Au démarrage on regarde le bouton rouge pendant 0,5 s au lieu d'une seule lecture
// (sinon une micro-coupure faisait rater le mode configuration).
// On considère qu'il est appuyé s'il est vu enfoncé au moins 30 % du temps.
int bouton_rouge_au_demarrage(void) {
    uint32_t debut = HAL_GetTick();
    int nbLectures = 0;
    int nbAppuye = 0;

    while (HAL_GetTick() - debut < 500) {
        if (bouton_enfonce(BOUTON_ROUGE) == 1) {
            nbAppuye++;
        }
        nbLectures++;
        HAL_Delay(5);
    }

    if (nbLectures > 0 && nbAppuye * 10 >= nbLectures * 3) {
        // Cet appui a servi à choisir le mode, il ne doit pas compter comme un appui long
        etat[BOUTON_ROUGE] = 1;
        dejaTraite[BOUTON_ROUGE] = 1;
        return 1;
    }
    return 0;
}

// À appeler à chaque tour de la boucle principale
void boutons_mise_a_jour(void) {
    uint32_t maintenant = HAL_GetTick();
    int ordre[2] = {BOUTON_ROUGE, BOUTON_VERT};

    // On traite d'abord le bouton dont le changement a commencé le plus tôt
    // (c'est le vrai bouton : la ligne de l'autre descend un peu après)
    if (maintenant - debutChangement[BOUTON_VERT] > maintenant - debutChangement[BOUTON_ROUGE]) {
        ordre[0] = BOUTON_VERT;
        ordre[1] = BOUTON_ROUGE;
    }

    for (int k = 0; k < 2; k++) {
        int b = ordre[k];
        int autre = 1 - b;   // L'autre bouton
        int lu = bouton_enfonce(b);

        if (lu == etat[b]) {
            // Pas de changement
            debutChangement[b] = maintenant;
        } else if (lu == 1 && etat[autre] == 1) {
            // L'autre bouton est déjà appuyé : on ignore celui-ci
            debutChangement[b] = maintenant;
        } else if (lu == 1 && maintenant - debutChangement[b] >= DELAI_APPUI) {
            // Appuyé depuis 40 ms : l'appui est accepté
            etat[b] = 1;
            debutAppui[b] = maintenant;
            dejaTraite[b] = 0;
        } else if (lu == 0 && maintenant - debutChangement[b] >= DELAI_RELACHE) {
            // Relâché depuis 300 ms : le relâchement est accepté
            etat[b] = 0;
        }

        // Bouton maintenu 5 s -> appui long (une seule fois par appui)
        if (etat[b] == 1 && dejaTraite[b] == 0 && maintenant - debutAppui[b] >= DUREE_APPUI_LONG) {
            appuiLong[b] = 1;
            dejaTraite[b] = 1;
        }
    }
}

// Renvoie 1 une seule fois après un appui long, puis 0
int bouton_appui_long(int bouton) {
    if (appuiLong[bouton] == 1) {
        appuiLong[bouton] = 0;
        return 1;
    }
    return 0;
}

// Appelé à chaque changement de mode : on oublie les appuis en cours.
// Si on tient encore un bouton, il faudra le lâcher et rappuyer 5 s.
void boutons_reinitialiser(void) {
    for (int b = 0; b < 2; b++) {
        appuiLong[b] = 0;
        dejaTraite[b] = etat[b];
    }
}
