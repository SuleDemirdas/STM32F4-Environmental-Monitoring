/*
 * filter.h
 *
 *  Created on: Apr 12, 2026
 *      Author: Şule Nur Demirdaş
 */

#ifndef INC_FILTER_H_
#define INC_FILTER_H_

#include "stdint.h"

#define MAX_WINDOW_SIZE		20
float calculate_median(float* array, uint8_t array_size);
void bubble_sort(float* array, uint8_t array_size);

#endif /* INC_FILTER_H_ */
