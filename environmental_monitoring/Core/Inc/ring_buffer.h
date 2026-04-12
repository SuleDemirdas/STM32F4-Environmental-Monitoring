/**
 * @file ring_buffer.h
 * @brief Circular (ring) buffer for float sensor samples.
 *
 * Provides a fixed-size, overwrite-on-full circular buffer used to store
 * the most recent RING_BUFFER_SIZE filtered sensor readings. When the buffer
 * is full, the oldest sample is silently overwritten by the newest one.
 *
 * @author Şule Nur Demirdaş
 * @date   April 2026
 */

#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_

#include "stdint.h"
#include "stdbool.h"

/**
 * @brief Ring buffer control structure.
 *
 * All fields should be treated as private; use the API functions below
 * to interact with the buffer.
 */
typedef struct
{
    float    *buffer; /**< Pointer to the externally allocated storage array. */
    uint16_t  head;   /**< Write index — points to the next free slot. */
    uint16_t  tail;   /**< Read index  — points to the oldest sample. */
    uint16_t  count;  /**< Number of valid samples currently in the buffer. */
    uint16_t  size;   /**< Total capacity of the buffer (number of floats). */
} buf_handle_t;

/**
 * @brief Initialises a ring buffer handle.
 *
 * Must be called once before any other buffer operation. The caller
 * provides the backing storage array and its capacity.
 *
 * @param[out] p_handle  Pointer to the buffer handle to initialise.
 * @param[in]  p_buffer  Pointer to the float array used as storage.
 * @param[in]  size      Number of elements in @p p_buffer.
 */
void buffer_init(buf_handle_t *p_handle, float *p_buffer, uint16_t size);

/**
 * @brief Checks whether the buffer is full.
 * @param[in] p_handle Pointer to the buffer handle.
 * @return true if count == size, false otherwise.
 */
bool buffer_isFull(buf_handle_t *p_handle);

/**
 * @brief Resets the buffer to an empty state without clearing storage memory.
 * @param[in,out] p_handle Pointer to the buffer handle.
 */
void buffer_clear(buf_handle_t *p_handle);

/**
 * @brief Checks whether the buffer is empty.
 * @param[in] p_handle Pointer to the buffer handle.
 * @return true if count == 0, false otherwise.
 */
bool buffer_isEmpty(buf_handle_t *p_handle);

/**
 * @brief Reads and removes the oldest sample from the buffer (FIFO order).
 *
 * @param[in,out] p_handle      Pointer to the buffer handle.
 * @param[out]    p_sensor_data Pointer to the float that receives the value.
 * @return  0 on success, -1 if the buffer is empty.
 */
int buffer_get_value(buf_handle_t *p_handle, float *p_sensor_data);

/**
 * @brief Writes a new sample into the buffer.
 *
 * If the buffer is full the oldest sample is overwritten (count is kept
 * at size). This ensures the buffer always holds the most recent samples.
 *
 * @param[in,out] p_handle      Pointer to the buffer handle.
 * @param[in]     p_sensor_data Float value to store.
 * @return Always 0.
 */
int buffer_write_value(buf_handle_t *p_handle, float p_sensor_data);

#endif /* INC_RING_BUFFER_H_ */
