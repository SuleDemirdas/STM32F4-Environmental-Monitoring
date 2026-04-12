/**
 * @file filter.c
 * @brief Sliding-window median filter implementation.
 *
 * @author Şule Nur Demirdaş
 * @date   April 2026
 */

#include "filter.h"

float filter_sensor_value(Filter_Handle_t *p_filt, float raw_sensor_value,
                           uint8_t window_size)
{
    if (p_filt->count < window_size)
    {
        /* Window not yet full — append new sample. */
        p_filt->window[p_filt->count] = raw_sensor_value;
        p_filt->count++;
    }
    else
    {
        /* Window full — shift samples left to evict the oldest, then append. */
        for (int i = 0; i < window_size - 1; i++)
            p_filt->window[i] = p_filt->window[i + 1];
        p_filt->window[window_size - 1] = raw_sensor_value;
    }

    /* Sort a copy so the original window order is preserved. */
    float sorted_window[MAX_WINDOW_SIZE];
    for (int i = 0; i < p_filt->count; i++)
        sorted_window[i] = p_filt->window[i];

    bubble_sort(sorted_window, p_filt->count);
    return calculate_median(sorted_window, p_filt->count);
}

void bubble_sort(float *array, uint8_t array_size)
{
    float temp;
    for (uint8_t i = 0; i < array_size - 1; i++)
    {
        for (uint8_t j = 0; j < array_size - i - 1; j++)
        {
            if (array[j] > array[j + 1])
            {
                temp         = array[j];
                array[j]     = array[j + 1];
                array[j + 1] = temp;
            }
        }
    }
}

float calculate_median(float *array, uint8_t array_size)
{
    if (array_size % 2 == 0)
        return (array[array_size / 2] + array[(array_size / 2) - 1]) / 2.0f;
    else
        return array[(array_size - 1) / 2];
}
