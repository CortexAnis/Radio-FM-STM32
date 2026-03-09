/**
 * EEPROM 24LC128 - Sauvegarde presets et fréquence
 * Envoi octet par octet (bus I2C = 8 bits)
 */

#include "eeprom.h"
#include "main.h"

extern uint16_t presets[3];

#define ADDR_I2C  (0x50 << 1)   /* 24LC128: adresse 0x50, décalée pour HAL */

/* --- Écrit 1 octet --- */
static void write_byte(uint16_t addr, uint8_t val)
{
    HAL_I2C_Mem_Write(&hi2c1, ADDR_I2C, addr, I2C_MEMADD_SIZE_16BIT, &val, 1, 100);
    HAL_Delay(5);   /* EEPROM: 5 ms pour terminer l'écriture */
}

/* --- Lit 1 octet --- */
static uint8_t read_byte(uint16_t addr)
{
    uint8_t val = 0;
    HAL_I2C_Mem_Read(&hi2c1, ADDR_I2C, addr, I2C_MEMADD_SIZE_16BIT, &val, 1, 100);
    return val;
}

/* --- Écrit un uint16 (2 octets) --- */
static void write_uint16(uint16_t addr, uint16_t val)
{
    write_byte(addr,     (uint8_t)(val >> 8));   /* octet haut */
    write_byte(addr + 1, (uint8_t)(val & 0xFF)); /* octet bas */
}

/* --- Lit un uint16 (2 octets) --- */
static uint16_t read_uint16(uint16_t addr)
{
    uint8_t hi = read_byte(addr);
    uint8_t lo = read_byte(addr + 1);
    return ((uint16_t)hi << 8) | lo;
}

/**
 * @brief Sauvegarde les 3 presets radio dans l'EEPROM 24LC128.
 *
 * Écrit séquentiellement les 3 entrées du tableau global presets[]
 * à partir de l'adresse ADDR_PRESETS (2 octets par preset, format uint16).
 * Chaque écriture est effectuée octet par octet via le bus I2C.
 */
void Presets_Save(void)
{
    for (uint8_t i = 0; i < PRESET_MAX; i++)
        write_uint16(ADDR_PRESETS + i * 2, presets[i]);
}

/**
 * @brief Charge les 3 presets radio depuis l'EEPROM 24LC128.
 *
 * Lit les 3 fréquences stockées à partir de ADDR_PRESETS et met à jour
 * le tableau global presets[]. Si une case vaut 0xFFFF (EEPROM vierge),
 * le preset correspondant est remis à 0 (preset vide).
 */
void Presets_Load(void)
{
    for (uint8_t i = 0; i < PRESET_MAX; i++) {
        uint16_t v = read_uint16(ADDR_PRESETS + i * 2);
        presets[i] = (v == 0xFFFF) ? 0 : v;   /* 0xFFFF = EEPROM vierge */
    }
}

/**
 * @brief Sauvegarde la fréquence courante de la radio dans l'EEPROM.
 *
 * @param freq Fréquence en centièmes de MHz (par ex. 8750 = 87.50 MHz)
 *
 * Écrit la valeur freq (uint16) à l'adresse ADDR_FREQ pour la retrouver
 * automatiquement au prochain démarrage.
 */
void sauver_freq_eeprom(uint16_t freq)
{
    write_uint16(ADDR_FREQ, freq);
}

/**
 * @brief Charge la dernière fréquence radio sauvegardée en EEPROM.
 *
 * Lit la valeur stockée à ADDR_FREQ. Si l'EEPROM est vierge (0xFFFF),
 * renvoie une fréquence par défaut (87.50 MHz).
 *
 * @retval uint16_t Fréquence en centièmes de MHz (ex. 8750 = 87.50 MHz)
 */
uint16_t charger_freq_eeprom(void)
{
    uint16_t v = read_uint16(ADDR_FREQ);
    return (v == 0xFFFF) ? 8750 : v;
}
