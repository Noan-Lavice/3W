#include "boutons_led.h"
#include "materiel.h"

/*
 * Bouton appuye = broche a 0
 * car on utilise une resistance pull-up interne.
 */

#define BTN_ROUGE_PORT GPIOC
#define BTN_ROUGE_PIN  GPIO_PIN_2

#define BTN_VERT_PORT  GPIOC
#define BTN_VERT_PIN   GPIO_PIN_3

#define LED_CLK_PORT   GPIOB
#define LED_CLK_PIN    GPIO_PIN_4

#define LED_DATA_PORT  GPIOB
#define LED_DATA_PIN   GPIO_PIN_5

static uint32_t debut_rouge = 0;
static uint32_t debut_vert = 0;

static bool rouge_deja_envoye = false;
static bool vert_deja_envoye = false;

static void LED_Clock(void)
{
    HAL_GPIO_WritePin(LED_CLK_PORT, LED_CLK_PIN, GPIO_PIN_SET);
    HAL_Delay(1);

    HAL_GPIO_WritePin(LED_CLK_PORT, LED_CLK_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
}

static void LED_EnvoyerOctet(uint8_t valeur)
{
    int i;

    for (i = 7; i >= 0; i--)
    {
        if (valeur & (1 << i))
            HAL_GPIO_WritePin(LED_DATA_PORT, LED_DATA_PIN, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(LED_DATA_PORT, LED_DATA_PIN, GPIO_PIN_RESET);

        LED_Clock();
    }
}

static void LED_RGB(uint8_t rouge, uint8_t vert, uint8_t bleu)
{
    uint8_t drapeau;
    int i;

    drapeau = 0xC0
        | ((~bleu & 0xC0) >> 2)
        | ((~vert & 0xC0) >> 4)
        | ((~rouge & 0xC0) >> 6);

    for (i = 0; i < 4; i++)
        LED_EnvoyerOctet(0x00);

    LED_EnvoyerOctet(drapeau);
    LED_EnvoyerOctet(bleu);
    LED_EnvoyerOctet(vert);
    LED_EnvoyerOctet(rouge);

    for (i = 0; i < 4; i++)
        LED_EnvoyerOctet(0x00);
}

void BoutonsLed_Init(void)
{
    Led_Couleur(COULEUR_ETEINTE);
}

bool BoutonRouge_Appuye(void)
{
    return HAL_GPIO_ReadPin(BTN_ROUGE_PORT, BTN_ROUGE_PIN) == GPIO_PIN_RESET;
}

bool BoutonVert_Appuye(void)
{
    return HAL_GPIO_ReadPin(BTN_VERT_PORT, BTN_VERT_PIN) == GPIO_PIN_RESET;
}

bool BoutonRouge_AppuiLong(void)
{
    if (BoutonRouge_Appuye())
    {
        if (debut_rouge == 0)
            debut_rouge = HAL_GetTick();

        if (!rouge_deja_envoye &&
            HAL_GetTick() - debut_rouge >= 5000)
        {
            rouge_deja_envoye = true;
            return true;
        }
    }
    else
    {
        debut_rouge = 0;
        rouge_deja_envoye = false;
    }

    return false;
}

bool BoutonVert_AppuiLong(void)
{
    if (BoutonVert_Appuye())
    {
        if (debut_vert == 0)
            debut_vert = HAL_GetTick();

        if (!vert_deja_envoye &&
            HAL_GetTick() - debut_vert >= 5000)
        {
            vert_deja_envoye = true;
            return true;
        }
    }
    else
    {
        debut_vert = 0;
        vert_deja_envoye = false;
    }

    return false;
}

void Led_Couleur(CouleurLed couleur)
{
    if (couleur == COULEUR_VERTE)
        LED_RGB(0, 80, 0);

    else if (couleur == COULEUR_JAUNE)
        LED_RGB(70, 70, 0);

    else if (couleur == COULEUR_BLEUE)
        LED_RGB(0, 0, 80);

    else if (couleur == COULEUR_ORANGE)
        LED_RGB(90, 35, 0);

    else if (couleur == COULEUR_ROUGE)
        LED_RGB(90, 0, 0);

    else if (couleur == COULEUR_BLANCHE)
        LED_RGB(60, 60, 60);

    else
        LED_RGB(0, 0, 0);
}
