#ifndef BME280_H
#define BME280_H

int bme280_init(void);   // 1 = capteur trouvé et réveillé
// On donne l'adresse de 3 variables (comme avec scanf) et la fonction les remplit :
// température en °C, humidité en %, pression en hPa. Renvoie 1 si la lecture a marché.
int bme280_lire(float *temperature, float *humidite, float *pression);

#endif
