/*
 * buzzer.c
 *
 *  Created on: May 26, 2026
 *      Author: ZhuanZ（无密码）
 */

#include "buzzer.h"

void Buzzer_Init(void)
{
  Buzzer_Off();
}

void Buzzer_On(void)
{
  HAL_GPIO_WritePin(alarm_GPIO_Port, alarm_Pin, GPIO_PIN_RESET);
}

void Buzzer_Off(void)
{
  HAL_GPIO_WritePin(alarm_GPIO_Port, alarm_Pin, GPIO_PIN_SET);
}
