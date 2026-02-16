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
 
 
 #define EYEBROW_L_FULL_RAISE    1820
 #define EYEBROW_L_FLAT     1520
 #define EYEBROW_L_FULL_DOWN     1320
 #define EYEBROW_R_FULL_RAISE    1080
 #define EYEBROW_R_FLAT     1380
 #define EYEBROW_R_FULL_DOWN     1600
 
 #define EYELID_L_FULL_OPEN      1690
 #define EYELID_L_FULL_CLOSE     1130
 #define EYELID_R_FULL_OPEN      1250
 #define EYELID_R_FULL_CLOSE     1800
 
 
 typedef struct {
   TIM_HandleTypeDef *eyebrow_htim;
   uint32_t eyebrow_left_channel;
   uint32_t eyebrow_right_channel;
   TIM_HandleTypeDef *eyelid_htim;
   uint32_t eyelid_left_channel;
   uint32_t eyelid_right_channel;
 } ActuatorControl;
 
 
 void robot_actuator_init(ActuatorControl *robot,
   TIM_HandleTypeDef *eyebrow_htim,
   uint32_t eyebrow_left_channel,
   uint32_t eyebrow_right_channel,
   TIM_HandleTypeDef *eyelid_htim,
   uint32_t eyelid_left_channel,
   uint32_t eyelid_right_channel
 );
 
 
 void robot_set_eyebrow(ActuatorControl *robot, float left, float right);
 
 void robot_set_eyelid(ActuatorControl *robot, float left_eye_openness, float right_eye_openness);
 
 
 #endif /* INC_ACTUATORS_H_ */
 