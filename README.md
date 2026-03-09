## Radio FM STM32 – Projet TP

Ce dépôt contient un projet de **radio FM numérique** basé sur un microcontrôleur **STM32F215**.  
Il utilise un tuner **RDA5807M**, un écran **OLED SSD1306** et une **EEPROM I2C** pour stocker des presets.

---

## 1. Matériel utilisé

- **Microcontrôleur** : STM32F215RGT6 (famille STM32F2)
- **Tuner FM** : RDA5807M (interface I2C)
- **Afficheur** : OLED 128x64, contrôleur SSD1306 (I2C)
- **EEPROM** : 24Cxx, adresse I2C `0xA0`
- **Boutons** :
  - Gauche : `PC3`
  - Centre : `PA0`
  - Droite : `PA1`

---

## 2. Structure du code

### Dossier `Core/Src`

- `main.c`  
  Point d’entrée de l’application :
  - Initialisation HAL, horloge, périphériques (GPIO, ADC, I2C, RTC, UART).
  - Initialisation écran (`ssd1306_Init`) et radio (`RDA_Init`, `RDA_Tune`).
  - Chargement des presets depuis l’EEPROM (`charger_presets_eeprom`).
  - Boucle `while(1)` :
    - Lecture des boutons via `gerer_bouton()`.
    - Gestion du **menu** (`menu_actif`) ou de la **page active** (`page_actuelle`).

- `boutons.c` / `boutons.h`  
  Gestion des trois boutons avec détection de **front montant** :
  - `gerer_bouton()` renvoie :
    - `0` : aucun bouton
    - `1` : gauche
    - `2` : droite
    - `3` : centre

- `radio.c` / `radio.h`  
  Fonctions de plus haut niveau autour du driver **RDA5807** :
  - Changement de fréquence, lecture de la qualité du signal, etc.

- `affichage.c` / `affichage.h`  
  Fonctions d’affichage pour l’écran SSD1306 :
  - `Affiche_Page(page)` : menu, radio, recherche, presets, audio, paramètres, info.
  - Fonctions dédiées : `afficher_page_recherche()`, `afficher_page_presets()`, `signal()`...

- `eeprom.c` / `eeprom.h`  
  Accès à l’EEPROM I2C :
  - `EEPROM_WriteUint16(addr, value)`
  - `EEPROM_ReadUint16(addr)`
  - `charger_presets_eeprom()` : charge un tableau `presets[3]` depuis l’EEPROM.

- `presets.c` / `presets.h`  
  Gestion des trois presets FM :
  - `sauvegarder_preset(index)`
  - `charger_preset(index)`
  - Tableau global `presets[3]`.

### Dossiers `Drivers/`

- `Drivers/ssd1306` : bibliothèque SSD1306 (origine 4ilo/afiskon, I2C/SPI, polices, symboles).
- `Drivers/RDA_5807` : driver bas niveau du tuner RDA5807M (accès registres, tune, seek, volume).
- `Drivers/STM32F2xx_HAL_Driver`, `Drivers/CMSIS` : HAL et fichiers CMSIS fournis par ST.

---

## 3. Logique de l’application

### 3.1 États principaux

L’application fonctionne comme une machine d’états simple :

- `menu_actif == 1` → **Menu principal** (page 0)
- `menu_actif == 0` → **Page active** (1 à 6)

```text
Page 0 : Menu
Page 1 : Radio FM
Page 2 : Recherche (seek)
Page 3 : Presets
Page 4 : Audio (bass boost)
Page 5 : Paramètres
Page 6 : Informations
```

### 3.2 Rôle des boutons

- **Bouton gauche (1)** :
  - Dans le menu : option précédente (avec boucle 0 ↔ 5).
  - Page Radio : fréquence précédente (`RDA_ManualDown`).
  - Page Recherche : recherche station précédente (`RDA_Seek` direction down).
  - Page Presets : preset précédent.
  - Page Audio : toggle bass boost (gauche ou droite).

- **Bouton droite (2)** :
  - Dans le menu : option suivante (avec boucle 5 ↔ 0).
  - Page Radio : fréquence suivante (`RDA_ManualUp`).
  - Page Recherche : recherche station suivante (`RDA_Seek` direction up).
  - Page Presets : preset suivant.
  - Page Audio : toggle bass boost (gauche ou droite).

- **Bouton centre (3)** :
  - Dans le menu : valide l’option → bascule vers la page correspondante.
  - Page Radio : retour au menu (sélection sur Radio).
  - Page Recherche : retour à la page Radio.
  - Page Presets :
    - Si preset vide → sauvegarde la fréquence courante.
    - Si preset existant → charge la fréquence et revient à la page Radio.
  - Page Audio : retour au menu (sélection sur Audio).
  - Pages Param/Info : retour au menu (sélection sur la page courante).

---

## 4. Pages de l’interface

### 4.1 Menu (page 0)

- Variable `menu_selection` de 0 à 5 :
  - 0 : Radio FM
  - 1 : Recherche
  - 2 : Presets
  - 3 : Audio
  - 4 : Paramètres
  - 5 : Info
- Gauche/droite déplacent la sélection, centre ouvre la page.

### 4.2 Radio FM (page 1)

- Affiche la fréquence actuelle et la qualité de réception.
- Gauche/droite : fréquence manuelle `RDA_ManualDown/Up()`, mise à jour écran.
- Centre : retour au menu.

### 4.3 Recherche (page 2)

- Gauche : `RDA_Seek` vers la station précédente.
- Droite : `RDA_Seek` vers la station suivante.
- Centre : retour à la page Radio FM.

### 4.4 Presets (page 3)

- 3 presets : indices 0, 1, 2.
- Gauche/droite : sélection du preset.
- Centre :
  - Si `presets[index] == 0` → sauvegarde de la fréquence courante dans l’EEPROM.
  - Sinon → charge le preset et retourne à la page Radio FM.

### 4.5 Audio (page 4)

- Gauche ou droite : bascule d’un mode audio (ex. `bass_boost_actif`).
- Centre : retour au menu (sur l’option Audio).

### 4.6 Paramètres / Info (pages 5 et 6)

- Contenu essentiellement statique (version firmware, infos système, etc.).
- Centre : retour au menu (sélection sur Paramètres ou Info).

---

## 5. Compilation et flash

Le projet est prévu pour être compilé avec **STM32CubeIDE** :

1. Ouvrir le fichier `.ioc` dans STM32CubeMX / CubeIDE pour vérifier la configuration.
2. Générer ou régénérer le code si nécessaire.
3. Compiler et flasher la carte STM32F2 via ST-Link.

Les drivers et le code HAL sont déjà présents dans le projet.

---

## 6. Pistes d’amélioration

- **Supprimer les `HAL_Delay()` bloquants** dans la boucle principale et autour des actions utilisateur :
  - Remplacement par une logique de type `millis()` via `HAL_GetTick()` (gestion d’intervalle non bloquante).
- **Utiliser les interruptions EXTI** pour les boutons (plutôt que polling dans la boucle) pour une meilleure réactivité.
- **Introduire un RTOS (ex. Zephyr ou FreeRTOS)** si le projet doit évoluer :
  - Une tâche pour l’affichage,
  - Une pour la radio,
  - Une pour la gestion des entrées/sorties.
- **Améliorer l’interface utilisateur** :
  - Ajout de volume, RDS, icônes plus lisibles, animations, etc.

---

## 7. Licence et crédits

- Bibliothèque **SSD1306** : projet open source (4ilo / afiskon).
- Bibliothèque **RDA5807** : basée sur les travaux de Ricardo Lima Caratti (pu2clr) et adaptations STM32.
- HAL STM32 et CMSIS : STMicroelectronics.

Voir les en-têtes de fichiers dans `Drivers/` pour les détails de licence.
