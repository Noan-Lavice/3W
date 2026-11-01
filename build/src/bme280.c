
#include "config.h"
#include "bme280.h"
#include "i2c_bus.h"

// Coefficients de calibration (noms de la datasheet)
static int dig_T1, dig_T2, dig_T3;
static int dig_P1, dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static int dig_H1, dig_H2, dig_H3, dig_H4, dig_H5, dig_H6;
static double t_fine;   // Calculé avec la température, utilisé pour la pression et l'humidité

int bme280_init(void) {
    uint8_t c[26];
    uint8_t h[7];
    uint8_t valeur;

    // 1) Lecture des coefficients de calibration (registres 0x88 à 0xA1)
    if (i2c_lire(ADRESSE_BME, 0x88, c, 26) == 0) {
        return 0;   // Le capteur ne répond pas
    }
    // Chaque coefficient est sur 2 octets, l'octet faible en premier : fort * 256 + faible
    // (int16_t) = nombre signé sur 16 bits, pour les coefficients qui peuvent être négatifs
    dig_T1 = c[1] * 256 + c[0];
    dig_T2 = (int16_t)(c[3] * 256 + c[2]);
    dig_T3 = (int16_t)(c[5] * 256 + c[4]);
    dig_P1 = c[7] * 256 + c[6];
    dig_P2 = (int16_t)(c[9] * 256 + c[8]);
    dig_P3 = (int16_t)(c[11] * 256 + c[10]);
    dig_P4 = (int16_t)(c[13] * 256 + c[12]);
    dig_P5 = (int16_t)(c[15] * 256 + c[14]);
    dig_P6 = (int16_t)(c[17] * 256 + c[16]);
    dig_P7 = (int16_t)(c[19] * 256 + c[18]);
    dig_P8 = (int16_t)(c[21] * 256 + c[20]);
    dig_P9 = (int16_t)(c[23] * 256 + c[22]);
    dig_H1 = c[25];

    // Suite des coefficients, pour l'humidité (registres 0xE1 à 0xE7)
    if (i2c_lire(ADRESSE_BME, 0xE1, h, 7) == 0) {
        return 0;
    }
    dig_H2 = (int16_t)(h[1] * 256 + h[0]);
    dig_H3 = h[2];
    dig_H4 = (int8_t)h[3] * 16 + (h[4] % 16);   // H4 et H5 se partagent l'octet h[4]
    dig_H5 = (int8_t)h[5] * 16 + (h[4] / 16);
    dig_H6 = (int8_t)h[6];

    // 2) Réveil du capteur (il dort à l'allumage)
    valeur = 0x01;   // Humidité mesurée 1 fois (à écrire avant le registre 0xF4)
    i2c_ecrire(ADRESSE_BME, 0xF2, &valeur, 1);
    valeur = 0xA0;   // Une mesure toutes les 1000 ms
    i2c_ecrire(ADRESSE_BME, 0xF5, &valeur, 1);
    valeur = 0x27;   // Température x1, pression x1, mode normal (mesure en continu)
    i2c_ecrire(ADRESSE_BME, 0xF4, &valeur, 1);

    HAL_Delay(10);   // Le temps de la première mesure
    return 1;
}

// Formules de la datasheet Bosch (section 8.1)

// Température en °C
static double calculer_temperature(int adc_T) {
    double var1 = (adc_T / 16384.0 - dig_T1 / 1024.0) * dig_T2;
    double var2 = (adc_T / 131072.0 - dig_T1 / 8192.0) * (adc_T / 131072.0 - dig_T1 / 8192.0) * dig_T3;
    t_fine = var1 + var2;
    return (var1 + var2) / 5120.0;
}

// Pression en Pa
static double calculer_pression(int adc_P) {
    double var1 = t_fine / 2.0 - 64000.0;
    double var2 = var1 * var1 * dig_P6 / 32768.0;
    var2 = var2 + var1 * dig_P5 * 2.0;
    var2 = var2 / 4.0 + dig_P4 * 65536.0;
    var1 = (dig_P3 * var1 * var1 / 524288.0 + dig_P2 * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * dig_P1;
    if (var1 == 0.0) {
        return 0;   // Évite une division par zéro
    }
    double p = 1048576.0 - adc_P;
    p = (p - var2 / 4096.0) * 6250.0 / var1;
    var1 = dig_P9 * p * p / 2147483648.0;
    var2 = p * dig_P8 / 32768.0;
    return p + (var1 + var2 + dig_P7) / 16.0;
}

// Humidité en %
static double calculer_humidite(int adc_H) {
    double h = t_fine - 76800.0;
    h = (adc_H - (dig_H4 * 64.0 + dig_H5 / 16384.0 * h)) *
        (dig_H2 / 65536.0 * (1.0 + dig_H6 / 67108864.0 * h * (1.0 + dig_H3 / 67108864.0 * h)));
    h = h * (1.0 - dig_H1 * h / 524288.0);
    if (h > 100.0) h = 100.0;
    if (h < 0.0) h = 0.0;
    return h;
}

int bme280_lire(float *temperature, float *humidite, float *pression) {
    uint8_t d[8];

    // Les mesures sont dans 8 registres à partir de 0xF7 : pression (3), température (3), humidité (2)
    if (i2c_lire(ADRESSE_BME, 0xF7, d, 8) == 0) {
        return 0;   // Le capteur ne répond pas
    }

    // Pression et température font 20 bits : 8 + 8 + les 4 bits de gauche du 3e octet
    int adc_P = d[0] * 4096 + d[1] * 16 + d[2] / 16;
    int adc_T = d[3] * 4096 + d[4] * 16 + d[5] / 16;
    int adc_H = d[6] * 256 + d[7];

    if (adc_T == 0x80000) {
        return 0;   // Valeur du capteur tant qu'il n'a pas encore mesuré
    }

    // La température doit être calculée en premier : elle remplit t_fine
    *temperature = calculer_temperature(adc_T);
    *pression    = calculer_pression(adc_P) / 100.0;   // Pa -> hPa
    *humidite    = calculer_humidite(adc_H);
    return 1;
}
