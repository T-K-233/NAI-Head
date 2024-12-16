/*
 * neck_control.h
 *
 *  Created on: Dec 14, 2024
 *      Author: TK
 */

#ifndef INC_ACTUATORS_H_
#define INC_ACTUATORS_H_

#include "stm32f4xx_hal.h"
#include "metal_math.h"


#define SERVO_IDLE_PWM            1500
#define NECK_ROLL_SCALE           250
#define NECK_PITCH_SCALE          80
#define NECK_YAW_SCALE            200



#define EYELID_L_FULLOPEN         1750
#define EYELID_R_FULLOPEN         1250
#define EYELID_L_FULLCLOSE        1280
#define EYELID_R_FULLCLOSE        1720


typedef struct {
  TIM_HandleTypeDef *neck_htim;
  uint32_t neck_roll_pitch_left_channel;
  uint32_t neck_roll_pitch_right_channel;
  uint32_t neck_yaw_channel;
  TIM_HandleTypeDef *eyelid_htim;
  uint32_t eyelid_left_channel;
  uint32_t eyelid_right_channel;
} ActuatorControl;


void robot_actuator_init(ActuatorControl *robot,
  TIM_HandleTypeDef *neck_htim,
  uint32_t neck_roll_pitch_left_channel,
  uint32_t neck_roll_pitch_right_channel,
  uint32_t neck_yaw_channel,
  TIM_HandleTypeDef *eyelid_htim,
  uint32_t eyelid_left_channel,
  uint32_t eyelid_right_channel
);


void robot_set_neck_roll_pitch_yaw(ActuatorControl *robot, float roll, float pitch, float yaw);

void robot_set_eyelid(ActuatorControl *robot, float left_eye_openness, float right_eye_openness);


#endif /* INC_ACTUATORS_H_ */
