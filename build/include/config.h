// config.h
// Les broches et les réglages du projet
//
// Montage : NUCLEO-L476RG + shield SD + Grove Base Shield (interrupteur sur 3V3)
//   Bouton rouge  -> D5 = PB4
//   Bouton vert   -> D6 = PB10 (même module que le rouge, branché sur D5)
//   LED RGB       -> D7 : CI = PA8, DI = PA9
//   Luminosité    -> A0 = PA0
//   RTC et BME280 -> I2C : SCL = PB8, SDA = PB9
//   Console       -> câble USB (UART2 : PA2 et PA3), 115200 bauds
//
// On n'a pas de GPS, et la carte SD n'est pas câblée (il manque les 3 fils ICSP) :
// l'enregistrement sur la carte SD est simulé, les lignes sont affichées dans la console.

#ifndef CONFIG_H
#define CONFIG_H

#include "stm32l4xx_hal.h"

#define VERSION_PROGRAMME   "4.0"
#define NUMERO_LOT          "LOT-2026-001"

// 1 = LOG_INTERVALL compté en secondes (pour la démo), 0 = en minutes (comme le sujet)
#define MODE_DEMO   1

// Boutons (port B)
#define BOUTON_ROUGE_PIN   GPIO_PIN_4
#define BOUTON_VERT_PIN    GPIO_PIN_10

// LED (port A)
#define LED_CI_PIN   GPIO_PIN_8
#define LED_DI_PIN   GPIO_PIN_9

// Adresses I2C
#define ADRESSE_RTC   0x68
#define ADRESSE_BME   0x76

// Durées en millisecondes
#define DUREE_APPUI_LONG      5000          // 5 s
#define DUREE_INACTIVITE      (30 * 60000)  // 30 min
#define PERIODE_MAINTENANCE   5000          // Une mesure toutes les 5 s

// Valeur qui veut dire "pas de mesure" : on écrit NA dans le fichier
#define NA   -9999

#endif
