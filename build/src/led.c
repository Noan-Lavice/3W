// led.c
// LED RGB Grove avec la puce P9813, branchée sur D7 :
// CI = PA8 (horloge), DI = PA9 (données).
// On met un bit sur DI, puis on fait passer CI de 0 à 1 : la puce lit le bit à ce moment-là.
#include "config.h"
#include "led.h"

static int couleurMode = ETEINT;   // Couleur du mode actuel
static int erreurs[NB_ERREURS];    // erreurs[i] = 1 si l'erreur i est active
static int couleurAffichee = -1;   // Couleur actuellement sur la LED (-1 = aucune)

// Petite attente de quelques microsecondes entre deux changements de CI.
// volatile : empêche le compilateur de supprimer cette boucle qui "ne fait rien".
static void petite_pause(void) {
    for (volatile int i = 0; i < 20; i++) {
    }
}

// Envoie 8 bits, en commençant par le bit de gauche
static void envoyer_octet(uint8_t octet) {
    for (int i = 0; i < 8; i++) {
        if (octet & 0x80) {   // Bit de gauche à 1 ?
            HAL_GPIO_WritePin(GPIOA, LED_DI_PIN, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(GPIOA, LED_DI_PIN, GPIO_PIN_RESET);
        }
        HAL_GPIO_WritePin(GPIOA, LED_CI_PIN, GPIO_PIN_RESET);
        petite_pause();
        HAL_GPIO_WritePin(GPIOA, LED_CI_PIN, GPIO_PIN_SET);   // La puce lit le bit ici
        petite_pause();
        octet = octet << 1;   // Le bit suivant passe à gauche
    }
}

// Envoie une couleur : rouge, vert et bleu de 0 (éteint) à 255 (à fond)
static void envoyer_couleur(uint8_t rouge, uint8_t vert, uint8_t bleu) {
    // Octet de contrôle demandé par la datasheet de la P9813 :
    // "11", puis les 2 premiers bits du bleu, du vert et du rouge, inversés
    uint8_t controle = 0xC0;
    if ((bleu & 0x80) == 0)  controle = controle | 0x20;
    if ((bleu & 0x40) == 0)  controle = controle | 0x10;
    if ((vert & 0x80) == 0)  controle = controle | 0x08;
    if ((vert & 0x40) == 0)  controle = controle | 0x04;
    if ((rouge & 0x80) == 0) controle = controle | 0x02;
    if ((rouge & 0x40) == 0) controle = controle | 0x01;

    // Trame : 4 octets à 0, contrôle, bleu, vert, rouge, 4 octets à 0
    for (int i = 0; i < 4; i++) envoyer_octet(0x00);
    envoyer_octet(controle);
    envoyer_octet(bleu);
    envoyer_octet(vert);
    envoyer_octet(rouge);
    for (int i = 0; i < 4; i++) envoyer_octet(0x00);
}

static void afficher(int couleur) {
    if (couleur == couleurAffichee) {
        return;   // Déjà affichée : rien à faire
    }
    if (couleur == ROUGE)  envoyer_couleur(255, 0, 0);
    if (couleur == VERT)   envoyer_couleur(0, 255, 0);
    if (couleur == BLEU)   envoyer_couleur(0, 0, 255);
    if (couleur == JAUNE)  envoyer_couleur(255, 255, 0);
    if (couleur == ORANGE) envoyer_couleur(255, 25, 0);   // Peu de vert, sinon on dirait du jaune
    if (couleur == BLANC)  envoyer_couleur(255, 255, 255);
    if (couleur == ETEINT) envoyer_couleur(0, 0, 0);
    couleurAffichee = couleur;
}

void led_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitStruct.Pin   = LED_CI_PIN | LED_DI_PIN;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;   // En sortie
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    afficher(ETEINT);
}

void led_couleur_mode(int couleur) {
    couleurMode = couleur;
}

void led_erreur(int erreur, int active) {
    erreurs[erreur] = active;
}

// Sans erreur : couleur du mode.
// Avec une erreur : clignotement à 1 Hz entre le rouge et une autre couleur.
//   Même durée             -> 500 ms rouge, 500 ms autre couleur
//   Couleur 2x plus longue -> 333 ms rouge, 667 ms autre couleur
// Si plusieurs erreurs, on affiche la plus importante (ordre du sujet).
void led_mise_a_jour(void) {
    int autre;
    uint32_t dureeRouge = 500;

    if (erreurs[ERREUR_RTC] == 1) {
        autre = BLEU;
    } else if (erreurs[ERREUR_CAPTEUR] == 1) {
        autre = VERT;
    } else if (erreurs[ERREUR_INCOHERENT] == 1) {
        autre = VERT;
        dureeRouge = 333;
    } else if (erreurs[ERREUR_SD_PLEINE] == 1) {
        autre = BLANC;
    } else if (erreurs[ERREUR_SD_ACCES] == 1) {
        autre = BLANC;
        dureeRouge = 333;
    } else {
        afficher(couleurMode);   // Pas d'erreur
        return;
    }

    // HAL_GetTick() % 1000 va de 0 à 999, puis recommence : une période de 1 s
    if (HAL_GetTick() % 1000 < dureeRouge) {
        afficher(ROUGE);
    } else {
        afficher(autre);
    }
}
