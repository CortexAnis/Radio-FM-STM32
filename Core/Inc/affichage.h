#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include <stdint.h>
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "RDA_5807.h"

extern uint8_t menu_actif;
extern uint8_t menu_selection;
extern uint8_t page_actuelle;
extern const char* version_firmware;

void afficher_menu(void);
void Affiche_Page(uint8_t i);
void afficher_page_radio(void);
void afficher_page_recherche(void);
void afficher_page_presets(void);
void afficher_page_audio(void);
void afficher_page_parametres(void);
void afficher_page_info(void);
void mettre_a_jour_frequence(void);
void signal(uint8_t reseau);
void ligne(void);

#endif

