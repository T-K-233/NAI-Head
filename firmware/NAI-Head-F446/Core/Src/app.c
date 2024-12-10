/*
 * app.c
 *
 *  Created on: Jul 13, 2024
 *      Author: TK
 */

#include "app.h"


#define ADC_SAMPLES    128

#define ADC_BASELINE    0


extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart2;

uint16_t counter = 0;
uint8_t value = 0;


uint8_t completed = 0;

uint16_t adc_data_buffer_1[ADC_SAMPLES];
uint16_t adc_data_buffer_2[ADC_SAMPLES];
int16_t data_1[ADC_SAMPLES];
int16_t data_2[ADC_SAMPLES];

float time_delay_avg = 0;


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  completed = 1;
}



void APP_init() {
  char str[128];
  sprintf(str, "start.\n");
  HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 100);

  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);


  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_8B_R, value);
  value += 1;


}

void APP_main() {
  char str[128];

  HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_8B_R, value);
  value += 1;

  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_data_buffer_1, ADC_SAMPLES);
  HAL_ADC_Start_DMA(&hadc2, (uint32_t *)adc_data_buffer_2, ADC_SAMPLES);
  HAL_TIM_Base_Start(&htim2);

  while (!completed) {}
  completed = 0;
  HAL_TIM_Base_Stop(&htim2);

  int16_t max_signal = INT16_MIN;
  int16_t min_signal = INT16_MAX;

  uint32_t sum_1 = 0;
  uint32_t sum_2 = 0;

  for (size_t i=0; i<ADC_SAMPLES; i+=1) {
    sum_1 += adc_data_buffer_1[i];
    sum_2 += adc_data_buffer_2[i];
  }

  uint16_t mean_1 = sum_1 / ADC_SAMPLES;
  uint16_t mean_2 = sum_2 / ADC_SAMPLES;

  for (size_t i=0; i<ADC_SAMPLES; i+=1) {
    data_1[i] = adc_data_buffer_1[i] - mean_1;
    data_2[i] = adc_data_buffer_2[i] - mean_2;

    if (data_1[i] > max_signal) {
      max_signal = data_1[i];
    }
    if (data_1[i] < min_signal) {
      min_signal = data_1[i];
    }
  }



  if (max_signal - min_signal > 200) {

  // sprintf(str, "max_signal: %d, min_signal: %d, mean_1: %d, mean_2: %d\n", max_signal, min_signal, mean_1, mean_2);
  // HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 100);

  // Calculate cross-correlation
  int32_t max_correlation = 0;
  int16_t time_delay = 0;
  
  const int16_t max_shift = ADC_SAMPLES / 4;
  
  for (int16_t shift = -max_shift; shift < max_shift; shift+=1) {
    int32_t correlation = 0;
    
    for (size_t i = 0; i < ADC_SAMPLES; i+=1) {
      int32_t j = i + shift;
      if (j >= 0 && j < ADC_SAMPLES) {
        correlation += (int32_t)data_1[i] * (int32_t)data_2[j];
      }
    }
    
    if (correlation > max_correlation) {
      max_correlation = correlation;
      time_delay = shift;
    }

  }

  time_delay_avg += 0.1 * (float)time_delay;

  sprintf(str, "Time delay: %d\n", (int32_t)(time_delay_avg * 100));
  HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 100);

  // Output the time delay
  // sprintf(str, "max_correlation: %ld, Time delay: %d samples\n", max_correlation, time_delay);
  // HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 100);

  // Original debug output
//  for (size_t i=0; i<ADC_SAMPLES; i+=1) {
//    sprintf(str, "0 %d %d 4096\n", adc_data_buffer_1[i], adc_data_buffer_2[i]);
//    HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 100);
//  }

//  HAL_Delay(100);
  }

  counter += 1;
}
