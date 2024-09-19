/*
 * app.c
 *
 *  Created on: Jul 13, 2024
 *      Author: TK
 */

#include "app.h"


extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart2;


// load the weight data block from the model.bin file
INCLUDE_FILE(".rodata", "./img.bin", image);
extern uint8_t image_data[];
extern size_t image_start[];
extern size_t image_end[];


#define EYELID_L_FULLOPEN         1750
#define EYELID_R_FULLOPEN         1250
#define EYELID_L_FULLCLOSE        1280
#define EYELID_R_FULLCLOSE        1720


#define EYE_MOVEMENT_X_SCALE      30.f
#define EYE_MOVEMENT_Y_SCALE      30.f


#define N_STATES    6


/**
 * state[0]: EyeOpenLeft
 * state[1]: EyeOpenRight
 * state[2]: EyeLeftX
 * state[3]: EyeLeftY
 * state[4]: EyeRightX
 * state[5]: EyeRightY
 *
 */
float state[N_STATES];



uint8_t counter;


GC9A01A tft1;
GC9A01A tft2;


void set_left_eye_openness(float val) {
  val = val < 1 ? val : 1;
  val = val > 0 ? val : 0;

  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, EYELID_L_FULLCLOSE * (1 - val) + EYELID_L_FULLOPEN * val);
}

void set_right_eye_openness(float val) {
  val = val < 1 ? val : 1;
  val = val > 0 ? val : 0;

  __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, EYELID_R_FULLCLOSE * (1 - val) + EYELID_R_FULLOPEN * val);
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  {
//    char str[256];
//    sprintf(str, "%f %f %f %f %f %f\n",
//        state[0], state[1],
//        state[2], state[3],
//        state[4], state[5]
//      );
//    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 100);
  }

  HAL_UART_Receive_IT(&huart2, (uint8_t *)state, N_STATES * sizeof(float));
}


void APP_init() {
  for (size_t i = 0; i < N_STATES; i += 1) {
    state[i] = 0;
  }


  HAL_UART_Receive_IT(&huart2, (uint8_t *)state, N_STATES * sizeof(float));


  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  GC9A01A_init(&tft1, &hspi1,
    TFT1_CS_GPIO, TFT1_CS_PIN,
    TFT1_DC_GPIO, TFT1_DC_PIN,
    TFT1_BL_GPIO, TFT1_BL_PIN,
    TFT1_RST_GPIO, TFT1_RST_PIN
  );

  GC9A01A_init(&tft2, &hspi2,
    TFT2_CS_GPIO, TFT2_CS_PIN,
    TFT2_DC_GPIO, TFT2_DC_PIN,
    TFT2_BL_GPIO, TFT2_BL_PIN,
    TFT2_RST_GPIO, TFT2_RST_PIN
  );


  // slow drawing
//  for (size_t i=0; i<tft1.width; i+=1) {
//    for (size_t j=0; j<tft1.height; j+=1) {
//      GC9A01A_draw_pixel(&tft1, j, i, 0xFFFF);
//      GC9A01A_draw_pixel(&tft2, j, i, 0xFFFF);
//    }
//  }
}

void APP_main() {
  set_left_eye_openness(state[0]);
  set_right_eye_openness(state[1]);

  int16_t eye_l_x_pixels = (int16_t)(-state[2] * EYE_MOVEMENT_X_SCALE);
  int16_t eye_l_y_pixels = (int16_t)(-state[3] * EYE_MOVEMENT_Y_SCALE);

  int16_t eye_r_x_pixels = (int16_t)(-state[4] * EYE_MOVEMENT_X_SCALE);
  int16_t eye_r_y_pixels = (int16_t)(-state[5] * EYE_MOVEMENT_Y_SCALE);

  // left eye
  GC9A01A_draw_pixels(&tft1, eye_l_x_pixels, eye_l_y_pixels, (uint16_t *)image_data, 240, 240);

  // right eye
  GC9A01A_draw_pixels(&tft2, eye_r_x_pixels, eye_r_y_pixels, (uint16_t *)image_data, 240, 240);

  counter += 1;
}
