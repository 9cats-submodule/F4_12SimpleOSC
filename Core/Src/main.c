/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
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
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "base.h"
#include "lcd.h"
#include "text.h"
#include "touch.h"
#include "w25qxx.h"
#include "hmi_user_uart.h"
#include "hmi_driver.h"

#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TP_CHECK(x0,y0,x1,y1) tp_dev.x[0] > x0 && tp_dev.y[0] > y0 && tp_dev.x[0] < x1 && tp_dev.y[0] < y1
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
extern u8 sendStatus,Txing_pos,Tx_pos;
extern u8 receiveStatus,IPD;
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
  MX_FSMC_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  delay_init(168);
  HAL_DAC_Start(&hdac,DAC1_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim7);
  W25QXX_Init();
  LCD_Init();
  font_init();
  tp_dev.init();
  TFT_Init();

  //��ʼʱ��ȡֵ
  //W25QXX_Read((u8 *)(&Amplitude),0x0000f000,2);
  //W25QXX_Read((u8 *)(&ARR),0x0000f010,4);
  //Amplitude = 1000;
  //ARR = 680;

  HAL_UART_Transmit_DMA(&huart1, (u8*)"AT+RST\r\n", sizeof("AT+RST\r\n")-1);
  delay_ms(700);
  HAL_UART_Transmit_DMA(&huart1, (u8*)"ATE0\r\n", sizeof("ATE0\r\n")-1);
  delay_ms(100);
  HAL_UART_Transmit_DMA(&huart1, (u8*)"AT+CIPMUX=1\r\n", sizeof("AT+CIPMUX=1\r\n")-1);
  delay_ms(200);
  HAL_UART_Transmit_DMA(&huart1, (u8*)"AT+CIPSERVER=1,7210\r\n", sizeof("AT+CIPSERVER=1,7210\r\n")-1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    static u8 str[20]="AT+CIPSEND=0,0000\r"; //11   13|14|15|16
    u8 j = 13;
    STR[0] = 0x31;
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	u16 Tx_size = Tx_stack_find_cmd(&TxBuffer);
    if(Tx_size && !sendStatus && !receiveStatus)
	{
    	u16 size = Tx_size;
    	sendStatus = 1;
		Txing_pos = Tx_pos;
        str[11] = IPD + 0x30;
        str[14] = str[15] = str[16] = str[17] = 0x0d;
        sprintf((char*)&str[13],"%-4d",Tx_size);
        //���Ϊ9999
        do{j++;}while(size/=10);
        str[j++] = '\r';
        str[j++] = '\n';
        HAL_UART_Transmit_DMA(&huart1, str, j);
    	delay_ms(50);
    	HAL_UART_Transmit_DMA(&huart1, TxBuffer, Tx_size);
    }


	  tp_dev.scan(0);
	  if(tp_dev.sta & TP_PRES_DOWN)
	  {
	    if(TP_PRES_TIME == 0 || (TP_PRES_TIME > 200 && TP_PRES_TIME%20 == 0))
	    {
	  		if(!TP_PRES_EVET) //�����tickû�д����¼�
	  		{
          TP_PRES_EVET = 1;
          TP_PRES_FACK = 1;
	  		  if(TP_CHECK(28,140,48,156))
	  		  {
	  			SetTextValue(0,21,(u8*)"2333");
						
	  		  	ARR = ARR<=50?ARR:ARR-1;
	  		  	LCD_ShowNum(150, 140, ARR, 4, 16);
	  		  }
	  		  if(TP_CHECK(192,140,212,156))
	  		  {
	  			SetTextValue(0,21,(u8*)"2333");
	  		  	ARR = ARR>=1000?ARR:ARR+1;
	  		  	LCD_ShowNum(150, 140, ARR, 4, 16);
	  		  }
	  		  if(TP_CHECK(28,180,48,196))
	  		  {
	  		  	Amplitude = Amplitude==0?0:Amplitude-1;
	  		  	LCD_ShowNum(150, 180, Amplitude, 4, 16);
	  		  }
	  		  if(TP_CHECK(192,180,212,196))
	  		  {
	  		  	Amplitude = Amplitude==4095?4095:Amplitude+1;
	  		  	LCD_ShowNum(150, 180, Amplitude, 4, 16);
	  		  }
	  		}
	    }
	  }
	  else
	  {
	    //if(tp_dev.sta & TP_PRES_FACK){}
	    TP_PRES_TIME = 0;
      TP_PRES_FACK = 0;
	  	TP_PRES_EVET = 0;

	    if(P_Amplitude != Amplitude)
	    {
	    	P_Amplitude = Amplitude;
				LCD_ShowNum(108, 100, Amplitude*3300/4096, 4, 16);
				W25QXX_Write((u8 *)(&Amplitude),0x0000f000,2);
	    }
	    if(P_ARR != ARR)
	    {
	  	  P_ARR = ARR;
				LCD_ShowNum(88,  80, 84000000/ARR/1000, 3, 16);
				LCD_ShowNum(120, 80, 84000000/ARR%1000, 3, 16);
				W25QXX_Write((u8 *)(&ARR),0x0000f010,4);
	      HAL_TIM_Base_DeInit(&htim8);
	      htim8.Init.Period = ARR;
	      HAL_TIM_Base_Init(&htim8);
				HAL_TIM_Base_Start_IT(&htim8);
	    }
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
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
