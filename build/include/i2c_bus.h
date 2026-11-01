// i2c_bus.h - Bus I2C1 (PB8 = SCL, PB9 = SDA), partagé par la RTC et le BME280
#ifndef I2C_BUS_H
#define I2C_BUS_H

#include <stdint.h>

void i2c_init(void);
// Lit ou écrit "nombre" octets à partir du registre "registre" du module à l'adresse "adresse"
// Renvoie 1 si ça a marché, 0 sinon (module absent ou débranché)
int  i2c_lire(uint8_t adresse, uint8_t registre, uint8_t *donnees, int nombre);
int  i2c_ecrire(uint8_t adresse, uint8_t registre, uint8_t *donnees, int nombre);

#endif
