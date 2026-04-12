/**
 * @file filter.h
 * @brief Sliding-window median filter for floating-point sensor values.
 *
 * The filter maintains a fixed-size window of the most recent raw readings.
 * Each call to filter_sensor_value() appends the new sample to the window
 * (evicting the oldest one when full), sorts a copy of the window, and
 * returns the median. This effectively suppresses impulse noise while
 * preserving step changes faster than a moving-average filter of the same
 * length.
 *
 * Utility functions bubble_sort() and calculate_median() are also exposed
 * so that calculate_statistics() in main.c can reuse them on the ring buffer.
 *
 * @author Şule Nur Demirdaş
 * @date   April 2026
 */

#ifndef INC_FILTER_H_
#define INC_FILTER_H_

#include "stdint.h"

/** @brief Maximum number of samples the filter window can hold. */
#define MAX_WINDOW_SIZE 20

/**
 * @brief Internal state of one sliding-window median filter instance.
 */
typedef struct {
    float   window[MAX_WINDOW_SIZE]; /**< Circular window of raw samples. */
    uint8_t count;                   /**< Number of valid samples currently in the window. */
} Filter_Handle_t;

/**
 * @brief Applies a sliding-window median filter to a single raw sample.
 *
 * On each call the new sample is inserted into the window. If the window
 * has not yet reached @p window_size the sample is appended; otherwise the
 * oldest sample is discarded (left-shift) and the new one placed at the end.
 * A copy of the window is then sorted and the median returned.
 *
 * @param[in,out] p_filt           Pointer to the filter state structure.
 * @param[in]     raw_sensor_value The latest raw measurement from the sensor.
 * @param[in]     window_size      Effective window length (must be ≤ MAX_WINDOW_SIZE).
 * @return Filtered (median) value of the current window contents.
 */
float filter_sensor_value(Filter_Handle_t *p_filt, float raw_sensor_value,
                           uint8_t window_size);

/**
 * @brief Computes the median of a pre-sorted float array.
 *
 * For even-length arrays the median is the average of the two central elements.
 * For odd-length arrays it is the single central element.
 *
 * @param[in] array      Pointer to a sorted array of floats.
 * @param[in] array_size Number of elements in @p array.
 * @return Median value.
 */
float calculate_median(float *array, uint8_t array_size);

/**
 * @brief Sorts a float array in ascending order using bubble sort.
 *
 * Runs in O(n²) time; suitable for the small window sizes used here
 * (≤ MAX_WINDOW_SIZE = 20 elements).
 *
 * @param[in,out] array      Pointer to the array to sort in place.
 * @param[in]     array_size Number of elements in @p array.
 */
void bubble_sort(float *array, uint8_t array_size);

#endif /* INC_FILTER_H_ */
