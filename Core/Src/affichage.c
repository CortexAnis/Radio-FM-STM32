#include "affichage.h"
#include "main.h"
#include "radio.h"
#include "presets.h"
#include "eeprom.h"
#include "ssd1306_symbols.h"

/* États globaux de l'IHM (déclarés dans main.c) */
extern uint8_t menu_actif;        /* 1 = on affiche le menu, 0 = une page fonctionnelle */
extern uint8_t menu_selection;    /* index de l'item sélectionné dans le menu */
extern uint8_t page_actuelle;     /* numéro de la page affichée (0 = menu, 1 = radio, ...) */
extern uint8_t bass_boost_actif;  /* état du Bass Boost audio */
extern const char* version_firmware;
extern uint16_t presets[3];       /* fréquences sauvegardées en presets */
extern uint8_t preset_selection;  /* index du preset surligné */

/**
 * @brief Mesure la batterie via l'ADC et renvoie un pourcentage.
 *
 * Démarre une conversion sur hadc1, lit la valeur brute 12 bits (0–4095)
 * et la convertit en pourcentage 0–100 %. La valeur retournée est saturée à 100.
 *
 * @retval uint8_t Niveau de batterie estimé en pourcentage (0–100)
 */
uint8_t lire_batterie(void)
{
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    uint8_t percent = (HAL_ADC_GetValue(&hadc1) * 100) / 4095;
    HAL_ADC_Stop(&hadc1);
    if (percent > 100) percent = 100;
    return percent;
}

/**
 * @brief Affiche une icône de batterie sur l'OLED en fonction du pourcentage.
 *
 * Utilise les symboles personnalisés Symbols_7x10 (#, $, %, &, !) pour
 * représenter 5 niveaux de batterie, de vide à plein.
 *
 * @param percent Niveau de batterie 0–100 % (valeur typiquement issue de lire_batterie()).
 */
static void afficher_batterie(uint8_t percent)
{
    ssd1306_SetCursor(4, 4);
    if (percent > 80)
        ssd1306_WriteString("#", Symbols_7x10, White);
    else if (percent > 60)
        ssd1306_WriteString("$", Symbols_7x10, White);
    else if (percent > 40)
        ssd1306_WriteString("%", Symbols_7x10, White);
    else if (percent > 20)
        ssd1306_WriteString("&", Symbols_7x10, White);
    else
        ssd1306_WriteString("!", Symbols_7x10, White);
}

/* 0–5 barres selon la qualité RDS/RSSI (seuils adaptés au signal faible) */
static const char signal_symbols[] = { '[', '\\', ']', '^', '_', '`' };

/**
 * @brief Affiche l'icône de niveau de signal radio (barres de réception).
 *
 * Convertit la qualité réseau (0–100) en un index de barres, puis affiche
 * le caractère Symbols_7x10 correspondant dans le coin supérieur droit.
 *
 * @param reseau Qualité du signal (0–100) renvoyée par RDA_GetQuality().
 */
void signal(uint8_t reseau)
{
    uint8_t bars;
    if (reseau <= 5)       bars = 1;
    else if (reseau <= 15) bars = 3;
    else if (reseau <= 30) bars = 3;
    else if (reseau <= 50) bars = 4;
    else if (reseau <= 70) bars = 5;
    else                   bars = 5;
    ssd1306_FillRectangle(98, 0, 127, 16, Black);
    char buf[2] = { signal_symbols[bars], '\0' };
    ssd1306_SetCursor(115, 4);
    ssd1306_WriteString(buf, Symbols_7x10, White);
}

/**
 * @brief Met à jour l'affichage de la fréquence sur l'écran.
 *
 * Récupère la fréquence courante auprès du tuner RDA5807, la formate en MHz
 * (par ex. "98.5") et la dessine en gros caractères au centre de l'écran.
 * Efface la zone de fréquence avant de réécrire.
 */
void mettre_a_jour_frequence(void)
{
    char buffer[10];
    sprintf(buffer, "%d.%d", RDA_GetRealFrequency(&hi2c1) / 100,
                            (RDA_GetRealFrequency(&hi2c1) / 10) % 10);
    ssd1306_FillRectangle(32, 19, 90, 45, Black);
    ssd1306_SetCursor(22, 22);
    ssd1306_WriteString(buffer, Font_16x26, White);
    ssd1306_SetCursor(105, 31);
    ssd1306_WriteString("Mhz", Font_7x10, White);
    ssd1306_UpdateScreen();
}

void ligne(void)
{
    ssd1306_Line(0, 18, 127, 18, White);
    ssd1306_Line(0, 51, 127, 51, White);
}

/**
 * @brief Page 0 : écran de menu principal avec les 6 entrées.
 *
 * Dessine le titre "MENU" et une grille 2x3 (Radio, Rech, Preset, Audio,
 * Param, Info) avec surbrillance de l'option sélectionnée (menu_selection).
 * Le bandeau inférieur affiche "<  OK  >".
 */
void afficher_menu(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(40, 2);
    ssd1306_WriteString("MENU", Font_16x15, White);

    if (menu_selection == 0) {
        ssd1306_FillRectangle(2, 15, 39, 30, White);
        ssd1306_SetCursor(4, 19);
        ssd1306_WriteString("Radio", Font_7x10, Black);
    } else {
        ssd1306_DrawRectangle(2, 15, 39, 30, White);
        ssd1306_SetCursor(4, 19);
        ssd1306_WriteString("Radio", Font_7x10, White);
    }
    if (menu_selection == 1) {
        ssd1306_FillRectangle(43, 15, 80, 30, White);
        ssd1306_SetCursor(45, 19);
        ssd1306_WriteString("Rech", Font_7x10, Black);
    } else {
        ssd1306_DrawRectangle(43, 15, 80, 30, White);
        ssd1306_SetCursor(45, 19);
        ssd1306_WriteString("Rech", Font_7x10, White);
    }
    if (menu_selection == 2) {
        ssd1306_FillRectangle(84, 15, 125, 30, White);
        ssd1306_SetCursor(86, 19);
        ssd1306_WriteString("Preset", Font_7x10, Black);
    } else {
        ssd1306_DrawRectangle(84, 15, 121, 30, White);
        ssd1306_SetCursor(86, 19);
        ssd1306_WriteString("Preset", Font_7x10, White);
    }
    if (menu_selection == 3) {
        ssd1306_FillRectangle(2, 33, 39, 48, White);
        ssd1306_SetCursor(4, 37);
        ssd1306_WriteString("Audio", Font_7x10, Black);
    } else {
        ssd1306_DrawRectangle(2, 33, 39, 48, White);
        ssd1306_SetCursor(4, 37);
        ssd1306_WriteString("Audio", Font_7x10, White);
    }
    if (menu_selection == 4) {
        ssd1306_FillRectangle(43, 33, 80, 48, White);
        ssd1306_SetCursor(45, 37);
        ssd1306_WriteString("Param", Font_7x10, Black);
    } else {
        ssd1306_DrawRectangle(43, 33, 80, 48, White);
        ssd1306_SetCursor(45, 37);
        ssd1306_WriteString("Param", Font_7x10, White);
    }
    if (menu_selection == 5) {
        ssd1306_FillRectangle(84, 33, 121, 48, White);
        ssd1306_SetCursor(86, 37);
        ssd1306_WriteString("Info", Font_7x10, Black);
    } else {
        ssd1306_DrawRectangle(84, 33, 121, 48, White);
        ssd1306_SetCursor(86, 37);
        ssd1306_WriteString("Info", Font_7x10, White);
    }
    ssd1306_SetCursor(34, 54);
    ssd1306_WriteString("<  OK  >", Font_7x10, White);
    ssd1306_Line(0, 51, 127, 51, White);
    ssd1306_UpdateScreen();
}

/**
 * @brief Route l'affichage vers la page correspondante.
 *
 * @param i Numéro de page à afficher :
 *          0 = menu, 1 = radio, 2 = recherche, 3 = presets,
 *          4 = audio, 5 = paramètres, 6 = info.
 *
 * Appelle la fonction d'affichage de page adaptée (afficher_page_radio, etc.).
 */
void Affiche_Page(uint8_t i)
{
    switch (i) {
        case 0: afficher_menu(); break;
        case 1: afficher_page_radio(); break;
        case 2: afficher_page_recherche(); break;
        case 3: afficher_page_presets(); break;
        case 4: afficher_page_audio(); break;
        case 5: afficher_page_parametres(); break;
        case 6: afficher_page_info(); break;
        default: afficher_menu(); break;
    }
}

/**
 * @brief Page 1 : écran principal Radio FM.
 *
 * Affiche le titre "FM", la fréquence courante, l'icône de batterie,
 * le niveau de signal, ainsi que le bandeau de commande
 * "Freq-  Menu  Freq+".
 */
void afficher_page_radio(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(50, 2);
    ssd1306_WriteString("FM", Font_16x15, White);
    mettre_a_jour_frequence();
    afficher_batterie(lire_batterie());
    signal(RDA_GetQuality(&hi2c1));
    ssd1306_SetCursor(0, 54);
    ssd1306_WriteString("Freq-  Menu  Freq+", Font_7x10, White);
    ligne();
    ssd1306_UpdateScreen();
}

/**
 * @brief Page 2 : écran de recherche automatique de station.
 *
 * Affiche le titre "RECHERCHE" et la fréquence courante.
 * Le bandeau inférieur "S-   OK   S+" indique :
 * - btn1 : recherche station précédente,
 * - btn2 : recherche station suivante,
 * - btn3 : validation / retour.
 */
void afficher_page_recherche(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(13, 2);
    ssd1306_WriteString("RECHERCHE", Font_16x15, White);
    char buffer[15];
    sprintf(buffer, "%d.%d", RDA_GetRealFrequency(&hi2c1) / 100,
                            (RDA_GetRealFrequency(&hi2c1) / 10) % 10);
    ssd1306_SetCursor(22, 25);
    ssd1306_WriteString(buffer, Font_16x26, White);
    ssd1306_SetCursor(26, 54);
    ssd1306_WriteString("S-   OK   S+", Font_7x10, White);
    ssd1306_SetCursor(105, 31);
    ssd1306_WriteString("Mhz", Font_7x10, White);
    ligne();
    ssd1306_UpdateScreen();
}

/**
 * @brief Page 3 : écran de gestion des 3 presets radio.
 *
 * Affiche les presets P1, P2, P3 avec leur fréquence ou "Vide".
 * La ligne en surbrillance (fond blanc, ">") correspond à preset_selection.
 * Le bandeau "<  OK  >" permet de choisir / valider un preset.
 */
void afficher_page_presets(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(26, 2);
    ssd1306_WriteString("PRESETS", Font_16x15, White);

    char p1[20], p2[20], p3[20];
    if (PRESET_VIDE(presets[0])) sprintf(p1, "P1: Vide"); else sprintf(p1, "P1: %d.%d MHz", presets[0] / 100, (presets[0] / 10) % 10);
    if (PRESET_VIDE(presets[1])) sprintf(p2, "P2: Vide"); else sprintf(p2, "P2: %d.%d MHz", presets[1] / 100, (presets[1] / 10) % 10);
    if (PRESET_VIDE(presets[2])) sprintf(p3, "P3: Vide"); else sprintf(p3, "P3: %d.%d MHz", presets[2] / 100, (presets[2] / 10) % 10);

    if (preset_selection == 0) {
        ssd1306_FillRectangle(0, 20, 127, 27, White);
        ssd1306_SetCursor(5, 21);
        ssd1306_WriteString(">", Font_7x10, Black);
        ssd1306_SetCursor(15, 21);
        ssd1306_WriteString(p1, Font_7x10, Black);
    } else {
        ssd1306_SetCursor(15, 21);
        ssd1306_WriteString(p1, Font_7x10, White);
    }
    if (preset_selection == 1) {
        ssd1306_FillRectangle(0, 28, 127, 35, White);
        ssd1306_SetCursor(5, 29);
        ssd1306_WriteString(">", Font_7x10, Black);
        ssd1306_SetCursor(15, 29);
        ssd1306_WriteString(p2, Font_7x10, Black);
    } else {
        ssd1306_SetCursor(15, 29);
        ssd1306_WriteString(p2, Font_7x10, White);
    }
    if (preset_selection == 2) {
        ssd1306_FillRectangle(0, 36, 127, 43, White);
        ssd1306_SetCursor(5, 37);
        ssd1306_WriteString(">", Font_7x10, Black);
        ssd1306_SetCursor(15, 37);
        ssd1306_WriteString(p3, Font_7x10, Black);
    } else {
        ssd1306_SetCursor(15, 37);
        ssd1306_WriteString(p3, Font_7x10, White);
    }
    ssd1306_SetCursor(35, 54);
    ssd1306_WriteString("<  OK  >", Font_7x10, White);
    ligne();
    ssd1306_UpdateScreen();
}

/**
 * @brief Page 4 : écran de réglages audio.
 *
 * Affiche l'état du Bass Boost (ON/OFF) et un niveau de volume indicatif.
 * Cette page est utilisée avec les actions de la page_actuelle == 4 dans main.c.
 */
void afficher_page_audio(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(40, 2);
    ssd1306_WriteString("AUDIO", Font_16x15, White);
    ssd1306_SetCursor(10, 25);
    ssd1306_WriteString("Bass Boost:", Font_7x10, White);
    if (bass_boost_actif) {
        ssd1306_SetCursor(80, 25);
        ssd1306_WriteString("ON", Font_7x10, White);
    } else {
        ssd1306_SetCursor(80, 25);
        ssd1306_WriteString("OFF", Font_7x10, White);
    }
    ssd1306_SetCursor(10, 35);
    ssd1306_WriteString("Volume:", Font_7x10, White);
    ssd1306_SetCursor(80, 35);
    ssd1306_WriteString("75%", Font_7x10, White);
    ssd1306_SetCursor(35, 54);
    ssd1306_WriteString("<  OK  >", Font_7x10, White);
    ligne();
    ssd1306_UpdateScreen();
}

/**
 * @brief Page 5 : écran de paramètres (placeholder pour extensions).
 *
 * Affiche un titre "PARAMETRES" et pourra accueillir de futurs réglages
 * liés au système (contraste, veille écran, etc.).
 */
void afficher_page_parametres(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(7, 2);
    ssd1306_WriteString("PARAMETRES", Font_16x15, White);
    ssd1306_SetCursor(10, 20);
    ssd1306_WriteString("Bande: 87-108MHz", Font_7x10, White);
    ssd1306_SetCursor(10, 30);
    ssd1306_WriteString("Espace: 100kHz", Font_7x10, White);
    ssd1306_SetCursor(10, 40);
    ssd1306_WriteString("Mode: Stereo", Font_7x10, White);
    ssd1306_SetCursor(35, 54);
    ssd1306_WriteString("<  OK  >", Font_7x10, White);
    ligne();
    ssd1306_UpdateScreen();
}

/**
 * @brief Page 6 : écran d'informations (version firmware, crédits, etc.).
 *
 * Affiche la version du firmware (version_firmware) et d'autres informations
 * utiles sur le projet (nom du BE, carte STM32, etc.).
 */
void afficher_page_info(void)
{
    ssd1306_Fill(Black);
    ssd1306_SetCursor(35, 2);
    ssd1306_WriteString("INFO", Font_16x15, White);
    ssd1306_SetCursor(10, 20);
    ssd1306_WriteString("Version:", Font_7x10, White);
    ssd1306_SetCursor(70, 20);
    ssd1306_WriteString(version_firmware, Font_7x10, White);
    ssd1306_SetCursor(10, 30);
    ssd1306_WriteString("Bass Boost:", Font_7x10, White);
    if (bass_boost_actif) {
        ssd1306_SetCursor(80, 30);
        ssd1306_WriteString("ACTIF", Font_7x10, White);
    } else {
        ssd1306_SetCursor(80, 30);
        ssd1306_WriteString("INACTIF", Font_7x10, White);
    }
    char freq_str[15], qual_str[15];
    sprintf(freq_str, "Freq: %d.%d MHz", RDA_GetRealFrequency(&hi2c1) / 100,
                                        (RDA_GetRealFrequency(&hi2c1) / 10) % 10);
    sprintf(qual_str, "Signal: %d%%", RDA_GetQuality(&hi2c1));
    ssd1306_SetCursor(10, 40);
    ssd1306_WriteString(freq_str, Font_7x10, White);
    ssd1306_SetCursor(10, 50);
    ssd1306_WriteString(qual_str, Font_7x10, White);
    ssd1306_Line(0, 18, 127, 18, White);
    ssd1306_UpdateScreen();
}
