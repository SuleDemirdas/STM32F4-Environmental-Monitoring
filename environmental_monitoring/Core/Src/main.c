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
#include "cmsis_os.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c_core.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "../../Middlewares/Third_Party/Segger_SystemView/include/SEGGER_SYSVIEW.h"
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
/* --------------------------------------------------------------------------
 * Platform abstraction layer — I2C wrappers
 * -------------------------------------------------------------------------- */

/**
 * @brief Platform I2C read wrapper for sensor drivers.
 *
 * Sensor drivers call this function pointer instead of HAL directly, keeping
 * the drivers portable across different MCU platforms.
 *
 * When @p reg_addr is 0 the function performs a raw master-receive
 * (no register address phase). Otherwise it uses HAL_I2C_Mem_Read.
 *
 * @param[in]  dev_addr 8-bit I2C device address (already left-shifted by 1).
 * @param[in]  reg_addr Register address to read from; 0 for raw receive.
 * @param[out] data     Pointer to the buffer that receives the data.
 * @param[in]  len      Number of bytes to read.
 * @return  0 on success, -1 on I2C error.
 */
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
/**
 * @brief Platform I2C write wrapper for sensor drivers.
 *
 * When @p reg_addr is 0 the function performs a raw master-transmit
 * (no register address phase). Otherwise it uses HAL_I2C_Mem_Write.
 *
 * @param[in] dev_addr 8-bit I2C device address (already left-shifted by 1).
 * @param[in] reg_addr Register address to write to; 0 for raw transmit.
 * @param[in] data     Pointer to the data to transmit.
 * @param[in] len      Number of bytes to write.
 * @return  0 on success, -1 on I2C error.
 */
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
/**
 * @brief Platform delay wrapper for sensor drivers.
 * @param[in] ms Delay duration in milliseconds.
 */
void stm32_delay_wrapper(uint32_t ms) {
    osDelay(ms);
}

/* --------------------------------------------------------------------------
 * Rtos object handles, task function prototypes
 * -------------------------------------------------------------------------- */
TaskHandle_t h_ConsumerTask;
TaskHandle_t h_ProducerTask;
BaseType_t	xConsumerTask;
BaseType_t	xProducerTask;

xSemaphoreHandle xDataAvailable;   // Producer → Consumer
xSemaphoreHandle xSpaceAvailable;  // Consumer → Producer

xSemaphoreHandle xMutexHumSensor;
xSemaphoreHandle xMutexTempSensor;
xSemaphoreHandle xMutexLightSensor;

void vConsumerTask( void * pvParameters );
void vProducerTask( void * pvParameters );

/* --------------------------------------------------------------------------
 * Sensor driver handles
 * -------------------------------------------------------------------------- */
BMP180_HandleTypeDef hbmp180; /**< BMP180 pressure/temperature sensor handle. */
BH1750_HandleTypeDef hbh1750; /**< BH1750 ambient-light sensor handle. */
AHT20_HandleTypeDef  haht20;  /**< AHT20 humidity/temperature sensor handle. */

/* --------------------------------------------------------------------------
 * Filter and buffer handles
 * -------------------------------------------------------------------------- */
Filter_Handle_t hFiltHum;   /**< Sliding-window median filter for humidity. */
Filter_Handle_t hFiltTemp;  /**< Sliding-window median filter for temperature. */
Filter_Handle_t hFiltLight; /**< Sliding-window median filter for illuminance. */

buf_handle_t hBufHum;   /**< Ring buffer storing filtered humidity samples. */
buf_handle_t hBufTemp;  /**< Ring buffer storing filtered temperature samples. */
buf_handle_t hBufLight; /**< Ring buffer storing filtered illuminance samples. */

Sensor_Stats_t hum_stats;   /**< Statistical results for humidity. */
Sensor_Stats_t temp_stats;  /**< Statistical results for temperature. */
Sensor_Stats_t light_stats; /**< Statistical results for illuminance. */

/** @brief Number of samples held in each ring buffer (equals the reporting period in seconds). */
#define RING_BUFFER_SIZE 30

static float storageHum[RING_BUFFER_SIZE];   /**< Backing array for hBufHum. */
static float storageTemp[RING_BUFFER_SIZE];  /**< Backing array for hBufTemp. */
static float storageLight[RING_BUFFER_SIZE]; /**< Backing array for hBufLight. */

float raw_hum;
float raw_light;
float raw_temp;

static uint8_t          seconds_counter  = 0; /**< Counts elapsed seconds; resets after 30. */
volatile uint8_t        read_sensor_flag = 0; /**< Set to 1 by TIM3 ISR; cleared in main loop. */

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

  /* Configure BMP180 */
  hbmp180.i2c_read = stm32_i2c_read_wrapper;
  hbmp180.i2c_write = stm32_i2c_write_wrapper;
  hbmp180.delay_ms = stm32_delay_wrapper;
  hbmp180.oss = 0;
  BMP180_Init(&hbmp180);

  /* Configure BH1750 */
  hbh1750.address = BH1750_ADDRESS_GND;
  hbh1750.delay_ms = stm32_delay_wrapper;
  hbh1750.i2c_read = stm32_i2c_read_wrapper;
  hbh1750.i2c_write = stm32_i2c_write_wrapper;
  hbh1750.mode = BH1750_ONE_TIME_H_RES_MODE;
  BH1750_Init(&hbh1750);

  /* Configure AHT20 */
  haht20.delay_ms = stm32_delay_wrapper;
  haht20.i2c_read = stm32_i2c_read_wrapper;
  haht20.i2c_write = stm32_i2c_write_wrapper;
  AHT20_Init(&haht20);

  /* Initialise ring buffers */
  buffer_init(&hBufHum, storageHum, RING_BUFFER_SIZE);
  buffer_init(&hBufTemp, storageTemp, RING_BUFFER_SIZE);
  buffer_init(&hBufLight, storageLight, RING_BUFFER_SIZE);

  HAL_TIM_Base_Start_IT(&htim3);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  xMutexHumSensor =  xSemaphoreCreateMutex( );
  xMutexTempSensor =  xSemaphoreCreateMutex( );
  xMutexLightSensor =  xSemaphoreCreateMutex( );
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  vSemaphoreCreateBinary(xSpaceAvailable);
  vSemaphoreCreateBinary(xDataAvailable);
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */


  /* USER CODE BEGIN RTOS_THREADS */
  xConsumerTask = xTaskCreate(
                    vConsumerTask,       	/* Function that implements the task. */
                    "Consumer",         	/* Text name for the task. */
                    128,     	 	/* Stack size in words, not bytes. */
                    NULL,    		/* Parameter passed into the task. */
                    5,		/* Priority at which the task is created. */
                    &h_ConsumerTask );    /* Used to pass out the created task's handle. */

  if( xConsumerTask != pdPASS )
  {
	  vTaskDelete( h_ConsumerTask );
  }

  xProducerTask = xTaskCreate(
  		  	  	  vProducerTask,       /* Function that implements the task. */
                    "Producer",          /* Text name for the task. */
					128,      			/* Stack size in words, not bytes. */
                    NULL,    /* Parameter passed into the task. */
                    5,				/* Priority at which the task is created. */
                    &h_ProducerTask );      /* Used to pass out the created task's handle. */
  if( xProducerTask != pdPASS )
  {
	  vTaskDelete( h_ConsumerTask );
  }
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  xSemaphoreGive(xSpaceAvailable);
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
/* --------------------------------------------------------------------------
 * Callbacks and user functions
 * -------------------------------------------------------------------------- */

/*
 *
 */
void vProducerTask( void * pvParameters )
{
    for( ;; )
    {

		if(xSemaphoreTake( xSpaceAvailable, ( TickType_t ) 10 ) == pdTRUE)
        {
        	xSemaphoreTake( xMutexHumSensor, ( TickType_t ) 10 );
            raw_hum = i2c_sensor_read(AHT20_ADDRESS, HUMIDITY_SENSOR, &haht20);
        	xSemaphoreGive( xMutexHumSensor );

        	xSemaphoreTake( xMutexTempSensor, ( TickType_t ) 10 );
            raw_temp = i2c_sensor_read(AHT20_ADDRESS, TEMPERATURE_SENSOR, &haht20);
        	xSemaphoreGive( xMutexTempSensor );

        	xSemaphoreTake( xMutexLightSensor, ( TickType_t ) 10 );
            raw_light = i2c_sensor_read(BH1750_ADDRESS, LIGHT_SENSOR, &hbh1750);
        	xSemaphoreGive( xMutexLightSensor );

        	xSemaphoreGive(xDataAvailable);

        }
		vTaskDelay(1000 * portTICK_PERIOD_MS);
    }
}
void vConsumerTask( void * pvParameters )
{
    for( ;; )
    {
        if(xSemaphoreTake( xDataAvailable, ( TickType_t ) 10 ) == pdTRUE)
        {
        	float hum_raw_data;
        	float temp_raw_data;
        	float light_raw_data;

        	xSemaphoreTake( xMutexHumSensor, ( TickType_t ) 10 );
        	hum_raw_data = raw_hum;
        	xSemaphoreGive( xMutexHumSensor );

			vTaskDelay(5000 * portTICK_PERIOD_MS);
        	xSemaphoreTake( xMutexTempSensor, ( TickType_t ) 10 );
        	temp_raw_data = raw_temp;
        	xSemaphoreGive( xMutexTempSensor );

        	xSemaphoreTake( xMutexLightSensor, ( TickType_t ) 10 );
        	light_raw_data = raw_light;
        	xSemaphoreGive( xMutexLightSensor );

        	xSemaphoreGive(xSpaceAvailable);

            float filt_hum = filter_sensor_value(&hFiltHum, hum_raw_data, 5);
            buffer_write_value(&hBufHum, filt_hum);

            float filt_temp = filter_sensor_value(&hFiltTemp, temp_raw_data, 5);
            buffer_write_value(&hBufTemp, filt_temp);

            float filt_light = filter_sensor_value(&hFiltLight, light_raw_data, 5);
            buffer_write_value(&hBufLight, filt_light);
        }
    }
}


/**
 * @brief Computes descriptive statistics from a ring buffer.
 *
 * Iterates over all valid samples in @p p_handle to find the minimum,
 * maximum and arithmetic mean. A second pass computes the population
 * standard deviation. The buffer is then copied to a temporary array,
 * sorted with bubble_sort(), and the median is extracted.
 *
 * @note The function returns immediately if the buffer is empty.
 *
 * @param[in]  p_handle Pointer to the ring buffer handle containing the samples.
 * @param[out] stats    Pointer to the Sensor_Stats_t structure to populate.
 */
void calculate_statistics(buf_handle_t *p_handle, Sensor_Stats_t *stats)
{
    if (p_handle->count == 0)
	{
    	return;
	}

    float temp_arr[RING_BUFFER_SIZE];
    float sum = 0.0f;

    for (uint16_t i = 0; i < p_handle->count; i++)
    {
        float val = p_handle->buffer[i];
        temp_arr[i] = val;
        sum += val;
    }

    bubble_sort(temp_arr, p_handle->count);
    stats->median = calculate_median(temp_arr, p_handle->count);

    stats->min = temp_arr[0];
    stats->max = temp_arr[p_handle->count - 1];

    float mean = sum / p_handle->count;
    float variance_sum = 0.0f;

    for (uint16_t i = 0; i < p_handle->count; i++)
    {
        variance_sum += (temp_arr[i] - mean) * (temp_arr[i] - mean);
    }
    stats->std_dev = sqrtf(variance_sum / p_handle->count);
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  if (htim->Instance == TIM3)
  {
	HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin );
  }
  /* USER CODE END Callback 1 */
}

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
