#include "boutons.h"
#include "main.h"

#define BUTTON_G_PIN    GPIO_PIN_3
#define BUTTON_G_PORT   GPIOC
#define BUTTON_D_PIN    GPIO_PIN_1
#define BUTTON_D_PORT   GPIOA
#define BUTTON_M_PIN    GPIO_PIN_0
#define BUTTON_M_PORT   GPIOA

static uint8_t prev_gauche = 0, prev_droite = 0, prev_centre = 0;

/**
 * @brief Lit l'état des 3 boutons et renvoie un événement unique.
 *
 * Détecte un front montant sur chaque bouton (gauche, droite, centre)
 * en comparant l'état courant aux variables statiques prev_gauche,
 * prev_droite et prev_centre. Évite ainsi les répétitions tant que
 * le bouton reste appuyé.
 *
 * @retval 0 Aucun nouvel appui détecté
 * @retval 1 Bouton gauche (décrément / précédent)
 * @retval 2 Bouton droite (incrément / suivant)
 * @retval 3 Bouton centre (validation / entrée menu)
 */
uint8_t gerer_bouton(void)
{
    uint8_t etat = 0;

    uint8_t g = (HAL_GPIO_ReadPin(BUTTON_G_PORT, BUTTON_G_PIN) == GPIO_PIN_RESET);
    uint8_t d = (HAL_GPIO_ReadPin(BUTTON_D_PORT, BUTTON_D_PIN) == GPIO_PIN_RESET);
    uint8_t m = (HAL_GPIO_ReadPin(BUTTON_M_PORT, BUTTON_M_PIN) == GPIO_PIN_RESET);

    if (g && !prev_gauche) etat = 1;
    else if (d && !prev_droite) etat = 2;
    else if (m && !prev_centre) etat = 3;

    prev_gauche  = g;
    prev_droite  = d;
    prev_centre  = m;

    return etat;
}

/**
 * @brief Teste si le bouton centre est maintenu appuyé (appui long).
 *
 * Appelé juste après un appui sur le bouton centre. La fonction
 * attend environ 400 ms en vérifiant régulièrement l'état du bouton.
 * Si le bouton est relâché avant la fin, l'appui est considéré comme court.
 *
 * @retval 1 Appui long détecté (centre encore appuyé après ~400 ms)
 * @retval 0 Appui court (relâché avant la fin du délai)
 */
uint8_t centre_appui_long(void)
{
    for (uint8_t i = 0; i < 40; i++) {
        HAL_Delay(10);
        if (HAL_GPIO_ReadPin(BUTTON_M_PORT, BUTTON_M_PIN) != GPIO_PIN_RESET)
            return 0;  /* relâché = appui court */
    }
    return 1;  /* toujours appuyé = appui long */
}
