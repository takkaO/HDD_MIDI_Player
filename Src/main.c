/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sc1602.h"
#include "midi_hdd.h"
#include "eeprom.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEFAULT_PROFILE 0x0000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
enum OPERATION_STATE {
	OP_WAIT,
	OP_SWITCH1_BUSY,
	OP_SWITCH2_BUSY,
	OP_UPDATE_DISPLAY,
	OP_SELECT_SETTING,
	_NOP_SELECT_START,
	OP_CONTROL_VOLUME,
	OP_CHANGE_PLAY_METHOD,
	OP_SWITCH_EN_DIS_HDD,
	OP_SAVE_SETTINGS,
	OP_SYSTEM_RESET,
	OP_DISPLAY_HDD_STATE,
	_NOP_SELECT_END
};

enum SELECTION {
	NO, YES
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
PlayMethod PlayMethod_Config() {
	PlayMethod pm = FIXED;
	LCD_printf(1, "Play Method");
	LCD_printf(2, "> FIXED");
	while (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)) {
		if (!HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)) {
			if (pm == FIXED) {
				pm = FLEXIBLE;
				LCD_Clear();
				LCD_printf(1, "Play Method");
				LCD_printf(2, "> FLEXIBLE");
			} else if (pm == FLEXIBLE) {
				pm = FIXED;
				LCD_Clear();
				LCD_printf(1, "Play Method");
				LCD_printf(2, "> FIXED");
			}
			while (!HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin))
				;
		}

	}
	while (!HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin))
		;
	return pm;
}

void HDD_Config(uint8_t* ea) {
	int i;
	for (i = 0; i < MAX_HDD_NUM; i++) {
		ea[i] = ENABLE;
		LCD_Clear();
		LCD_printf(1, "HDD/TIM:%d use?", i + 1);
		LCD_printf(2, "> ENABLE");
		while (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin)) {
			if (!HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin)) {
				ea[i] = !ea[i];
				if (ea[i] == ENABLE) {
					LCD_Clear();
					LCD_printf(1, "HDD/TIM:%d use?", i + 1);
					LCD_printf(2, "> ENABLE");
				} else {
					LCD_Clear();
					LCD_printf(1, "HDD/TIM:%d use?", i + 1);
					LCD_printf(2, "> DISABLE");
				}
				while (!HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin))
					;
			}
		}
		while (!HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin))
			;
	}
}

void Show_HDD_Status() {
	LCD_Clear();
	LCD_Set_Position(1, 1);
	LCD_puts("HDD/TIM:");
	char s[64];
	for (int i = 0; i < MAX_HDD_NUM; i++) {
		snprintf(s, 64, "%c ", Is_HDD_Enable(i) ? 'o' : 'x');
		LCD_puts(s);
	}
	if (Get_Volume() == 100) {
		LCD_printf(2, "%-8s Vol:MAX", (Get_PlayMethod() == FIXED ? "FIXED" : "FLEXIBLE"));
	} else {
		LCD_printf(2, "%-8s Vol:%2d%%", (Get_PlayMethod() == FIXED ? "FIXED" : "FLEXIBLE"),
				Get_Volume());
	}
}

uint8_t Check_Data_Structure(DATA_STRUCTURE ds) {
	if (ds.BYTE.play_method != FIXED && ds.BYTE.play_method != FLEXIBLE) {
		return 0;
	}
	if (100 < ds.BYTE.volume) {
		return 0;
	}
	if (ds.BYTE.hdd1 != DISABLE && ds.BYTE.hdd1 != ENABLE) {
		return 0;
	}
	if (ds.BYTE.hdd2 != DISABLE && ds.BYTE.hdd2 != ENABLE) {
		return 0;
	}
	if (ds.BYTE.hdd3 != DISABLE && ds.BYTE.hdd3 != ENABLE) {
		return 0;
	}
	if (ds.BYTE.hdd4 != DISABLE && ds.BYTE.hdd4 != ENABLE) {
		return 0;
	}
	return 1;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
	MX_I2C1_Init();
	MX_TIM1_Init();
	MX_TIM2_Init();
	MX_TIM3_Init();
	MX_TIM4_Init();
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USB_PCD_Init();
	MX_SPI1_Init();
	/* USER CODE BEGIN 2 */
	LCD_Initialize();
	LCD_Change_Mode(SYNC);
	LCD_Clear();
	LCD_printf(1, "Booting...");

	MIDI_Variable_Initialize();

	EEPROM_Initialize(&hi2c1);

	HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

	/* System initialize */
	PlayMethod pm = FIXED;
	uint8_t ea[MAX_HDD_NUM] = { 1 };
	TIM_HandleTypeDef htims[MAX_HDD_NUM] = { htim1, htim2, htim3, htim4 };

	DATA_STRUCTURE ds = Load_Settings(DEFAULT_PROFILE, EEPROM_DATA_LEN);
	if (Check_Data_Structure(ds)) {
		pm = ds.BYTE.play_method;
		for (int i = 0; i < MAX_HDD_NUM; i++) {
			ea[i] = *(&ds.BYTE.hdd1 + i);
		}
		Change_Volume(ds.BYTE.volume);
	}
	Change_PlayMethod(pm);
	for (int hdd_num = 0; hdd_num < MAX_HDD_NUM; hdd_num++) {
		Initialize_HDD(hdd_num, htims[hdd_num], ea[hdd_num]);
	}

	HAL_UART_Receive_IT(&huart1, &megv.uart_rx_data, 1);

	Show_HDD_Status();
	LCD_Change_Mode(ASYNC);

	uint8_t volume_step = 5;
	uint8_t hdd_num;
	uint8_t volume = Get_Volume();
	enum SELECTION yes_no = NO;
	enum OPERATION_STATE op_state = OP_WAIT;
	enum OPERATION_STATE next_operation = OP_WAIT;
	enum OPERATION_STATE tmp_operation;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		switch (op_state) {
		case OP_WAIT:
			if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET) {
				next_operation = OP_SELECT_SETTING;
				tmp_operation = _NOP_SELECT_START;
				op_state = OP_SWITCH1_BUSY;
			}
			break;
		case OP_SELECT_SETTING:
			if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET) {
				tmp_operation += 1;
				if (tmp_operation == _NOP_SELECT_END) {
					tmp_operation = _NOP_SELECT_START + 1;
				}
				op_state = OP_SWITCH1_BUSY;
				next_operation = OP_SELECT_SETTING;
			} else if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET) {
				if (tmp_operation == _NOP_SELECT_START) {
					tmp_operation += 1;
					next_operation = OP_SELECT_SETTING;
				} else {
					next_operation = tmp_operation;
				}
				op_state = OP_SWITCH2_BUSY;
			}
			break;
		case OP_CONTROL_VOLUME:
			/* Adjust volume */
			if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET) {
				volume =
						(Get_Volume() + volume_step) > 100 ?
								volume_step : Get_Volume() + volume_step;
				op_state = OP_SWITCH1_BUSY;
				next_operation = OP_CONTROL_VOLUME;
			} else if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET) {
				Change_Volume(volume);
				op_state = OP_SWITCH2_BUSY;
				next_operation = OP_DISPLAY_HDD_STATE;
			}
			break;
		case OP_CHANGE_PLAY_METHOD:
			/* Alter play method */
			if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET) {
				pm = (pm == FIXED ? FLEXIBLE : FIXED);
				op_state = OP_SWITCH1_BUSY;
				next_operation = OP_CHANGE_PLAY_METHOD;
			} else if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET) {
				Change_PlayMethod(pm);
				op_state = OP_SWITCH2_BUSY;
				next_operation = OP_DISPLAY_HDD_STATE;
			}
			break;
		case OP_SWITCH_EN_DIS_HDD:
			/* Switch HDD Enable/Disable */
			if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET) {
				hdd_num = ((hdd_num + 1) == MAX_HDD_NUM ? 0 : hdd_num + 1);
				op_state = OP_SWITCH1_BUSY;
				next_operation = OP_SWITCH_EN_DIS_HDD;
			} else if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET) {
				Enable_HDD(hdd_num, (Is_HDD_Enable(hdd_num) == ENABLE ? DISABLE : ENABLE));
				op_state = OP_SWITCH2_BUSY;
				next_operation = OP_DISPLAY_HDD_STATE;
			}
			break;
		case OP_SAVE_SETTINGS:
			/* Save current settings */
			if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET) {
				yes_no = (yes_no == YES) ? NO : YES;
				op_state = OP_SWITCH1_BUSY;
				next_operation = OP_SAVE_SETTINGS;
			} else if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET) {
				if (yes_no == YES) {
					// TODO: Implement non-blocking mode
					Change_PlayMethod(Get_PlayMethod());	// STOP Melody
					ds.BYTE.play_method = Get_PlayMethod();
					ds.BYTE.volume = Get_Volume();
					for (int i = 0; i < MAX_HDD_NUM; i++) {
						*(&ds.BYTE.hdd1 + i) = Is_HDD_Enable(i);
					}
					ds.BYTE.addr_high = 0x00;
					ds.BYTE.addr_low = 0x00;
					Save_Settings(ds, EEPROM_DATA_STRUCTURE_LEN);
				}
				yes_no = NO;	// reset state
				op_state = OP_SWITCH2_BUSY;
				next_operation = OP_DISPLAY_HDD_STATE;
			}
			break;
		case OP_SYSTEM_RESET:
			/* Load saved settings */
			if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_RESET) {
				yes_no = (yes_no == YES) ? NO : YES;
				op_state = OP_SWITCH1_BUSY;
				next_operation = OP_SYSTEM_RESET;
			} else if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_RESET) {
				if (yes_no == YES) {
					HAL_NVIC_SystemReset();
					// System reset!
				}
			}
			break;
		case OP_DISPLAY_HDD_STATE:
			op_state = OP_UPDATE_DISPLAY;
			next_operation = OP_WAIT;
			break;
		case OP_UPDATE_DISPLAY:
			/* Update LCD Display */
			switch (next_operation) {
			case OP_WAIT:
				break;
			case OP_SELECT_SETTING:
				LCD_Clear();
				LCD_printf(1, "Select operate");

				/* OP_SELECT_SETTING START */
				switch (tmp_operation) {
				case _NOP_SELECT_START:
					LCD_printf(2, "sel:SW1 ent:SW2");
					break;
				case OP_CONTROL_VOLUME:
					LCD_printf(2, "> Control volume");
					break;
				case OP_CHANGE_PLAY_METHOD:
					LCD_printf(2, "> Alter p-method");
					pm = Get_PlayMethod();
					break;
				case OP_SWITCH_EN_DIS_HDD:
					LCD_printf(2, "> HDD en/dis");
					hdd_num = 0;
					break;
				case OP_DISPLAY_HDD_STATE:
					LCD_printf(2, "> Return");
					break;
				default:
					LCD_printf(2, "Error (;w;)");
					break;
				}
				/* OP_SELECT_SETTING END */
				break;
			case OP_CONTROL_VOLUME:
				LCD_Clear();
				LCD_printf(1, "Control volume");
				LCD_printf(2, "Volume %3d %%", Get_Volume());
				break;
			case OP_CHANGE_PLAY_METHOD:
				LCD_Clear();
				LCD_printf(1, "Alter PlayMethod");
				LCD_printf(2, "Method: %s", pm == FIXED ? "FIXED" : "FLEXIBLE");
				break;
			case OP_SWITCH_EN_DIS_HDD:
				LCD_Clear();
				LCD_printf(1, "Switch HDD en/dis");
				LCD_printf(2, "HDD number %d", hdd_num + 1);
				break;
			case OP_SAVE_SETTINGS:
				LCD_Clear();
				LCD_printf(1, "Save settings");
				LCD_printf(2, "save? %s", yes_no == YES ? "YES" : "NO");
				break;
			case OP_SYSTEM_RESET:
				LCD_Clear();
				LCD_printf(1, "System reset");
				LCD_printf(2, "reset? %s", yes_no == YES ? "YES" : "NO");
				break;
			case OP_DISPLAY_HDD_STATE:
				Show_HDD_Status();
				break;
			default:
				break;
			}
			op_state = next_operation;
			break;
		case OP_SWITCH1_BUSY:
			if (HAL_GPIO_ReadPin(SW1_GPIO_Port, SW1_Pin) == GPIO_PIN_SET) {
				op_state = OP_UPDATE_DISPLAY;
			}
			break;
		case OP_SWITCH2_BUSY:
			if (HAL_GPIO_ReadPin(SW2_GPIO_Port, SW2_Pin) == GPIO_PIN_SET) {
				op_state = OP_UPDATE_DISPLAY;
			}
			break;
		default:
			break;
		}
		LCD_Loop();

		Parse_MIDI();
		Play_MIDI();
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1
			| RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (megv.uart_rx_data == 0xFE) {
		/* Through active sensing */
		// Prepare for next RX interrupt
		HAL_UART_Receive_IT(huart, &megv.uart_rx_data, 1);
		return;
	}
	HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
	megv.Buffer[megv.rxp] = megv.uart_rx_data;
	megv.rxp = AddCounter(megv.rxp, RAW_DATA_BUF_LEN);

	//Prepare for next RX interrupt
	HAL_UART_Receive_IT(huart, &megv.uart_rx_data, 1);
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
	 tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
