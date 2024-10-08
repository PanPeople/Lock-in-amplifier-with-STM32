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
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// buffer size for the ADC values
#define ADC_BUFFER_SIZE 600
#define LookUpTableSize 100

// IIR | Butterworth | LPF | order = 1 | cutoff = 7.5 Hz | fs = 1MHz
#define NZEROS 1
#define NPOLES 1
#define GAIN   4.245131737e+003

static float xv[NZEROS+1], yv[NPOLES+1];

#define NUM_CHUNKS 10

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// const uint16_t sineLookupTable[] = {
// 2048, 2060, 2073, 2085, 2098, 2110, 2123, 2135, 2148, 2161, 2173, 2186, 2198, 2211, 2223, 2236, 2248, 2261, 2273, 2286, 2298, 2311, 2323, 2335, 2348, 2360, 2373, 2385, 2398, 2410, 2422, 2435, 2447, 2459, 2472, 2484, 2496, 2508, 2521, 2533, 2545, 2557, 2569, 2581, 2594, 2606, 2618, 2630, 2642, 2654, 2666, 2678, 2690, 2702, 2714, 2725, 2737, 2749, 2761, 2773, 2784, 2796, 2808, 2819, 2831, 2843, 2854, 2866, 2877, 2889, 2900, 2912, 2923, 2934, 2946, 2957, 2968, 2979, 2990, 3002, 3013, 3024, 3035, 3046, 3057, 3068, 3078, 3089, 3100, 3111, 3122, 3132, 3143, 3154, 3164, 3175, 3185, 3195, 3206, 3216,
// 3226, 3237, 3247, 3257, 3267, 3277, 3287, 3297, 3307, 3317, 3327, 3337, 3346, 3356, 3366, 3375, 3385, 3394, 3404, 3413, 3423, 3432, 3441, 3450, 3459, 3468, 3477, 3486, 3495, 3504, 3513, 3522, 3530, 3539, 3548, 3556, 3565, 3573, 3581, 3590, 3598, 3606, 3614, 3622, 3630, 3638, 3646, 3654, 3662, 3669, 3677, 3685, 3692, 3700, 3707, 3714, 3722, 3729, 3736, 3743, 3750, 3757, 3764, 3771, 3777, 3784, 3791, 3797, 3804, 3810, 3816, 3823, 3829, 3835, 3841, 3847, 3853, 3859, 3865, 3871, 3876, 3882, 3888, 3893, 3898, 3904, 3909, 3914, 3919, 3924, 3929, 3934, 3939, 3944, 3949, 3953, 3958, 3962, 3967, 3971,
// 3975, 3980, 3984, 3988, 3992, 3996, 3999, 4003, 4007, 4010, 4014, 4017, 4021, 4024, 4027, 4031, 4034, 4037, 4040, 4042, 4045, 4048, 4051, 4053, 4056, 4058, 4060, 4063, 4065, 4067, 4069, 4071, 4073, 4075, 4076, 4078, 4080, 4081, 4083, 4084, 4085, 4086, 4087, 4088, 4089, 4090, 4091, 4092, 4093, 4093, 4094, 4094, 4094, 4095, 4095, 4095, 4095, 4095, 4095, 4095, 4094, 4094, 4094, 4093, 4093, 4092, 4091, 4090, 4089, 4088, 4087, 4086, 4085, 4084, 4083, 4081, 4080, 4078, 4076, 4075, 4073, 4071, 4069, 4067, 4065, 4063, 4060, 4058, 4056, 4053, 4051, 4048, 4045, 4042, 4040, 4037, 4034, 4031, 4027, 4024,
// 4021, 4017, 4014, 4010, 4007, 4003, 3999, 3996, 3992, 3988, 3984, 3980, 3975, 3971, 3967, 3962, 3958, 3953, 3949, 3944, 3939, 3934, 3929, 3924, 3919, 3914, 3909, 3904, 3898, 3893, 3888, 3882, 3876, 3871, 3865, 3859, 3853, 3847, 3841, 3835, 3829, 3823, 3816, 3810, 3804, 3797, 3791, 3784, 3777, 3771, 3764, 3757, 3750, 3743, 3736, 3729, 3722, 3714, 3707, 3700, 3692, 3685, 3677, 3669, 3662, 3654, 3646, 3638, 3630, 3622, 3614, 3606, 3598, 3590, 3581, 3573, 3565, 3556, 3548, 3539, 3530, 3522, 3513, 3504, 3495, 3486, 3477, 3468, 3459, 3450, 3441, 3432, 3423, 3413, 3404, 3394, 3385, 3375, 3366, 3356,
// 3346, 3337, 3327, 3317, 3307, 3297, 3287, 3277, 3267, 3257, 3247, 3237, 3226, 3216, 3206, 3195, 3185, 3175, 3164, 3154, 3143, 3132, 3122, 3111, 3100, 3089, 3078, 3068, 3057, 3046, 3035, 3024, 3013, 3002, 2990, 2979, 2968, 2957, 2946, 2934, 2923, 2912, 2900, 2889, 2877, 2866, 2854, 2843, 2831, 2819, 2808, 2796, 2784, 2773, 2761, 2749, 2737, 2725, 2714, 2702, 2690, 2678, 2666, 2654, 2642, 2630, 2618, 2606, 2594, 2581, 2569, 2557, 2545, 2533, 2521, 2508, 2496, 2484, 2472, 2459, 2447, 2435, 2422, 2410, 2398, 2385, 2373, 2360, 2348, 2335, 2323, 2311, 2298, 2286, 2273, 2261, 2248, 2236, 2223, 2211,
// 2198, 2186, 2173, 2161, 2148, 2135, 2123, 2110, 2098, 2085, 2073, 2060, 2048, 2035, 2022, 2010, 1997, 1985, 1972, 1960, 1947, 1934, 1922, 1909, 1897, 1884, 1872, 1859, 1847, 1834, 1822, 1809, 1797, 1784, 1772, 1760, 1747, 1735, 1722, 1710, 1697, 1685, 1673, 1660, 1648, 1636, 1623, 1611, 1599, 1587, 1574, 1562, 1550, 1538, 1526, 1514, 1501, 1489, 1477, 1465, 1453, 1441, 1429, 1417, 1405, 1393, 1381, 1370, 1358, 1346, 1334, 1322, 1311, 1299, 1287, 1276, 1264, 1252, 1241, 1229, 1218, 1206, 1195, 1183, 1172, 1161, 1149, 1138, 1127, 1116, 1105, 1093, 1082, 1071, 1060, 1049, 1038, 1027, 1017, 1006,
// 995, 984, 973, 963, 952, 941, 931, 920, 910, 900, 889, 879, 869, 858, 848, 838, 828, 818, 808, 798, 788, 778, 768, 758, 749, 739, 729, 720, 710, 701, 691, 682, 672, 663, 654, 645, 636, 627, 618, 609, 600, 591, 582, 573, 565, 556, 547, 539, 530, 522, 514, 505, 497, 489, 481, 473, 465, 457, 449, 441, 433, 426, 418, 410, 403, 395, 388, 381, 373, 366, 359, 352, 345, 338, 331, 324, 318, 311, 304, 298, 291, 285, 279, 272, 266, 260, 254, 248, 242, 236, 230, 224, 219, 213, 207, 202, 197, 191, 186, 181,
// 176, 171, 166, 161, 156, 151, 146, 142, 137, 133, 128, 124, 120, 115, 111, 107, 103, 99, 96, 92, 88, 85, 81, 78, 74, 71, 68, 64, 61, 58, 55, 53, 50, 47, 44, 42, 39, 37, 35, 32, 30, 28, 26, 24, 22, 20, 19, 17, 15, 14, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 14, 15, 17, 19, 20, 22, 24, 26, 28, 30, 32, 35, 37,
// 39, 42, 44, 47, 50, 53, 55, 58, 61, 64, 68, 71, 74, 78, 81, 85, 88, 92, 96, 99, 103, 107, 111, 115, 120, 124, 128, 133, 137, 142, 146, 151, 156, 161, 166, 171, 176, 181, 186, 191, 197, 202, 207, 213, 219, 224, 230, 236, 242, 248, 254, 260, 266, 272, 279, 285, 291, 298, 304, 311, 318, 324, 331, 338, 345, 352, 359, 366, 373, 381, 388, 395, 403, 410, 418, 426, 433, 441, 449, 457, 465, 473, 481, 489, 497, 505, 514, 522, 530, 539, 547, 556, 565, 573, 582, 591, 600, 609, 618, 627,
// 636, 645, 654, 663, 672, 682, 691, 701, 710, 720, 729, 739, 749, 758, 768, 778, 788, 798, 808, 818, 828, 838, 848, 858, 869, 879, 889, 900, 910, 920, 931, 941, 952, 963, 973, 984, 995, 1006, 1017, 1027, 1038, 1049, 1060, 1071, 1082, 1093, 1105, 1116, 1127, 1138, 1149, 1161, 1172, 1183, 1195, 1206, 1218, 1229, 1241, 1252, 1264, 1276, 1287, 1299, 1311, 1322, 1334, 1346, 1358, 1370, 1381, 1393, 1405, 1417, 1429, 1441, 1453, 1465, 1477, 1489, 1501, 1514, 1526, 1538, 1550, 1562, 1574, 1587, 1599, 1611, 1623, 1636, 1648, 1660, 1673, 1685, 1697, 1710, 1722, 1735,
// 1747, 1760, 1772, 1784, 1797, 1809, 1822, 1834, 1847, 1859, 1872, 1884, 1897, 1909, 1922, 1934, 1947, 1960, 1972, 1985, 1997, 2010, 2022, 2035
// };


// sine wave lookup table | length = 100
// 12 bit max value is 4095
const uint16_t sineLookupTable[] = {
2048, 2176, 2304, 2431, 2557, 2680, 2801, 2919,
3034, 3145, 3251, 3353, 3449, 3540, 3625, 3704,
3776, 3842, 3900, 3951, 3995, 4031, 4059, 4079,
4091, 4095, 4091, 4079, 4059, 4031, 3995, 3951,
3900, 3842, 3776, 3704, 3625, 3540, 3449, 3353,
3251, 3145, 3034, 2919, 2801, 2680, 2557, 2431,
2304, 2176, 2048, 1919, 1791, 1664, 1538, 1415,
1294, 1176, 1061, 950, 844, 742, 646, 555,
470, 391, 319, 253, 195, 144, 100, 64,
36, 16, 4, 0, 4, 16, 36, 64,
100, 144, 195, 253, 319, 391, 470, 555,
646, 742, 844, 950, 1061, 1176, 1294, 1415,
1538, 1664, 1791, 1919, 2047
}; //101 -> change later to 100


const float sineLookupTableFloat[] = {
 0.0000000000, 0.0627905195, 0.1253332336, 0.1873813146,
 0.2486898872, 0.3090169944, 0.3681245527, 0.4257792916,
 0.4817536741, 0.5358267950, 0.5877852523, 0.6374239897,
 0.6845471059, 0.7289686274, 0.7705132428, 0.8090169944,
 0.8443279255, 0.8763066800, 0.9048270525, 0.9297764859,
 0.9510565163, 0.9685831611, 0.9822872507, 0.9921147013,
 0.9980267284, 1.0000000000, 0.9980267284, 0.9921147013,
 0.9822872507, 0.9685831611, 0.9510565163, 0.9297764859,
 0.9048270525, 0.8763066800, 0.8443279255, 0.8090169944,
 0.7705132428, 0.7289686274, 0.6845471059, 0.6374239897,
 0.5877852523, 0.5358267950, 0.4817536741, 0.4257792916,
 0.3681245527, 0.3090169944, 0.2486898872, 0.1873813146,
 0.1253332336, 0.0627905195, 0.0000000000,-0.0627905195,
-0.1253332336,-0.1873813146,-0.2486898872,-0.3090169944,
-0.3681245527,-0.4257792916,-0.4817536741,-0.5358267950,
-0.5877852523,-0.6374239897,-0.6845471059,-0.7289686274,
-0.7705132428,-0.8090169944,-0.8443279255,-0.8763066800,
-0.9048270525,-0.9297764859,-0.9510565163,-0.9685831611,
-0.9822872507,-0.9921147013,-0.9980267284,-1.0000000000,
-0.9980267284,-0.9921147013,-0.9822872507,-0.9685831611,
-0.9510565163,-0.9297764859,-0.9048270525,-0.8763066800,
-0.8443279255,-0.8090169944,-0.7705132428,-0.7289686274,
-0.6845471059,-0.6374239897,-0.5877852523,-0.5358267950,
-0.4817536741,-0.4257792916,-0.3681245527,-0.3090169944,
-0.2486898872,-0.1873813146,-0.1253332336,-0.0627905195 };

const float cosineLookupTableFloat[] = {
1.0000000000, 0.9980267284, 0.9921147013,
 0.9822872507, 0.9685831611, 0.9510565163, 0.9297764859,
 0.9048270525, 0.8763066800, 0.8443279255, 0.8090169944,
 0.7705132428, 0.7289686274, 0.6845471059, 0.6374239897,
 0.5877852523, 0.5358267950, 0.4817536741, 0.4257792916,
 0.3681245527, 0.3090169944, 0.2486898872, 0.1873813146,
 0.1253332336, 0.0627905195, 0.0000000000,-0.0627905195,
-0.1253332336,-0.1873813146,-0.2486898872,-0.3090169944,
-0.3681245527,-0.4257792916,-0.4817536741,-0.5358267950,
-0.5877852523,-0.6374239897,-0.6845471059,-0.7289686274,
-0.7705132428,-0.8090169944,-0.8443279255,-0.8763066800,
-0.9048270525,-0.9297764859,-0.9510565163,-0.9685831611,
-0.9822872507,-0.9921147013,-0.9980267284,-1.0000000000,
-0.9980267284,-0.9921147013,-0.9822872507,-0.9685831611,
-0.9510565163,-0.9297764859,-0.9048270525,-0.8763066800,
-0.8443279255,-0.8090169944,-0.7705132428,-0.7289686274,
-0.6845471059,-0.6374239897,-0.5877852523,-0.5358267950,
-0.4817536741,-0.4257792916,-0.3681245527,-0.3090169944,
-0.2486898872,-0.1873813146,-0.1253332336,-0.0627905195,
0.0000000000, 0.0627905195, 0.1253332336, 0.1873813146,
 0.2486898872, 0.3090169944, 0.3681245527, 0.4257792916,
 0.4817536741, 0.5358267950, 0.5877852523, 0.6374239897,
 0.6845471059, 0.7289686274, 0.7705132428, 0.8090169944,
 0.8443279255, 0.8763066800, 0.9048270525, 0.9297764859,
 0.9510565163, 0.9685831611, 0.9822872507, 0.9921147013,
 0.9980267284
};

// temp buffer to store the ADC values | length = 600
uint16_t ADC_buffer[ADC_BUFFER_SIZE];
// temp buffer
uint16_t ADC_buffer_temp[ADC_BUFFER_SIZE/2];
float ADC_buffer_temp_float[ADC_BUFFER_SIZE/2];
// output buffer
uint16_t LOCK_IN_OUT[ADC_BUFFER_SIZE/2];

// pointers to the ADC buffer using ping-pong technique
uint16_t *ADC_buffer1 = &ADC_buffer[0];
uint16_t *ADC_buffer2 = &ADC_buffer[ADC_BUFFER_SIZE/2];



uint16_t temp[4];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint16_t calculate_mean(uint16_t* samples, uint16_t num_samples);
float calculata_mean_float(float* samples, uint16_t num_samples);
void replace_dot_with_comma(char* str);



void lock_in_calculation(void);// all variables are global for now

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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_DAC_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */


  char buffer[50];
  sprintf(buffer, "TEST...\n\r");
  HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);


  // start the ADC with IT
  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_buffer, ADC_BUFFER_SIZE);

  // start the DAC channel 1 with DMA
  HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)sineLookupTable, LookUpTableSize, DAC_ALIGN_12B_R);


  //HAL_ADC_Start_IT(&hadc1);
    // start the timer 3
    HAL_TIM_Base_Start(&htim3);
    // start the timer 2
    HAL_TIM_Base_Start(&htim2);


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
// half transfer callback function for the ADC
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{

    // prescale the ADC value to  range -3.3V to 3.3V and store it in the ADC_buffer_temp_float
    for (uint16_t i = 0; i < ADC_BUFFER_SIZE/2; i++)
    {
        ADC_buffer_temp_float[i] = (float)ADC_buffer1[i] * 1.0 / 4095.0;
    }

    // calculate the mean value of the ADC_buffer_temp_float
    float mean = calculata_mean_float(ADC_buffer_temp_float, ADC_BUFFER_SIZE/2);

    // // remove the DC offset
    // for (uint16_t i = 0; i < ADC_BUFFER_SIZE/2; i++)
    // {
    //     ADC_buffer_temp_float[i] = ADC_buffer_temp_float[i] - mean;
    // }


  // iterate over the first half of the buffer
  for (uint16_t i = 0; i < ADC_BUFFER_SIZE/2; i++)
  {

    // remove the DC offset
    ADC_buffer_temp_float[i] = ADC_buffer_temp_float[i] - mean;

    // char buffer[50];
    // sprintf(buffer, "Signal2 = %f.\r\n",ADC_buffer_temp_float[i]);
    // HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

    // multiply the ADC value with the sine wave value
    ADC_buffer_temp_float[i] = ADC_buffer_temp_float[i]*sineLookupTableFloat[i%100];

    // IIR filter
    xv[0] = xv[1]; 
    xv[1] = ADC_buffer_temp_float[i] / GAIN;
    yv[0] = yv[1]; 
    yv[1] =   (xv[0] + xv[1])
                  + (  0.9995288721 * yv[0]);
    ADC_buffer_temp_float[i] = yv[1];

    // multiply by the square root of 2
    ADC_buffer_temp_float[i] = ADC_buffer_temp_float[i] * 1.41421356237;


    // send the value every 10 samples
//    if(i%299 == 0)
//    {
//      // send the value to the serial port
//      char buffer[50];
//      sprintf(buffer, "Signal1 = %lu, Signal2 = %lu\r\n", ADC_buffer_temp[i],ADC_buffer1[i]);
//      HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
//    }

//     // send the value to the serial port
//     char buffer[50];
// //    sprintf(buffer, "Signal1 = %lu\r\n", ADC_buffer_temp[i]);
//     sprintf(buffer, "Signal1 = %lu\r\n", ADC_buffer_temp[i]);
//     HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);


      char buffer[50];
      sprintf(buffer, "Signal2 = %f\r\n",ADC_buffer_temp_float[i]);
      //replace_dot_with_comma(buffer);
      HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);




  }
  // take whole ADC_buffer_temp and transmit it via UART by adding the "Signal1 = " string to every value using sprintf
  //HAL_UART_Transmit(&huart2, (uint8_t*)ADC_buffer_temp, ADC_BUFFER_SIZE/2, HAL_MAX_DELAY);


}


// callback function for the ADC conversion complete
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{

    // prescale the ADC value to  range -3.3V to 3.3V and store it in the ADC_buffer_temp_float
    for (uint16_t i = 0; i < ADC_BUFFER_SIZE/2; i++)
    {
        ADC_buffer_temp_float[i] = (float)ADC_buffer1[i] * 1.0 / 4095.0;
    }

    // calculate the mean value of the ADC_buffer_temp_float
    float mean = calculata_mean_float(ADC_buffer_temp_float, ADC_BUFFER_SIZE/2);

    // // remove the DC offset
    // for (uint16_t i = 0; i < ADC_BUFFER_SIZE/2; i++)
    // {
    //     ADC_buffer_temp_float[i] = ADC_buffer_temp_float[i] - mean;
    // }


  // iterate over the first half of the buffer
  for (uint16_t i = 0; i < ADC_BUFFER_SIZE/2; i++)
  {

    // remove the DC offset
    ADC_buffer_temp_float[i] = ADC_buffer_temp_float[i] - mean;

    // char buffer[50];
    // sprintf(buffer, "Signal2 = %f.\r\n",ADC_buffer_temp_float[i]);
    // HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);

    // multiply the ADC value with the sine wave value
    ADC_buffer_temp_float[i] = ADC_buffer_temp_float[i]*sineLookupTableFloat[i%100];

    // IIR filter
    xv[0] = xv[1]; 
    xv[1] = ADC_buffer_temp_float[i] / GAIN;
    yv[0] = yv[1]; 
    yv[1] =   (xv[0] + xv[1])
                  + (  0.9995288721 * yv[0]);
    ADC_buffer_temp_float[i] = yv[1];

    // multiply by the square root of 2
    ADC_buffer_temp_float[i] = ADC_buffer_temp_float[i] * 1.41421356237;


    // send the value every 10 samples
//    if(i%299 == 0)
//    {
//      // send the value to the serial port
//      char buffer[50];
//      sprintf(buffer, "Signal1 = %lu, Signal2 = %lu\r\n", ADC_buffer_temp[i],ADC_buffer1[i]);
//      HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
//    }

//     // send the value to the serial port
//     char buffer[50];
// //    sprintf(buffer, "Signal1 = %lu\r\n", ADC_buffer_temp[i]);
//     sprintf(buffer, "Signal1 = %lu\r\n", ADC_buffer_temp[i]);
//     HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);


      char buffer[50];
      sprintf(buffer, "Signal2 = %f\r\n",ADC_buffer_temp_float[i]);
      //replace_dot_with_comma(buffer);
      HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);




  }
  // take whole ADC_buffer_temp and transmit it via UART by adding the "Signal1 = " string to every value using sprintf
  //HAL_UART_Transmit(&huart2, (uint8_t*)ADC_buffer_temp, ADC_BUFFER_SIZE/2, HAL_MAX_DELAY);


}

// Function to replace dot with comma in a string
void replace_dot_with_comma(char* str) {
    while (*str != '\0') {
        if (*str == '.') {
            *str = ',';
        }
        str++;
    }
}




// Function to calculate the mean value using chunked summation
uint16_t calculate_mean(uint16_t* samples, uint16_t num_samples) {
    uint32_t partial_sums[NUM_CHUNKS] = {0};
    uint32_t total_sum = 0;
    uint16_t chunk_size = num_samples / NUM_CHUNKS;

    // Calculate partial sums
    for (uint16_t i = 0; i < NUM_CHUNKS; i++) {
        for (uint16_t j = 0; j < chunk_size; j++) {
            partial_sums[i] += samples[i * chunk_size + j];
        }
    }

    // Combine partial sums
    for (uint16_t i = 0; i < NUM_CHUNKS; i++) {
        total_sum += partial_sums[i];
    }

    // Handle any remaining samples
    uint16_t remaining_samples = num_samples % NUM_CHUNKS;
    for (uint16_t i = num_samples - remaining_samples; i < num_samples; i++) {
        total_sum += samples[i];
    }

    // Calculate mean
    return (uint16_t)(total_sum / num_samples);
}

float calculata_mean_float(float* samples, uint16_t num_samples) {
    float partial_sums[NUM_CHUNKS] = {0};
    float total_sum = 0;
    uint16_t chunk_size = num_samples / NUM_CHUNKS;

    // Calculate partial sums
    for (uint16_t i = 0; i < NUM_CHUNKS; i++) {
        for (uint16_t j = 0; j < chunk_size; j++) {
            partial_sums[i] += samples[i * chunk_size + j];
        }
    }

    // Combine partial sums
    for (uint16_t i = 0; i < NUM_CHUNKS; i++) {
        total_sum += partial_sums[i];
    }

    // Handle any remaining samples
    uint16_t remaining_samples = num_samples % NUM_CHUNKS;
    for (uint16_t i = num_samples - remaining_samples; i < num_samples; i++) {
        total_sum += samples[i];
    }

    // Calculate mean
    return total_sum / num_samples;
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
