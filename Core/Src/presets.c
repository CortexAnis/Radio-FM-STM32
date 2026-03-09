/**
 * Presets - Sauvegarder et charger une fréquence dans un slot
 */

#include "presets.h"
#include "eeprom.h"
#include "RDA_5807.h"
#include "main.h"

extern uint16_t presets[3];
uint8_t preset_selection = 0;

/* Met la fréquence actuelle dans le preset num_preset, puis sauvegarde en EEPROM */
void sauvegarder_preset(uint8_t num_preset)
{
    if (num_preset >= PRESET_MAX) return;
    presets[num_preset] = RDA_GetRealFrequency(&hi2c1);
    Presets_Save();
}

/* Charge le preset num_preset : règle la radio sur cette fréquence */
void charger_preset(uint8_t num_preset)
{
    if (num_preset >= PRESET_MAX) return;
    uint16_t freq = presets[num_preset];
    if (!PRESET_VIDE(freq)) {
        RDA_Tune(&hi2c1, freq);
        HAL_Delay(200);
    }
}
