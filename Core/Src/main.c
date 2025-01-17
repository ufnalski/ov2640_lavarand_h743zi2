/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "crc.h"
#include "dcmi.h"
#include "dma.h"
#include "i2c.h"
#include "mbedtls.h"
#include "memorymap.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mbedtls/sha256.h"
#include "ov2640_jpeg.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UART_RANDOM_STREAM &huart2
#define USE_CRC32 0 // 1 -> CRC32, 0 -> SHA256
#define PRNG 0 // 1 -> do not use camera, 0 -> use camera
#define NUMBER_OF_BITS_TO_STREAM 1000000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
unsigned char jpeg_picture_hash[32]; /* SHA-256 outputs 32 bytes */

uint32_t jpeg_picture_crc; /* CRC32 */

extern uint8_t pictureBuff[JPEG_BUFF_SIZE];

uint32_t jpeg_begin;
uint32_t jpeg_end;

extern uint8_t uart_message[256];
extern uint32_t uart_size;

uint16_t number_of_config_errors;

uint32_t current_bit = 0;

uint32_t deterministic_seed_modifier = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
	MX_DMA_Init();
	MX_USART3_UART_Init();
	MX_USART2_UART_Init();
	MX_DCMI_Init();
	MX_TIM1_Init();
	MX_I2C4_Init();
	MX_MBEDTLS_Init();
	MX_CRC_Init();
	/* USER CODE BEGIN 2 */

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1); // OV2640 XCLK
	HAL_Delay(1);

	number_of_config_errors = OV2640_Init_JPEG();
	uart_size =
			sprintf((char*) uart_message,
					"Number of OV2640 memory write/read errors: %d.\r\nLet's continue and see what happens :)\r\n",
					number_of_config_errors);
	HAL_UART_Transmit(OV2640_DEBUG_UART, uart_message, uart_size, 10);
	HAL_Delay(4000);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
#if PRNG
		mbedtls_sha256(pictureBuff, 50000, jpeg_picture_hash, 0); // arbitrary 50k bits
		deterministic_seed_modifier++;
		HAL_Delay(1); // to help Live Expressions
		for (uint8_t i = 0; i < 4; i++)
		{
			pictureBuff[i] = deterministic_seed_modifier >> (8 * i);
		}
		HAL_UART_Transmit(OV2640_DEBUG_UART, (uint8_t*) "SHA-256: ",
				sizeof("SHA-256: "), 10);

		for (uint8_t i = 0; i < 32; i++)
		{
			uart_size = sprintf((char*) uart_message, "%02x",
					jpeg_picture_hash[i]);
			HAL_UART_Transmit(OV2640_DEBUG_UART, uart_message, uart_size, 10);
		}
		HAL_UART_Transmit(OV2640_DEBUG_UART, (uint8_t*) "\r\n", sizeof("\r\n"),
				10);

		for (uint8_t i = 0; i < 32; i++)
		{
			for (uint8_t j = 0; j < 8; j++)
			{

				if (current_bit < NUMBER_OF_BITS_TO_STREAM)
				{
					if (((jpeg_picture_hash[i] >> j) & 0x1) == 1)
					{
						uint8_t bit1 = '1';
						HAL_UART_Transmit(UART_RANDOM_STREAM, &bit1, 1, 10);
						current_bit++;
					}
					else
					{
						uint8_t bit0 = '0';
						HAL_UART_Transmit(UART_RANDOM_STREAM, &bit0, 1, 10);
						current_bit++;
					}
				}
				else
				{
					HAL_UART_Transmit(OV2640_DEBUG_UART,
							(uint8_t*) "All requested bits have been streamed.\r\n",
							sizeof("All requested bits have been streamed.\r\n"),
							10);
					HAL_Delay(2000);
				}
			}
		}
#else
		HAL_DCMI_Stop(&hdcmi);
		HAL_Delay(10);

		if (0 == OV2640_TakePicture(&jpeg_begin, &jpeg_end))
		{
#if USE_CRC32
			jpeg_picture_crc = HAL_CRC_Calculate(&hcrc,
					(uint32_t*) (pictureBuff + jpeg_begin),
					jpeg_end - jpeg_begin + 1);
			HAL_UART_Transmit(OV2640_DEBUG_UART, (uint8_t*) "CRC-32: ",
					sizeof("CRC-32: "), 10);
			uart_size = sprintf((char*) uart_message, "%08x", jpeg_picture_crc);
			HAL_UART_Transmit(OV2640_DEBUG_UART, uart_message, uart_size, 10);
			HAL_UART_Transmit(OV2640_DEBUG_UART, (uint8_t*) "\r\n",
					sizeof("\r\n"), 10);
			for (uint8_t j = 0; j < 32; j++)
			{

				if (current_bit < NUMBER_OF_BITS_TO_STREAM)
				{
					if (((jpeg_picture_crc >> j) & 0x1) == 1)
					{
						uint8_t bit1 = '1';
						HAL_UART_Transmit(UART_RANDOM_STREAM, &bit1, 1, 10);
						current_bit++;
					}
					else
					{
						uint8_t bit0 = '0';
						HAL_UART_Transmit(UART_RANDOM_STREAM, &bit0, 1, 10);
						current_bit++;
					}
				}
				else
				{
					HAL_UART_Transmit(OV2640_DEBUG_UART,
							(uint8_t*) "All requested bits have been streamed.\r\n",
							sizeof("All requested bits have been streamed.\r\n"),
							10);
					HAL_Delay(2000);
				}
			}
#else
			mbedtls_sha256(pictureBuff + jpeg_begin, jpeg_end - jpeg_begin + 1,
					jpeg_picture_hash, 0);
			HAL_UART_Transmit(OV2640_DEBUG_UART, (uint8_t*) "SHA-256: ",
					sizeof("SHA-256: "), 10);

			for (uint8_t i = 0; i < 32; i++)
			{
				uart_size = sprintf((char*) uart_message, "%02x",
						jpeg_picture_hash[i]);
				HAL_UART_Transmit(OV2640_DEBUG_UART, uart_message, uart_size,
						10);
			}
			HAL_UART_Transmit(OV2640_DEBUG_UART, (uint8_t*) "\r\n",
					sizeof("\r\n"), 10);

			for (uint8_t i = 0; i < 32; i++)
			{
				for (uint8_t j = 0; j < 8; j++)
				{

					if (current_bit < NUMBER_OF_BITS_TO_STREAM)
					{
						if (((jpeg_picture_hash[i] >> j) & 0x1) == 1)
						{
							uint8_t bit1 = '1';
							HAL_UART_Transmit(UART_RANDOM_STREAM, &bit1, 1, 10);
							current_bit++;
						}
						else
						{
							uint8_t bit0 = '0';
							HAL_UART_Transmit(UART_RANDOM_STREAM, &bit0, 1, 10);
							current_bit++;
						}
					}
					else
					{
						HAL_UART_Transmit(OV2640_DEBUG_UART,
								(uint8_t*) "All requested bits have been streamed.\r\n",
								sizeof("All requested bits have been streamed.\r\n"),
								10);
						HAL_Delay(2000);
					}
				}
			}
#endif // USE_CRC32
	}
#endif // PRNG
	/* USER CODE END WHILE */

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
RCC_OscInitTypeDef RCC_OscInitStruct =
{ 0 };
RCC_ClkInitTypeDef RCC_ClkInitStruct =
{ 0 };

/** Supply configuration update enable
 */
HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

/** Configure the main internal regulator output voltage
 */
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
{
}

__HAL_RCC_SYSCFG_CLK_ENABLE();
__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY))
{
}

/** Initializes the RCC Oscillators according to the specified parameters
 * in the RCC_OscInitTypeDef structure.
 */
RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
RCC_OscInitStruct.PLL.PLLM = 1;
RCC_OscInitStruct.PLL.PLLN = 120;
RCC_OscInitStruct.PLL.PLLP = 2;
RCC_OscInitStruct.PLL.PLLQ = 15;
RCC_OscInitStruct.PLL.PLLR = 2;
RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
RCC_OscInitStruct.PLL.PLLFRACN = 0;
if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
{
	Error_Handler();
}

/** Initializes the CPU, AHB and APB buses clocks
 */
RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
		| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1
		| RCC_CLOCKTYPE_D1PCLK1;
RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
{
	Error_Handler();
}
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
