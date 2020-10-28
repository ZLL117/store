/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
CAN_HandleTypeDef hcan;
IWDG_HandleTypeDef hiwdg;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
static void MX_IWDG_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t Uart1Temp;
uint8_t Uart2Temp;
uint8_t Uart1Cnt;
uint8_t Uart2Cnt;
uint16_t LEDCnt;
uint16_t rx1Cnt;
uint16_t rx2Cnt;
uint8_t rx1Done;
uint8_t rx2Done;
uint8_t rx1Buff[1024];
uint8_t rx2Buff[1024];
uint8_t tempBuff[1024];
uint8_t heartBeatBuff[256];

uint16_t heartBeatCnt;//心跳时间间隔
uint16_t healthCnt;//副芯片死机计时
uint16_t startCnt;//开机计时
uint8_t sendTimes;//心跳序号
extern uint8_t busyStatus;//当前正在开锁标志
extern button_sta STATUS;//按钮当前状态

//extern GPIO_TypeDef *bsp_gpio[64];
//extern uint32_t bsp_pin[64];

uint8_t displaylock;//显示当前正在开的锁
extern uint16_t lockCnt;//开锁时计时

uint8_t HEAD[2];
uint8_t TAIL[2];
uint16_t crcnum;
uint16_t crcadd;
uint8_t aaa[4]={"AABB"};

uint8_t test;
extern uint8_t testdisplay;
extern GPIO_TypeDef *bsp_gpio[64];
extern uint32_t bsp_pin[64];
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
  MX_CAN_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
	//MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
	allLine_Up();
	allCheck();

  /* USER CODE END 2 */
	printf("restart");
//	GPIOD->CRH&=0xFFFFFFF0;GPIOD->CRH|=0x00000003;
//	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);
//	HAL_Delay(1000);
//	test = HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_0);
//	printf("%d",test);
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//HAL_IWDG_Refresh(&hiwdg);
		
		if(startCnt<5000)
			BSP_Display(CABINETNO,displaylock);
		else
			Clear_Display();
		if(testdisplay)
			BSP_Display(CABINETNO,displaylock);
		if((STATUS != RELEASE)&&(STATUS != PAUSE))
			allOpen();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		if(rx2Done)
		{
			busyStatus = 1;
			healthCnt = 0;
			rx2Done = 0;
			rx2Cnt = 0;
			memcpy(tempBuff,rx2Buff,sizeof(rx2Buff));
			memset(rx2Buff,0,sizeof(rx2Buff));
			HEAD[0]=0xEE;
			TAIL[0]=0xFF;
			if(strstr((char *)tempBuff,(char *)HEAD)&&strstr((char *)tempBuff,(char *)TAIL))
			{
				uint8_t *TEMPPOINT = tempBuff;
				uint8_t *HEADPOINT = (uint8_t *)strstr((char *)tempBuff,(char *)HEAD);//EE地址
				uint8_t *TAILPOINT = (uint8_t *)strstr((char *)tempBuff,(char *)TAIL);//FF地址
				uint8_t *ATAILPOINT = (uint8_t *)strstr((char *)TAILPOINT+1,(char *)TAIL);//FF地址
				if(ATAILPOINT)
					TAILPOINT = ATAILPOINT;
				if(HEADPOINT<TAILPOINT)
				{
					crcadd = TAILPOINT-TEMPPOINT-2;
					crcnum = tempBuff[crcadd]<<8|tempBuff[crcadd+1];
					if(crcnum == CalCrc16(HEADPOINT+1, TAILPOINT-HEADPOINT-3,0xFFFF))
					{
						JSON_Handle(tempBuff);
//						printf("%s",tempBuff);
					}
//					else
//					{
//						printf("%s",aaa);
//						printf("%s",tempBuff);						
//					}
				}
			}
//			else
//			{
//				printf("%s",aaa);
//				printf("%s",tempBuff);
//				
//			}
			if(strstr(tempBuff,"openRelayOnly"))
			{
				test1fun();
			}
			if(strstr(tempBuff,"openDisplay"))
			{
				test2fun();
			}
			if(strstr(tempBuff,"openOneToSix"))
			{
				test3fun();
			}
			if(strstr(tempBuff,"openOneToSixForce"))
			{
				test4fun();
			}
			if(strstr(tempBuff,"closeRelayOnly"))
			{
				test5fun();
			}
			if(strstr(tempBuff,"closeDisplay"))
			{
				test6fun();
			}
			if(strstr(tempBuff,"allOpen"))
			{
				STATUS = ALLOPEN;
				printf("allOpenR");
			}
			if(strstr(tempBuff,"display"))
			{
				startCnt = 0;
				printf("displayR");
			}
			busyStatus = 0;
			HAL_UART_Receive_IT(&huart2,&Uart2Temp,1);
		}
		if(rx1Done)
		{
			rx1Done = 0;
			rx1Cnt = 0;
			if(strstr((char *)rx1Buff,"receive"))
			{
				healthCnt = 0;
			}
			if(strstr((char *)rx1Buff,"reset"))
			{
				printf("mcu reset\r\n");
			}
			memset(rx1Buff,0,sizeof(rx1Buff));
			HAL_UART_Receive_IT(&huart1,&Uart1Temp,1);
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

  /** Initializes the CPU, AHB and APB busses clocks 
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
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}
/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}
/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 8;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_5TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

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
  htim3.Init.Prescaler = 71;
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
	HAL_TIM_Base_Start_IT(&htim3);
  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */
	HAL_UART_Receive_IT(&huart1,&Uart1Temp,1);
  /* USER CODE END USART1_Init 2 */

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
	HAL_UART_Receive_IT(&huart2,&Uart2Temp,1);
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

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15 
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12 
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15 
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PE2 PE3 PE4 PE5 
                           PE7 PE8 PE9 PE10 
                           PE11 PE12 PE13 PE14 
                           PE15 PE0 PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15|GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PF0 PF1 PF2 PF3 
                           PF4 PF5 PF6 PF7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PF9 PF10 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3 
                           PC4 PC5 PC8 PC9 
                           PC10 PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA4 PA5 PA6 
                           PA7 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10 
                           PB11 PB12 PB13 PB15 
                           PB3 PB4 PB5 PB6 
                           PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10 
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_15 
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6 
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PF11 PF12 PF13 PF14 
                           PF15 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14 
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PG0 PG1 PG2 PG3 
                           PG9 PG10 PG11 PG12 
                           PG13 PG14 PG15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12 
                          |GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PD8 PD9 PD10 PD11 
                           PD12 PD13 PD14 PD15 
                           PD0 PD1 PD2 PD3 
                           PD4 PD5 PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15 
                          |GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PG4 PG5 PG6 PG7 
                           PG8 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 3, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == htim3.Instance)
	{
		if(Uart1Cnt)
			Uart1Cnt++;
		if(Uart1Cnt>100)//一帧定为100ms,最多可接收115200/8/10=1440个字节
		{
			Uart1Cnt = 0;
			rx1Done = 1;
		}
		if(Uart2Cnt)
			Uart2Cnt++;
		if(Uart2Cnt>100)
		{
			Uart2Cnt = 0;
			rx2Done = 1;
		}
		LEDCnt++;
		if(LEDCnt<1000)
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);
		else
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);
		if(LEDCnt>2000)
			LEDCnt = 0;
		heartBeatCnt++;
		if(heartBeatCnt>3000)
		{
			heartBeatCnt = 0;
			if(!busyStatus)
			{
				sprintf((char *)heartBeatBuff,"send%d",++sendTimes);
				healthCnt = 1;
				HAL_UART_Transmit(&huart1,heartBeatBuff,strlen((char *)heartBeatBuff), 0xFFFF);
				if(sendTimes>9)
					sendTimes = 0;
			}			
		}
		if(healthCnt)
			healthCnt++;
		if(healthCnt == 1000)
		{
			//复位副芯片
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_SET);
			printf("mcu2 reset\r\n");
		}
		if(healthCnt == 1100)
		{
			//复位副芯片
			HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET);
			printf("mcu2 reset complete\r\n");
			healthCnt = 0;
		}
		if(lockCnt)
			lockCnt++;
		if(lockCnt>500)
			lockCnt = 0;
		if(startCnt<5000)
			startCnt++;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//	HAL_UART_Transmit(&huart1,"uart",4,100);
	if(huart->Instance == huart1.Instance)
	{
		if(!Uart1Cnt)
		Uart1Cnt ++;
		if(rx1Cnt<1024)
			rx1Buff[rx1Cnt++] = Uart1Temp;
		HAL_UART_Receive_IT(&huart1,&Uart1Temp,1);	
	}
	if(huart->Instance == huart2.Instance)
	{
		if(!Uart2Cnt)
		Uart2Cnt ++;
		if(rx2Cnt<1024)
		{
			rx2Buff[rx2Cnt++] = Uart2Temp;
				
		}	
		else
			printf("out range");	
		HAL_UART_Receive_IT(&huart2,&Uart2Temp,1);	
	}
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin==GPIO_PIN_9)
	{
		test = HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_9);
		HAL_Delay(1000);
		if(HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_9)==GPIO_PIN_RESET)
		{
			STATUS = ALLOPEN;
		}
//		HAL_Delay(900);
//		if(HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_9)==GPIO_PIN_RESET)//长按停止
//		{
//			STATUS = RELEASE;
//			//
//		}
	}
	 if(GPIO_Pin==GPIO_PIN_10)
	{
		test = HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_10);
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_10)==GPIO_PIN_RESET)
		{
			if(STATUS == ALLOPEN|STATUS == RECOVER)//暂停或开始
			{
				STATUS = PAUSE;
			}
			else if(STATUS == PAUSE)
			{
				STATUS = RECOVER;
			}
		}
		HAL_Delay(900);
		if(HAL_GPIO_ReadPin(GPIOF,GPIO_PIN_10)==GPIO_PIN_RESET)//长按停止
		{
			STATUS = RELEASE;
		}
	}
}

#ifdef __GNUC_
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
 
	return ch;
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
