#include "radio.h"
#include "RDA_5807.h"
#include "main.h"

/**
 * @brief État global du Bass Boost.
 *
 * Variable partagée entre l'IHM (affichage et interaction utilisateur)
 * et ce module radio, qui applique réellement le Bass Boost sur le RDA5807.
 */
uint8_t bass_boost_actif = 0;

/**
 * @brief Applique le réglage de Bass Boost au tuner RDA5807.
 *
 * Lit la variable globale bass_boost_actif et appelle RDA_SetBass()
 * avec TRUE ou FALSE pour activer/désactiver le renforcement des basses.
 * À appeler après un changement de bass_boost_actif (page Audio).
 */
void appliquer_bass_boost(void)
{
    if (bass_boost_actif) {
        RDA_SetBass(&hi2c1, TRUE);
    } else {
        RDA_SetBass(&hi2c1, FALSE);
    }
}
