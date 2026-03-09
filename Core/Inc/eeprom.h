#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

#define PRESET_MAX       3
#define PRESET_VIDE(p)   ((p) == 0 || (p) == 0xFFFF)

/* Adresses dans l'EEPROM (24LC128) */
#define ADDR_PRESETS     0x0000   /* 3 presets × 2 octets = 6 octets */
#define ADDR_FREQ        0x0008   /* 1 fréquence = 2 octets */

void Presets_Save(void);
void Presets_Load(void);
void sauver_freq_eeprom(uint16_t freq);
uint16_t charger_freq_eeprom(void);

#endif
