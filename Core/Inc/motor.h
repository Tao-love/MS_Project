/*
 * motor.h
 *
 *  Created on: May 25, 2026
 *      Author: ZhuanZ（无密码）
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#include "main.h"
#include "tim.h"

#define MOTOR1 1
#define MOTOR2 2
#define speed_L_init 50
#define speed_R_init 50

void Motor_Init(void);
void Motor_Forward(uint8_t motor_id);
void Motor_Reverse(uint8_t motor_id);
void Motor_SlowStop(uint8_t motor_id);
void Motor_FastStop(uint8_t motor_id);
void Motor_SetSpeed(uint8_t motor_id, uint16_t speed);
void Car_Forward(void);
void Car_Reverse(void);
void Car_SlowStop(void);
void Car_FastStop(void);

#endif /* INC_MOTOR_H_ */
