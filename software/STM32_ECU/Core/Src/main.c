/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "button.h"
#include "can.h"
#include "motor.h"
#include "buzzer.h"
#include <stdbool.h>
#include <string.h>
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
DMA_HandleTypeDef hdma_adc1;

CAN_HandleTypeDef hcan;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

/* Definitions for CANRxHandlerTask */
osThreadId_t CANRxHandlerTaskHandle;
const osThreadAttr_t CANRxHandlerTask_attributes = {
  .name = "CANRxHandlerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CANTxHandlerTask */
osThreadId_t CANTxHandlerTaskHandle;
const osThreadAttr_t CANTxHandlerTask_attributes = {
  .name = "CANTxHandlerTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
uint16_t potentiometer[TEN] = {0};

CAN_Frame canTxQueue[CAN_TX_QUEUE_SIZE];

Motor_Handle dc_motor;
Motor_Handle encoder_motor;
Buzzer_Handle buzzer;

volatile DigitalOutput_Resp_Frame digital_output_data[NUMBER_OF_DIG_OUT_RES_FRAME] = {0};
volatile DigitalInput_Resp_Frame digital_input_data[NUMBER_OF_DIG_IN_RES_FRAME] = {0};
volatile DigitalOutput_Cmd_Frame digital_output_cmd_data[NUMBER_OF_DIG_OUT_CMD_FRAME] = {0};
volatile AnalogInput_Resp_Frame analog_input_data[NUMBER_OF_ANALOG_IN_RES_FRAME] = {0};

CAN_RxFrame can_rx_frame;
/*Rotary Encoder Initial*/
static int32_t encoder_position = 0;
static uint16_t encoder_last_count = 32768;
volatile int32_t shared_encoder_speed = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_CAN_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
void CANRxHandler(void *argument);
void CANTxHandler(void *argument);

/* USER CODE BEGIN PFP */
/**
  * @brief  Process Digital Output Commands
  * @param  cmd_data: Pointer to the Digital Output Command Frame
  *                                   containing the commands to process
  * @retval None
  */
void digital_output_process(volatile DigitalOutput_Cmd_Frame *cmd_data);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &can_rx_frame.header, can_rx_frame.data);
  // Process received CAN message
  CAN_process_command(&can_rx_frame, digital_output_cmd_data);
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

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_CAN_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
  HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
  __HAL_TIM_SET_COUNTER(&htim2, 32768);  // Set center position

  Motor_Init(&dc_motor, &htim1, TIM_CHANNEL_1, GPIOA, GPIO_PIN_11, GPIOA, GPIO_PIN_12);
  Motor_SetDirection(&dc_motor, MOTOR_DIR_FORWARD);  // quay thuận
  Motor_Enable(&dc_motor, true);
  Buzzer_Init(&buzzer, &htim3);

  Motor_Init(&encoder_motor, &htim1, TIM_CHANNEL_2,
             GPIOA, MOTOR2_IN3_Pin,   // PA2
             GPIOA, MOTOR2_IN4_Pin);  // PA3
  Motor_SetDirection(&encoder_motor, MOTOR_DIR_FORWARD);
  Motor_Enable(&encoder_motor, true);



  HAL_CAN_Start(&hcan);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)potentiometer, TEN);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of CANRxHandlerTask */
  CANRxHandlerTaskHandle = osThreadNew(CANRxHandler, NULL, &CANRxHandlerTask_attributes);

  /* creation of CANTxHandlerTask */
  CANTxHandlerTaskHandle = osThreadNew(CANTxHandler, NULL, &CANTxHandlerTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
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

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hcan.Init.Prescaler = 80;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_5TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = ENABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */
  CAN_FilterTypeDef canfilterconfig;

  canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
  canfilterconfig.FilterBank = 10;  // anything between 0 to SlaveStartFilterBank
  canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  canfilterconfig.FilterIdHigh = 0;
  canfilterconfig.FilterIdLow = 0x0000;
  canfilterconfig.FilterMaskIdHigh = 0;
  canfilterconfig.FilterMaskIdLow = 0x0000;
  canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
  canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
  canfilterconfig.SlaveStartFilterBank = 13;  // 13 to 27 are assigned to slave CAN (CAN 2) OR 0 to 12 are assgned to CAN1

  HAL_CAN_ConfigFilter(&hcan, &canfilterconfig);
  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 799;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 799;
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
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, MOTOR2_IN3_Pin|MOTOR2_IN4_Pin|MOTOR_IN1_Pin|MOTOR_IN2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DHT11_DATA_GPIO_Port, DHT11_DATA_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, BTN_COL0_Pin|BTN_COL1_Pin|BTN_COL2_Pin|BTN_COL3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : MOTOR2_IN3_Pin MOTOR2_IN4_Pin MOTOR_IN1_Pin MOTOR_IN2_Pin */
  GPIO_InitStruct.Pin = MOTOR2_IN3_Pin|MOTOR2_IN4_Pin|MOTOR_IN1_Pin|MOTOR_IN2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : DHT11_DATA_Pin */
  GPIO_InitStruct.Pin = DHT11_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DHT11_DATA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_COL0_Pin BTN_COL1_Pin BTN_COL2_Pin BTN_COL3_Pin */
  GPIO_InitStruct.Pin = BTN_COL0_Pin|BTN_COL1_Pin|BTN_COL2_Pin|BTN_COL3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_ROW0_Pin BTN_ROW1_Pin BTN_ROW2_Pin BTN_ROW3_Pin */
  GPIO_InitStruct.Pin = BTN_ROW0_Pin|BTN_ROW1_Pin|BTN_ROW2_Pin|BTN_ROW3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief  Process Digital Output Commands
  * @param  cmd_data: Pointer to the Digital Output Command Frame
  *                                   containing the commands to process
  * @retval None
  */
void digital_output_process(volatile DigitalOutput_Cmd_Frame *cmd_data) {

  // Digital Output 10
  if (cmd_data[1].signal[1].switchCmd == 1) {
    TIM1->CCR3 = (cmd_data[1].signal[1].dutyCycle * 10U) - 1U;
  } else {
    TIM1->CCR3 = 0;
  }
/*
  // Digital Output 19
  if (cmd_data[2].signal[2].switchCmd == 1) {
    TIM1->CCR2 = (cmd_data[2].signal[2].dutyCycle * 10U) - 1U;
  } else {
    TIM1->CCR2 = 0;
  }
  */

  //  XI NHAN TRÁI (Output 8)
    if (cmd_data[1].signal[0].switchCmd == 1) {
        // Duty > 0 → BẬT, Duty = 0 → TẮT
        if (cmd_data[1].signal[0].dutyCycle > 0) {
            TIM3->CCR1 = 999;  // Full brightness
        } else {
            TIM3->CCR1 = 0;    // OFF
        }
    } else {
        TIM3->CCR1 = 0;
    }

    // XI NHAN PHẢI (Output 12)
    if (cmd_data[1].signal[4].switchCmd == 1) {
        if (cmd_data[1].signal[4].dutyCycle > 0) {
            TIM3->CCR2 = 999;
        } else {
            TIM3->CCR2 = 0;
        }
    } else {
        TIM3->CCR2 = 0;
    }
/*
  if (cmd_data[3].signal[3].switchCmd == 1) {
      Motor_Enable(&dc_motor, true);
      // dutyCycle 0-100: map sang direction
      if (cmd_data[3].signal[3].dutyCycle <= 40) {
        Motor_SetDirection(&dc_motor, MOTOR_DIR_BACKWARD);
      } else if (cmd_data[3].signal[3].dutyCycle <= 60) {
        Motor_SetDirection(&dc_motor, MOTOR_DIR_STOP);
      } else {
        Motor_SetDirection(&dc_motor, MOTOR_DIR_FORWARD);
      }
    } else {
      Motor_Enable(&dc_motor, false);
    }
    */

  // Digital Output 22
    if (cmd_data[2].signal[6].switchCmd == 1) {
        if (cmd_data[2].signal[6].dutyCycle > 0) {
          TIM3->CCR4 = 999;  // LED đỏ sáng full brightness
        } else {
          TIM3->CCR4 = 0;    // LED đỏ tắt
        }
      } else {
        TIM3->CCR4 = 0;
      }

}
/**
  * @brief  Process Rotary Encoder and calculate speed
  * @retval Speed value (0-250)
  */
int32_t process_encoder(void) {
    uint16_t current_count = __HAL_TIM_GET_COUNTER(&htim2);
    int16_t delta = (int16_t)(current_count - encoder_last_count);

    encoder_position += delta;
    encoder_last_count = current_count;

    // Clamp position
    if (encoder_position < 0) encoder_position = 0;
    if (encoder_position > 500) encoder_position = 500;  // ← Giảm max xuống 500

    // Map position to speed (0-250)
    int32_t speed = encoder_position / 2;  // ← ĐỔI TỪ /4 THÀNH /2

    return speed;
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_CANRxHandler */
/**
  * @brief  Function implementing the CANRxHandlerTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_CANRxHandler */
void CANRxHandler(void *argument)
{
  /* USER CODE BEGIN 5 */
  
  btn_matrix_init();
 // TIM3->CCR1 = 999; // Initialize PWM duty cycle to 999
  uint32_t avg_pot = 0;
  static bool car_lights_on = false;
  static uint32_t debug_counter = 0;


  /* Infinite loop */
  for(;;)
  {
    // Process button matrix input
    btn_matrix_process(digital_input_data);
    static uint8_t prev_s11_state = 0;
        uint8_t s11_state = btn_state[20];  // ← button_s11 = index 20

        if (s11_state == 1 && prev_s11_state == 0) {
            // S11 vừa được nhấn
            car_lights_on = !car_lights_on;

            // Điều khiển LED trắng (Digital Output 32 - PB1 - TIM3_CH4)
            if (car_lights_on) {
                TIM3->CCR4 = 999;  // LED đỏ ON (full brightness)
            } else {
                TIM3->CCR4 = 0;    // LED đỏ OFF
            }
            GPIOC->ODR ^= GPIO_PIN_13;
        }
        prev_s11_state = s11_state;


    // Process Digital Output Commands immediately
    digital_output_process(digital_output_cmd_data);
    avg_pot = 0;
        for (uint8_t i = 0; i < TEN; i++) {
          avg_pot += potentiometer[i];
        }
        avg_pot /= TEN;
        Motor_UpdateFromADC(&dc_motor, avg_pot);

        int32_t encoder_speed = process_encoder();
                shared_encoder_speed = encoder_speed;  // ← LƯU VÀO BIẾN SHARED
                debug_counter++;
                       if (debug_counter >= 50) {  // Mỗi 500ms
                           debug_counter = 0;

                           // Nếu encoder > 0 → Toggle LED
                           if (encoder_speed > 0) {
                               GPIOC->ODR ^= GPIO_PIN_13;  // Toggle LED
                           }

                           // Có thể print qua SWO nếu có:
                           // printf("Encoder: pos=%ld, speed=%ld\n", encoder_position, encoder_speed);
                       }

                uint16_t motor_speed = (encoder_speed * 999UL) / 250UL;

                // Force enable + direction
                encoder_motor.enabled = true;
                encoder_motor.direction = MOTOR_DIR_FORWARD;

                // Set PWM trực tiếp
                if (motor_speed > 0) {
                    TIM1->CCR2 = motor_speed;
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);  // IN3 = LOW
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);    // IN4 = HIGH
                } else {
                    TIM1->CCR2 = 0;
                }

        Buzzer_Update(&buzzer);
        osDelay(10);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_CANTxHandler */
/**
* @brief Function implementing the CANTxHandlerTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_CANTxHandler */
void CANTxHandler(void *argument)
{
  /* USER CODE BEGIN CANTxHandler */
  DHT11_Init();
  CAN_Frame tx_frame;
  uint32_t potentiometer_value = 0;
  static uint32_t dht11_read_counter = 0;
  static uint32_t frame_send_counter = 0;
  DHT11_Data dht11_data;

  // Initialize default values for Digital Output Response
  for (uint8_t i = 0; i < NUMBER_OF_DIG_OUT_RES_FRAME; i++) {
    for (uint8_t j = 0; j < DIGITAL_OUT_RESP_SIGNAL_PER_FRAME; j++) {
      digital_output_data[i].signal[j].statusPS = 0;
      digital_output_data[i].signal[j].elDiagnosis = 0;
      digital_output_data[i].signal[j].currentFB = 5;
    }
  }

  // Initialize Digital Input Response Frame
  for (uint8_t i = 0; i < NUMBER_OF_DIG_IN_RES_FRAME; i++) {
    for (uint8_t j = 0; j < DIGITAL_IN_RESP_SIGNAL_PER_FRAME; j++) {
      digital_input_data[i].signal[j].inputStatus = 0;
      digital_input_data[i].signal[j].freshness = 0;
      digital_input_data[i].signal[j].elDiagnosis = 0;
    }
  }

  /* Infinite loop */
  for(;;)
  {
      potentiometer_value = 0;
      frame_send_counter++;

      // ========== ĐỌC DHT11 MỖI 2 GIÂY (40 x 100ms) ==========
      dht11_read_counter++;
      if (dht11_read_counter >= 20) { // 20 x 100ms = 2 giây
          dht11_read_counter = 0;

          DHT11_Status status = DHT11_Read(&dht11_data);
          if (status == DHT11_OK) {
              float temp = DHT11_GetTemperature();
              uint16_t temp_adc = (uint16_t)((temp / 50.0f) * 4095.0f);
              analog_input_data[0].signal[2].analogValue = temp_adc;
              analog_input_data[0].signal[2].elDiagnosis = 0;

              float humidity = DHT11_GetHumidity();
              uint16_t humidity_adc = (uint16_t)((humidity / 100.0f) * 4095.0f);
              analog_input_data[0].signal[3].analogValue = humidity_adc;
              analog_input_data[0].signal[3].elDiagnosis = 0;

              /* BUZZER ALARM CONTROL*/
              if (temp > TEMP_ALARM_THRESHOLD) {
                  // (> 50°C) → BẬT BUZZER
                  if (!Buzzer_IsActive(&buzzer)) {
                      Buzzer_Start(&buzzer);
                      // Toggle LED PC13 để báo hiệu alarm
                      GPIOC->ODR |= GPIO_PIN_13;
                  }
              } else {
                  // Nhiệt độ bình thường → TẮT BUZZER
                  if (Buzzer_IsActive(&buzzer)) {
                      Buzzer_Stop(&buzzer);
                      // Tắt LED PC13
                      GPIOC->ODR &= ~GPIO_PIN_13;
                  }
              }
              /* end BUZZER ALARM CONTROL*/

          } else {
              analog_input_data[0].signal[2].elDiagnosis = 1;
              analog_input_data[0].signal[3].elDiagnosis = 1;
          }
      }

      // ========== CẬP NHẬT POTENTIOMETER ==========
      for (uint8_t i = 0; i < TEN; i++) {
          potentiometer_value += potentiometer[i];
      }
      potentiometer_value /= TEN;
      analog_input_data[0].signal[0].analogValue = potentiometer_value;

      int32_t encoder_speed = shared_encoder_speed;  // ĐỌC TỪ BIẾN SHARED
      uint16_t encoder_adc = (uint16_t)((encoder_speed * 4095.0f) / 250.0f);
      analog_input_data[0].signal[1].analogValue = encoder_adc;
      analog_input_data[0].signal[1].elDiagnosis = 0; // không có lỗi

      // ========== FRESHNESS UPDATE ==========
      for (uint8_t i = 0; i < NUMBER_OF_DIG_IN_RES_FRAME; i++) {
          for (uint8_t j = 0; j < DIGITAL_IN_RESP_SIGNAL_PER_FRAME; j++) {
              digital_input_data[i].signal[j].freshness++;
          }
      }

      // ========== GỬI FRAMES THEO CHU KỲ ROUND-ROBIN ==========
      uint8_t group = frame_send_counter % 4;
      bool send_success = true;

      switch(group) {
          case 0:
              for (uint8_t i = 0; i < 5 && i < NUMBER_OF_DIG_OUT_RES_FRAME; i++) {
                  tx_frame.id = DIGITAL_OUTPUT_RES_ID(i);
                  memcpy(tx_frame.data, (uint8_t*)&digital_output_data[i].sdu,
                         sizeof(digital_output_data[i].sdu));
                  if (!CAN_EnqueueTxFrame(&tx_frame, canTxQueue)) {
                      send_success = false;
                      break;
                  }
              }
              break;

          case 1:
              for (uint8_t i = 5; i < NUMBER_OF_DIG_OUT_RES_FRAME; i++) {
                  tx_frame.id = DIGITAL_OUTPUT_RES_ID(i);
                  memcpy(tx_frame.data, (uint8_t*)&digital_output_data[i].sdu,
                         sizeof(digital_output_data[i].sdu));
                  if (!CAN_EnqueueTxFrame(&tx_frame, canTxQueue)) {
                      send_success = false;
                      break;
                  }
              }
              for (uint8_t i = 0; i < 2 && i < NUMBER_OF_DIG_IN_RES_FRAME && send_success; i++){
                  tx_frame.id = DIGITAL_INPUT_RES_ID(i);
                  memcpy(tx_frame.data, (uint8_t*)&digital_input_data[i].sdu,
                         sizeof(digital_input_data[i].sdu));
                  if (!CAN_EnqueueTxFrame(&tx_frame, canTxQueue)) {
                      send_success = false;
                      break;
                  }
              }
              break;

          case 2:
              for (uint8_t i = 2; i < NUMBER_OF_DIG_IN_RES_FRAME; i++){
                  tx_frame.id = DIGITAL_INPUT_RES_ID(i);
                  memcpy(tx_frame.data, (uint8_t*)&digital_input_data[i].sdu,
                         sizeof(digital_input_data[i].sdu));
                  if (!CAN_EnqueueTxFrame(&tx_frame, canTxQueue)) {
                      send_success = false;
                      break;
                  }
              }
              for (uint8_t i = 0; i < 3 && i < NUMBER_OF_ANALOG_IN_RES_FRAME && send_success; i++) {
                  tx_frame.id = ANALOG_INPUT_RES_ID(i);
                  memcpy(tx_frame.data, (uint8_t*)&analog_input_data[i].sdu,
                         sizeof(analog_input_data[i].sdu));
                  if (!CAN_EnqueueTxFrame(&tx_frame, canTxQueue)) {
                      send_success = false;
                      break;
                  }
              }
              break;

          case 3:
              for (uint8_t i = 3; i < NUMBER_OF_ANALOG_IN_RES_FRAME; i++) {
                  tx_frame.id = ANALOG_INPUT_RES_ID(i);
                  memcpy(tx_frame.data, (uint8_t*)&analog_input_data[i].sdu,
                         sizeof(analog_input_data[i].sdu));
                  if (!CAN_EnqueueTxFrame(&tx_frame, canTxQueue)) {
                      send_success = false;
                      break;
                  }
              }
              break;
      }

      if (send_success) {
          CAN_send(&hcan, canTxQueue);
      }

      osDelay(100);
  }
  /* USER CODE END CANTxHandler */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

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
