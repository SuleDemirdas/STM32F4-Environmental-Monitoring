/*
 * filter.c
 *
 *  Created on: Apr 12, 2026
 *      Author: Şule Nur Demirdaş
 */

#include "filter.h"

float filter_sensor_value(Filter_Handle_t *p_filt, float raw_sensor_value, uint8_t window_size)
{
	// Add data to the window
	if (p_filt->count < window_size) {
		p_filt->window[p_filt->count] = raw_sensor_value;
		p_filt->count++;
	}
	else
	{
		// if window is full delete old data and add new
		for(int i = 0; i < window_size - 1; i++)
		{
			p_filt->window[i] = p_filt->window[i+1];
		}
		p_filt->window[window_size - 1] = raw_sensor_value;
	}

	float sorted_window[MAX_WINDOW_SIZE];
	for (int i = 0; i < p_filt->count; i++)
	{
		sorted_window[i] = p_filt->window[i];
	}

	bubble_sort(sorted_window, p_filt->count);

	return calculate_median(sorted_window, p_filt->count);
}

void bubble_sort(float* array, uint8_t array_size)
{
    float temp;
    for (uint8_t i = 0; i < array_size - 1; i++) {
        for (uint8_t j = 0; j < array_size - i - 1; j++) {
            if (array[j] > array[j + 1]) {
            	temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

float calculate_median(float* array, uint8_t array_size)
{
	if((array_size % 2 == 0))
	{
		return (array[array_size / 2] + array[(array_size / 2) - 1]) / 2;
	}
	else
	{
		return array[(array_size - 1) / 2];
	}
}
