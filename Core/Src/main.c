/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body (精简版，仅保留电机控制)
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can_user.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
// 当前目标角度（可通过按键切换进行测试）
extern volatile uint32_t stop_flag;
extern volatile uint32_t zero_request;
volatile float target_angle_cmd = 0.0f;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
// 声明 C++ 桥接函数
extern void motor_set_target_angle(float angle);
extern void motor_set_stop_flag(int flag);
extern void motor_request_zero(void);
extern void motor_set_feedforward_only(void);
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
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN1_Init();
  MX_TIM6_Init();

  /* USER CODE BEGIN 2 */
  if (HAL_CAN_ConfigFilter(&hcan1, &can_filter_config) != HAL_OK ||
      HAL_CAN_Start(&hcan1) != HAL_OK ||
      HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK ||
      HAL_TIM_Base_Start_IT(&htim6) != HAL_OK) {
      Error_Handler();
  }

  motor_set_stop_flag(1);
  motor_set_feedforward_only();
  motor_set_target_angle(0.0f);

  // 3. 设置初始目标角度为 0 度
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // 空循环，电机控制完全交给 TIM6 中断和 CAN 回调处理
    // 你可以在这里加一个延时，防止空循环占用太多CPU（虽然不占用也不影响）
    HAL_Delay(10);
    /* USER CODE END WHILE */
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

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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
// 利用按键中断来切换目标角度，方便测试前馈效果
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == KEY_Pin) // 确保 KEY_Pin 是你在 CubeMX 里配置的按键引脚宏
  {
    static uint32_t last_key_tick = 0;
    static uint8_t has_last_key_tick = 0;
    uint32_t now = HAL_GetTick();

    // 20ms 软件消抖
    if (!has_last_key_tick || (now - last_key_tick) >= 20U)
    {
      has_last_key_tick = 1;
      last_key_tick = now;

      // 切换目标角度: 0 -> 30 -> 45 -> 90 -> 0 ...
      if (target_angle_cmd == 0.0f) {
          target_angle_cmd = 30.0f;
      } else if (target_angle_cmd == 30.0f) {
          target_angle_cmd = 45.0f;
      } else if (target_angle_cmd == 45.0f) {
          target_angle_cmd = 90.0f;
      } else {
          target_angle_cmd = 0.0f;
      }

      // 将新角度传给电机
      motor_set_target_angle(target_angle_cmd);
    }
  }
}
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}