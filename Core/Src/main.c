/**
 * Jaqueline Maurer Machado		Turma 4422		Nº 11
 * Exercício 2 da Lista de DMA
 */

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
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DAC_FREQ 2000 ///frequência do DAC → 2KHz
#define ADC_FREQ 10000 ///frequência do ADC → 10KHz
///número de pontos para cada ciclo da onda → 360º/100=3,6
///o número de pontos define também a frequência da onda → 100 pontos a 2KHz vai dar uma frequência da onda de 20Hz
#define POINTS_PERIOD 1000
#define PI 3.141592 ///define do valor de PI usado para fazer os cálculos da onda senoidal
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
void triangle_gen(int val_n); ///função que gera uma onda triangular
void sine_gen (int val_n); ///função que gera uma onda senoidal
void freq_change (uint16_t PSC, uint32_t FREQ, TIM_HandleTypeDef* htim);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t sin_val[POINTS_PERIOD]; ///vetor que recebe os valores da onda senoidal
uint16_t triangle_val[POINTS_PERIOD]; ///vetor que recebe os valores da onda triangular
volatile uint16_t adc_val[2] = {0,0}; ///vetor que armazena os valores de cada canal do ADC
uint16_t ch0=0, ch1=0; ///canais do ADC
enum{IN0=0, IN1}; ///enumeração
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
  MX_USART2_UART_Init();
  MX_ADC2_Init();
  MX_DAC_Init();
  MX_TIM4_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  ///Muda a frequência do trigger do ADC → TIMER2
  freq_change (84,ADC_FREQ,&htim2);
  ///Muda a frequência do trigger do DAC → TIMER4
  freq_change(84,DAC_FREQ,&htim4);
  ///Inicialização do OC do canal 1 do TIMER2 → PA15
  ///Usado para testar a frequência
  HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
  ///Inicialização do OC do canal 1 do TIMER4 → PB6
  ///Usado para testar a frequência
  HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1);
  ///Gera o sinal senoidal
  sine_gen(POINTS_PERIOD);
  ///Gera o sinal triangular
  triangle_gen(POINTS_PERIOD);
  ///Inicialização do DAC por meio de DMA → Canal 1: Onda Triangular → PA4
  ///Usando modo circular, então só precisa mandar iniciar uma vez
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)&triangle_val[0], POINTS_PERIOD, DAC_ALIGN_12B_R);
  ///Inicialização do DAC por meio de DMA → Canal 2: Onda Senoidal → PA5
  ///Usando modo circular, então só precisa mandar iniciar uma vez
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2,(uint32_t*)&sin_val[0], POINTS_PERIOD, DAC_ALIGN_12B_R);
  ///Inicialização da amostragem do ADC2 → PA0 e PA1
  ///Usando modo normal, então é necessário "pedir" para ele fazer de novo
  HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&adc_val[0],2);
  /* USER CODE END 2 */

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
}

/* USER CODE BEGIN 4 */
///Função que faz o cálculo da onda senoidal
	void sine_gen(int val_n){
		int i;
		for (i=0; i<val_n; i++){
			sin_val[i] = (1800*(1 + sin(i*2*PI/val_n)));
		}
	}
///Função que faz o cálculo da onda triangular
	void triangle_gen(int val_n){
		int i;
		for(i=0; i<val_n; i++){
			triangle_val[i] = i*4000/val_n;
		}
	}
///Função que calcula o novo valor da frequência quando mudar o o define de DAC_FREQ ou ADC_FREQ, já que ela é ajustável como pede no exercício
	void freq_change(uint16_t PSC, uint32_t FREQ, TIM_HandleTypeDef* htim){
		int ARR = (84000000/(PSC*FREQ))-1;
		if(ARR>65000){
			PSC = 1291;
			ARR = (84000000/(PSC*FREQ))-1;
		}
		__HAL_TIM_SET_PRESCALER(htim, PSC); ///seta o novo valor de PSC
		__HAL_TIM_SET_AUTORELOAD(htim, ARR); ///seta o novo valor de ARR
	}

	/**
	 * Essa função significa que a conversão está completa e gera uma interrupção
	 * Quando o ADC mandar os valores por DMA para a memória, vai gerar uma interrupção
	 * Com essa interrupção, por estar no modo normal, é preciso "pedir" para que o DMA faça esse processo novamente
	 */
	void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
		ch0 = adc_val[IN0]; ///salva os valores do canal 0 do ADC na variável ch0
		ch1 = adc_val[IN1]; ///salva os valores do canal 1 do ADC na variável ch1

		HAL_ADC_Start_DMA(&hadc2, &adc_val[0], 2); ///inicializa novamente fazer uma nova conversão e enviar para a memória
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
