/*
 * motor.c
 *
 *  Created on: May 25, 2026
 *      Author: ZhuanZ（无密码）
 */

#include "motor.h"

void Motor_Init(void)
{
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

  Motor_SetSpeed(MOTOR1, speed_L_init);
  Motor_SetSpeed(MOTOR2, speed_R_init);

  Motor_SlowStop(MOTOR1);
  Motor_SlowStop(MOTOR2);
}

void Motor_SetSpeed(uint8_t motor_id, uint16_t speed)
{
  uint32_t max_pwm = __HAL_TIM_GET_AUTORELOAD(&htim2);

  if (motor_id == MOTOR1)
  {
    if (speed > max_pwm)
    {
      speed = max_pwm;
    }

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, speed);
    return;
  }

  if (motor_id == MOTOR2)
  {
    if (speed > max_pwm)
    {
      speed = max_pwm;
    }

    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, speed);
  }
}

void Motor_Forward(uint8_t motor_id)
{
  if (motor_id == MOTOR1)
  {
    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
    return;
  }

  if (motor_id == MOTOR2)
  {
    HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_RESET);
  }
}

void Motor_Reverse(uint8_t motor_id)
{
  if (motor_id == MOTOR1)
  {
    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_SET);
    return;
  }

  if (motor_id == MOTOR2)
  {
    HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_SET);
  }
}

void Motor_SlowStop(uint8_t motor_id)
{
  if (motor_id == MOTOR1)
  {
    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_RESET);
    return;
  }

  if (motor_id == MOTOR2)
  {
    HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_RESET);
  }
}

void Motor_FastStop(uint8_t motor_id)
{
  if (motor_id == MOTOR1)
  {
    HAL_GPIO_WritePin(IN1_GPIO_Port, IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN2_GPIO_Port, IN2_Pin, GPIO_PIN_SET);
    return;
  }

  if (motor_id == MOTOR2)
  {
    HAL_GPIO_WritePin(IN3_GPIO_Port, IN3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(IN4_GPIO_Port, IN4_Pin, GPIO_PIN_SET);
  }
}

void Car_Forward(void)
{
  Motor_Forward(MOTOR1);
  Motor_Forward(MOTOR2);
}

void Car_Reverse(void)
{
  Motor_Reverse(MOTOR1);
  Motor_Reverse(MOTOR2);
}

void Car_SlowStop(void)
{
  Motor_SlowStop(MOTOR1);
  Motor_SlowStop(MOTOR2);
}

void Car_FastStop(void)
{
  Motor_FastStop(MOTOR1);
  Motor_FastStop(MOTOR2);
}
