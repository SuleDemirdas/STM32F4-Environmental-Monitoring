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
#include "i2c_core.h"
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
I2C_HandleTypeDef hi2c3;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C3_Init(void);
static void MX_TIM3_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int8_t stm32_i2c_read_wrapper(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	if (reg_addr == 0)
	{
	        if (HAL_I2C_Master_Receive(&hi2c3, dev_addr, data, len, 100) == HAL_OK)
	        {
	            return 0;
	        }
	    }
    if(HAL_I2C_Mem_Read(&hi2c3, dev_addr, reg_addr, 1, data, len, 100) == HAL_OK)
	{
    	return 0;
	}
    return -1;
}

int8_t stm32_i2c_write_wrapper(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len) {
	if(reg_addr == 0)
	{
		if (HAL_I2C_Master_Transmit(&hi2c3, dev_addr, data, len, 100) == HAL_OK)
		{
			return 0;
		}
	}
    if(HAL_I2C_Mem_Write(&hi2c3, dev_addr, reg_addr, 1, data, len, 100) == HAL_OK)
	{
    	return 0;
	}
    return -1;
}

void stm32_delay_wrapper(uint32_t ms) {
    HAL_Delay(ms);
}

BMP180_HandleTypeDef hbmp180;
BH1750_HandleTypeDef hbh1750;
AHT20_HandleTypeDef	haht20;

Filter_Handle_t hFiltHum, hFiltTemp, hFiltLight;

buf_handle_t hBufHum, hBufTemp, hBufLight;
Sensor_Stats_t hum_stats, temp_stats, light_stats;

#define RING_BUFFER_SIZE 30
float storageHum[RING_BUFFER_SIZE];
float storageTemp[RING_BUFFER_SIZE];
float storageLight[RING_BUFFER_SIZE];

uint8_t seconds_counter = 0;
volatile uint8_t read_sensor_flag = 0;

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
  MX_I2C3_Init();
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  hbmp180.i2c_read = stm32_i2c_read_wrapper;
  hbmp180.i2c_write = stm32_i2c_write_wrapper;
  hbmp180.delay_ms = stm32_delay_wrapper;
  hbmp180.oss = 0;
  BMP180_Init(&hbmp180);

  hbh1750.address = BH1750_ADDRESS_GND;
  hbh1750.delay_ms = stm32_delay_wrapper;
  hbh1750.i2c_read = stm32_i2c_read_wrapper;
  hbh1750.i2c_write = stm32_i2c_write_wrapper;
  hbh1750.mode = BH1750_ONE_TIME_H_RES_MODE;
  BH1750_Init(&hbh1750);

  haht20.delay_ms = stm32_delay_wrapper;
  haht20.i2c_read = stm32_i2c_read_wrapper;
  haht20.i2c_write = stm32_i2c_write_wrapper;
  AHT20_Init(&haht20);

  buffer_init(&hBufHum, storageHum, RING_BUFFER_SIZE);
  buffer_init(&hBufTemp, storageTemp, RING_BUFFER_SIZE);
  buffer_init(&hBufLight, storageLight, RING_BUFFER_SIZE);

  HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if(read_sensor_flag)
      {
          float raw_hum = i2c_sensor_read(AHT20_ADDRESS, HUMIDITY_SENSOR, &haht20);
          float filt_hum = filter_sensor_value(&hFiltHum, raw_hum, 5);
          buffer_write_value(&hBufHum, filt_hum);

          float raw_temp = i2c_sensor_read(AHT20_ADDRESS, TEMPERATURE_SENSOR, &haht20);
          float filt_temp = filter_sensor_value(&hFiltTemp, raw_temp, 5);
          buffer_write_value(&hBufTemp, filt_temp);

          float raw_light = i2c_sensor_read(BH1750_ADDRESS, LIGHT_SENSOR, &hbh1750);
          float filt_light = filter_sensor_value(&hFiltLight, raw_light, 5);
          buffer_write_value(&hBufLight, filt_light);

          seconds_counter++;

		  if(seconds_counter >= 30)
			  {
				  calculate_statistics(&hBufHum, &hum_stats);
				  calculate_statistics(&hBufTemp, &temp_stats);
				  calculate_statistics(&hBufLight, &light_stats);

				  char uart_buf[128];
				  sprintf(uart_buf, "HUM:%.2f,%.2f,%.2f,%.2f\r\n", hum_stats.min, hum_stats.max, hum_stats.median, hum_stats.std_dev);
				  HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, strlen(uart_buf), 100);

				  sprintf(uart_buf, "TMP:%.2f,%.2f,%.2f,%.2f\r\n", temp_stats.min, temp_stats.max, temp_stats.median, temp_stats.std_dev);
				  HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, strlen(uart_buf), 100);

				  sprintf(uart_buf, "LUX:%.2f,%.2f,%.2f,%.2f\r\n", light_stats.min, light_stats.max, light_stats.median, light_stats.std_dev);
				  HAL_UART_Transmit(&huart2, (uint8_t*)uart_buf, strlen(uart_buf), 100);

				  seconds_counter = 0;
			  }
		  read_sensor_flag = 0;
      }
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C3_Init(void)
{

  /* USER CODE BEGIN I2C3_Init 0 */

  /* USER CODE END I2C3_Init 0 */

  /* USER CODE BEGIN I2C3_Init 1 */

  /* USER CODE END I2C3_Init 1 */
  hi2c3.Instance = I2C3;
  hi2c3.Init.ClockSpeed = 100000;
  hi2c3.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c3.Init.OwnAddress1 = 0;
  hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c3.Init.OwnAddress2 = 0;
  hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C3_Init 2 */

  /* USER CODE END I2C3_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 12499;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_GREEN_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM3)
	{
		read_sensor_flag = 1;
		HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin );
	}
}

void calculate_statistics(buf_handle_t *p_handle, Sensor_Stats_t *stats)
{
    if (p_handle->count == 0)
	{
    	return;
	}

    float temp_arr[RING_BUFFER_SIZE];
    float sum = 0.0f;

    stats->min = p_handle->buffer[0];
    stats->max = p_handle->buffer[0];

    for (uint16_t i = 0; i < p_handle->count; i++) {
        float val = p_handle->buffer[i];
        temp_arr[i] = val;
        sum += val;

        if (val < stats->min)
		{
        	stats->min = val;
		}
        if (val > stats->max)
		{
        	stats->max = val;
		}
    }

    float mean = sum / p_handle->count;
    float variance_sum = 0.0f;

    for (uint16_t i = 0; i < p_handle->count; i++)
    {
        variance_sum += (temp_arr[i] - mean) * (temp_arr[i] - mean);
    }
    stats->std_dev = sqrtf(variance_sum / p_handle->count);

    bubble_sort(temp_arr, p_handle->count);
    stats->median = calculate_median(temp_arr, p_handle->count);
}

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
