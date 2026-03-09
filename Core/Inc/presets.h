#ifndef PRESETS_H
#define PRESETS_H

#include <stdint.h>

extern uint16_t presets[3];
extern uint8_t preset_selection;

void sauvegarder_preset(uint8_t num_preset);
void charger_preset(uint8_t num_preset);

#endif

