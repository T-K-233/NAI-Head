/*
 * neck_control.c
 *
 *  Created on: Dec 14, 2024
 *      Author: TK
 */

 #include "actuators.h"


 void robot_actuator_init(
     ActuatorControl *robot,
     TIM_HandleTypeDef *eyebrow_htim,
     uint32_t eyebrow_left_channel,
     uint32_t eyebrow_right_channel,
     TIM_HandleTypeDef *eyelid_htim,
     uint32_t eyelid_left_channel,
     uint32_t eyelid_right_channel
   ) {
   robot->eyebrow_htim = eyebrow_htim;
   robot->eyebrow_left_channel = eyebrow_left_channel;
   robot->eyebrow_right_channel = eyebrow_right_channel;
 //  robot->neck_yaw_channel = neck_yaw_channel;
   robot->eyelid_htim = eyelid_htim;
   robot->eyelid_left_channel = eyelid_left_channel;
   robot->eyelid_right_channel = eyelid_right_channel;
 
   // enable PWM outputs
   HAL_TIM_PWM_Start(robot->eyebrow_htim, robot->eyebrow_left_channel);
   HAL_TIM_PWM_Start(robot->eyebrow_htim, robot->eyebrow_right_channel);
 
   HAL_TIM_PWM_Start(robot->eyelid_htim, robot->eyelid_left_channel);
   HAL_TIM_PWM_Start(robot->eyelid_htim, robot->eyelid_right_channel);
 }
 
 void robot_set_eyebrow(ActuatorControl *robot, float left, float right) {
   left = clampf(left, -1.f, 1.f);
   right = clampf(right, -1.f, 1.f);
 
 
   uint16_t left_pwm_value = EYEBROW_L_FLAT;
   uint16_t right_pwm_value = EYEBROW_R_FLAT;
 
   if (left > 0) {
     left_pwm_value += left * (EYEBROW_L_FULL_RAISE - EYEBROW_L_FLAT);
   }
   else {
     left_pwm_value -= (-left) * (EYEBROW_L_FLAT - EYEBROW_L_FULL_DOWN);
   }
 
   if (right > 0) {
     right_pwm_value -= right * (EYEBROW_R_FLAT - EYEBROW_R_FULL_RAISE);
   }
   else {
     right_pwm_value += (-right) * (EYEBROW_R_FULL_DOWN - EYEBROW_R_FLAT);
   }
 
   __HAL_TIM_SET_COMPARE(robot->eyebrow_htim, robot->eyebrow_left_channel, left_pwm_value);
   __HAL_TIM_SET_COMPARE(robot->eyebrow_htim, robot->eyebrow_right_channel, right_pwm_value);
 }
 
 
 void robot_set_eyelid(ActuatorControl *robot, float left_eye_openness, float right_eye_openness) {
   left_eye_openness = clampf(left_eye_openness, 0.f, 1.f);
   right_eye_openness = clampf(right_eye_openness, 0.f, 1.f);
 
   uint16_t left_pwm_value = EYELID_L_FULL_CLOSE * (1 - left_eye_openness) + EYELID_L_FULL_OPEN * left_eye_openness;
   uint16_t right_pwm_value = EYELID_R_FULL_CLOSE * (1 - right_eye_openness) + EYELID_R_FULL_OPEN * right_eye_openness;
 
   __HAL_TIM_SET_COMPARE(robot->eyelid_htim, robot->eyelid_left_channel, left_pwm_value);
   __HAL_TIM_SET_COMPARE(robot->eyelid_htim, robot->eyelid_right_channel, right_pwm_value);
 }
 
 