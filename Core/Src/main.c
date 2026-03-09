/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "ssd1306_symbols.h"
#include "ssd1306_conf.h"
#include "ssd1306_tests.h"
#include "RDA_5807.h"
#include "boutons.h"
#include "eeprom.h"
#include "radio.h"
#include "presets.h"
#include "affichage.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_UART4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Variables d'état de l'IHM et de l'application */
// Navigation dans le menu principal
uint8_t menu_actif = 0;           /* 0 = page fonctionnelle, 1 = menu affiché */
uint8_t menu_selection = 0;       /* index de l'élément surligné dans le menu (0–5) */

// Page actuellement affichée (0 = menu, 1 = radio, 2 = recherche, 3 = presets, ...)
uint8_t page_actuelle = 0;

// Informations firmware
const char* version_firmware = "v1.0.0";

// Tableau de 3 presets (fréquences en centièmes de MHz, ex: 8750 = 87.50 MHz)
uint16_t presets[3] = {0, 0, 0};

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_UART4_Init();
  /* USER CODE BEGIN 2 */
  ssd1306_Init();
  RDA_Init(&hi2c1);
  HAL_Delay(10);
  Presets_Load();
  RDA_Tune(&hi2c1, charger_freq_eeprom());

  // Afficher la page Radio en premier (page principale)
  page_actuelle = 1;
  menu_actif = 0;  // Commencer sur la page Radio
  Affiche_Page(page_actuelle);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//    /* USER CODE END WHILE */



    // Navigation avec les boutons
    uint8_t btn = gerer_bouton();

        if (menu_actif == 1) {
            // On est dans le menu (page 0)
            if (btn == 1) {
                // Bouton gauche : Option précédente
                if (menu_selection > 0) {
                    menu_selection--;
                } else {
                    menu_selection = 5;  // Boucler à la fin
                }
                Affiche_Page(0);  // Réafficher le menu
            }
            else if (btn == 2) {
                // Bouton droite : Option suivante
                menu_selection++;
                if (menu_selection >= 6) {
                    menu_selection = 0;  // Boucler au début
                }
                Affiche_Page(0);  // Réafficher le menu
            }
            else if (btn == 3) {
                // Bouton centre : Aller à la page sélectionnée
                // menu_selection 0 = Radio FM (page 1)
                // menu_selection 1 = Recherche (page 2)
                // menu_selection 2 = Presets (page 3)
                // menu_selection 3 = Audio (page 4)
                // menu_selection 4 = Paramètres (page 5)
                // menu_selection 5 = Info (page 6)
                page_actuelle = menu_selection + 1;  // +1 car page 0 = menu
                menu_actif = 0;  // Sortir du menu
                Affiche_Page(page_actuelle);
            }
        } else {
            // On est sur une page spécifique (pas le menu)
            if (page_actuelle == 1) {
                // Page Radio FM
                if (btn == 1) {
                    RDA_ManualDown(&hi2c1);
                    HAL_Delay(200);
                    sauver_freq_eeprom(RDA_GetRealFrequency(&hi2c1));
                    mettre_a_jour_frequence();
                    signal(RDA_GetQuality(&hi2c1));
                    ssd1306_UpdateScreen();
                }
                else if (btn == 2) {
                    RDA_ManualUp(&hi2c1);
                    HAL_Delay(200);
                    sauver_freq_eeprom(RDA_GetRealFrequency(&hi2c1));
                    mettre_a_jour_frequence();
                    signal(RDA_GetQuality(&hi2c1));
                    ssd1306_UpdateScreen();
                }
                else if (btn == 3) {
                    // Bouton centre : Retour au menu
                    page_actuelle = 0;
                    menu_actif = 1;
                    menu_selection = 0;
                    Affiche_Page(0);
                }
            }
            else if (page_actuelle == 4) {
                // Page Audio - Toggle Bass Boost
                if (btn == 1 || btn == 2) {
                    // Bouton gauche ou droite : Toggle Bass Boost
                    bass_boost_actif = !bass_boost_actif;
                    Affiche_Page(4);  // Réafficher la page
                }
                else if (btn == 3) {
                    // Bouton centre : Retour au menu
                    page_actuelle = 0;
                    menu_actif = 1;
                    menu_selection = 3;  // Rester sur l'option Audio
                    Affiche_Page(0);
                }
            }
            else if (page_actuelle == 2) {
                // Page Recherche
                if (btn == 1) {
                    RDA_Seek(&hi2c1, 0, 0, NULL);
                    HAL_Delay(200);
                    sauver_freq_eeprom(RDA_GetRealFrequency(&hi2c1));
                    afficher_page_recherche();
                }
                else if (btn == 2) {
                    RDA_Seek(&hi2c1, 0, 1, NULL);
                    HAL_Delay(200);
                    sauver_freq_eeprom(RDA_GetRealFrequency(&hi2c1));
                    afficher_page_recherche();
                }
                else if (btn == 3) {
                    // Bouton centre : Retour à la radio
                    page_actuelle = 1;  // Page Radio FM
                    menu_actif = 0;
                    Affiche_Page(1);
                }
            }
            else if (page_actuelle == 3) {
                // Page Presets
                if (btn == 1) {
                    // Bouton gauche : Preset précédent
                    if (preset_selection > 0) {
                        preset_selection--;
                    } else {
                        preset_selection = 2;  // Boucler à la fin
                    }
                    afficher_page_presets();
                }
                else if (btn == 2) {
                    // Bouton droite : Preset suivant
                    preset_selection++;
                    if (preset_selection >= 3) {
                        preset_selection = 0;  // Boucler au début
                    }
                    afficher_page_presets();
                }
                else if (btn == 3) {
                    // Bouton centre : Charger, sauvegarder ou écraser
                    if (PRESET_VIDE(presets[preset_selection])) {
                        sauvegarder_preset(preset_selection);
                        afficher_page_presets();
                    } else {
                        if (centre_appui_long()) {
                            // Appui long = écraser avec la freq actuelle
                            sauvegarder_preset(preset_selection);
                            afficher_page_presets();
                        } else {
                            // Appui court = charger et aller à la radio
                            charger_preset(preset_selection);
                            sauver_freq_eeprom(RDA_GetRealFrequency(&hi2c1));
                            page_actuelle = 1;
                            menu_actif = 0;
                            Affiche_Page(1);
                        }
                    }
                }
            }
            else {
                // Autres pages (Paramètres, Info)
                if (btn == 3) {
                    // Bouton centre : Retour au menu
                    menu_actif = 1;
                    // Garder la sélection correspondante à la page actuelle
                    if (page_actuelle > 0 && page_actuelle <= 6) {
                        menu_selection = page_actuelle - 1;
                    }
                    page_actuelle = 0;
                    Affiche_Page(0);
                }
            }
        }

	  HAL_Delay(10);
    /* USER CODE BEGIN 3 */
  }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV6;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
#define RTC_MAGIC_INIT 0x5A5AU
  /* Ne régler l’heure qu’au premier démarrage (domaine backup réinitialisé) */
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != RTC_MAGIC_INIT) {
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, RTC_MAGIC_INIT);
  }
#undef RTC_MAGIC_INIT
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pins : PC3 (Bouton Gauche) */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;  // Pull-up : bouton connecté à GND
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 (Bouton Centre) et PA1 (Bouton Droite) */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;  // Pull-up : bouton connecté à GND
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
