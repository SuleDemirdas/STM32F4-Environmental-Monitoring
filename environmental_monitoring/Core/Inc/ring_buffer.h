/*
 * ring_buffer.h
 *
 *  Created on: Apr 10, 2026
 *      Author: Şule Nur Demirdaş
 */

#ifndef INC_RING_BUFFER_H_
#define INC_RING_BUFFER_H_

#include "stdint.h"
#include "stdbool.h"

typedef struct
{
	float *buffer;
	uint16_t head;
	uint16_t tail;
	uint16_t count;
	uint16_t size;
}buf_handle_t;

void buffer_init(buf_handle_t *p_handle, float *p_buffer, uint16_t size);
bool buffer_isFull(buf_handle_t *p_handle);
void buffer_clear(buf_handle_t *p_handle);
bool buffer_isEmpty(buf_handle_t *p_handle);
int buffer_get_value(buf_handle_t *p_handle, float *p_sensor_data);
int buffer_write_value(buf_handle_t *p_handle, float p_sensor_data);

#endif /* INC_RING_BUFFER_H_ */
