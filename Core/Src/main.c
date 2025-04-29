/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "adc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dht11/dht11.h" // 包含DHT11温湿度传感器驱动头文件
#include "mq4/mq4.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**
 * @函数名      : Enable_DWT
 * @描述        : 启用DWT（数据观察点和跟踪）单元
 * @注意事项    : 必须在使用delay_us函数前调用此函数
 * @实现细节    : 启用CoreDebug DEMCR寄存器的TRCENA位，并启用DWT的CYCCNT计数器
 */
void Enable_DWT(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // 启用跟踪和调试功能
  DWT->CYCCNT = 0;                                // 复位循环计数器
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // 启用循环计数器
}
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
  Enable_DWT(); // 启用DWT
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  /* 初始化传感器 */
  // 初始化DHT11温湿度传感器
  DHT11_Init(DHT11_DATA2_GPIO_Port, DHT11_DATA2_Pin);
  char uart_buf[50];      // 定义UART发送缓冲区
  DHT11_Data sensor_data; // 定义DHT11数据结构体

  // 初始化MQ4甲烷气体传感器
  MQ4_Init(&hadc1);              // 传递ADC句柄
  MQ4_CalibState mq4_calib_stat; // 校准状态跟踪变量
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* MQ4传感器校准处理 */
    // mq4_calib_stat = MQ4_GetCalibStatus();
    // if (mq4_calib_stat != MQ4_CALIB_DONE)
    // {
    //   // 执行校准过程（非阻塞）
    //   MQ4_Calibrate();

    //   // 每秒发送一次校准状态信息
    //   static uint32_t last_msg = 0;
    //   if (HAL_GetTick() - last_msg > 1000)
    //   {
    //     char buf[60];
    //     int len = snprintf(buf, sizeof(buf),
    //                        "[MQ4] Calibrating... %d/%d samples, Remain: %ds\r\n",
    //                        MQ4_GetSampleCount(),      // 已采样次数
    //                        MQ4_GetCalibrationTotal(), // 总采样次数
    //                        MQ4_GetRemainingTime());   // 剩余校准时间
    //     // 确保使用实际长度而非strlen
    //     HAL_UART_Transmit(&huart1, (uint8_t *)buf, len, 100);
    //     last_msg = HAL_GetTick();
    //   }
    //   HAL_Delay(200);
    //   continue; // 跳过传感器数据采集，继续校准
    // }

    /* 传感器数据采集与发送 */
    // 1. 读取DHT11温湿度数据
    if (DHT11_Read(&sensor_data) == HAL_OK)
    {
      // 读取MQ4甲烷气体浓度数据
      float ppm = MQ4_ReadPPM();

      // 发送所有数据
      char report[128];
      int report_len = snprintf(report, sizeof(report),
                                "Humidity: %d.%d%%, Temperature: %d.%d C, Methane: %.1f PPM\r\n",
                                sensor_data.humidity, sensor_data.humidity_dec,
                                sensor_data.temperature, sensor_data.temperature_dec,
                                ppm);

      // 单次UART传输所有数据
      HAL_UART_Transmit(&huart1, (uint8_t *)report, report_len, 300); // 增加超时时间到200ms
    }
    else
    {
      // 发送读取错误消息
      const char *err_msg = "DHT11 Read Error!\r\n";
      HAL_UART_Transmit(&huart1, (uint8_t *)err_msg, 20, 300);
    }

    // 延时2秒再次读取，避免频繁读取传感器
    HAL_Delay(2000);
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
