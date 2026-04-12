/*
 * ring_buffer.h
 *
 *  Created on: Apr 10, 2026
 *      Author: Şule Nur Demirdaş
 */


#include "ring_buffer.h"

void buffer_init(buf_handle_t *p_handle, float *p_buffer, uint16_t size)
{
	p_handle->buffer = p_buffer;
	p_handle->tail = 0;
	p_handle->head = 0;
	p_handle->count = 0;
	p_handle->size = size;
}

bool buffer_isFull(buf_handle_t *p_handle)
{
	if(p_handle->count == p_handle->size)
	{
		return true;
	}
	return false;
}

void buffer_clear(buf_handle_t *p_handle)
{
	p_handle->head = 0;
	p_handle->tail = 0;
	p_handle->count = 0;
}

bool buffer_isEmpty(buf_handle_t *p_handle)
{
	if(p_handle->count == 0 )
	{
		return true;
	}
	return false;
}

int buffer_get_value(buf_handle_t *p_handle, float *p_sensor_data)
{
	if(buffer_isEmpty(p_handle))
	{
		return -1;
	}
	*p_sensor_data = p_handle->buffer[p_handle->tail];
	p_handle->tail = (p_handle->tail +1) % p_handle->size;
	p_handle->count--;
	return 0;
}
int buffer_write_value(buf_handle_t *p_handle, float p_sensor_data)
{
    if (p_handle->count < p_handle->size)
    {
        p_handle->count++;
    }

    p_handle->buffer[p_handle->head] = p_sensor_data;
    p_handle->head = (p_handle->head + 1) % p_handle->size;
    return 0;
}
