#ifndef BOUTONS_H
#define BOUTONS_H

#include "stm32f2xx_hal.h"
#include <stdint.h>

#define BTN_GAUCHE  1
#define BTN_DROITE  2
#define BTN_CENTRE  3
#define BTN_CENTRE_LONG 4  /* appui long centre (écraser preset) */

uint8_t gerer_bouton(void);
uint8_t centre_appui_long(void);  /* retourne 1 si centre maintenu ~400ms */

#endif

