/*
 * neck_control.c
 *
 *  Created on: Dec 14, 2024
 *      Author: TK
 */

#include "actuators.h"


void robot_actuator_init(
    ActuatorControl *robot,
    TIM_HandleTypeDef *neck_htim,
    uint32_t neck_roll_pitch_left_channel,
    uint32_t neck_roll_pitch_right_channel,
    uint32_t neck_yaw_channel,
    TIM_HandleTypeDef *eyelid_htim,
    uint32_t eyelid_left_channel,
    uint32_t eyelid_right_channel
  ) {
  robot->neck_htim = neck_htim;
  robot->neck_roll_pitch_left_channel = neck_roll_pitch_left_channel;
  robot->neck_roll_pitch_right_channel = neck_roll_pitch_right_channel;
  robot->neck_yaw_channel = neck_yaw_channel;
  robot->eyelid_htim = eyelid_htim;
  robot->eyelid_left_channel = eyelid_left_channel;
  robot->eyelid_right_channel = eyelid_right_channel;

  // enable PWM outputs
  HAL_TIM_PWM_Start(robot->neck_htim, robot->neck_roll_pitch_left_channel);
  HAL_TIM_PWM_Start(robot->neck_htim, robot->neck_roll_pitch_right_channel);
  HAL_TIM_PWM_Start(robot->neck_htim, robot->neck_yaw_channel);

  HAL_TIM_PWM_Start(robot->eyelid_htim, robot->eyelid_left_channel);
  HAL_TIM_PWM_Start(robot->eyelid_htim, robot->eyelid_right_channel);
}

void robot_set_neck_roll_pitch_yaw(ActuatorControl *robot, float roll, float pitch, float yaw) {
  roll = clampf(roll, -1.f, 1.f);
  pitch = clampf(pitch, -1.f, 1.f);
  yaw = clampf(yaw, -1.f, 1.f);

  __HAL_TIM_SET_COMPARE(robot->neck_htim, robot->neck_roll_pitch_left_channel, SERVO_IDLE_PWM + (NECK_PITCH_SCALE * pitch) + (NECK_ROLL_SCALE * roll));
  __HAL_TIM_SET_COMPARE(robot->neck_htim, robot->neck_roll_pitch_right_channel, SERVO_IDLE_PWM - (NECK_PITCH_SCALE * pitch) + (NECK_ROLL_SCALE * roll));

  __HAL_TIM_SET_COMPARE(robot->neck_htim, robot->neck_yaw_channel, SERVO_IDLE_PWM + (NECK_YAW_SCALE * yaw));
}


void robot_set_eyelid(ActuatorControl *robot, float left_eye_openness, float right_eye_openness) {
  left_eye_openness = clampf(left_eye_openness, 0.f, 1.f);
  right_eye_openness = clampf(right_eye_openness, 0.f, 1.f);

  uint16_t left_pwm_value = EYELID_L_FULLCLOSE * (1 - left_eye_openness) + EYELID_L_FULLOPEN * left_eye_openness;
  uint16_t right_pwm_value = EYELID_R_FULLCLOSE * (1 - right_eye_openness) + EYELID_R_FULLOPEN * right_eye_openness;

  __HAL_TIM_SET_COMPARE(robot->eyelid_htim, robot->eyelid_left_channel, left_pwm_value);
  __HAL_TIM_SET_COMPARE(robot->eyelid_htim, robot->eyelid_right_channel, right_pwm_value);
}

