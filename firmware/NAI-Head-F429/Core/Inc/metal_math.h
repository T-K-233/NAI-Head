/*
 * metal.h
 *
 *  Created on: Dec 14, 2024
 *      Author: TK
 */

#ifndef INC_METAL_MATH_H_
#define INC_METAL_MATH_H_


static inline float clampf(float value, float min, float max) {
  value = value > min ? value : min;
  value = value < max ? value : max;
  return value;
}


#endif /* INC_METAL_MATH_H_ */
