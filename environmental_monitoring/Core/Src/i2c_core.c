/*
 * i2c_core.c
 *
 *  Created on: Apr 6, 2026
 *      Author: Şule Nur Demirdaş
 */


#include "i2c_core.h"

float i2c_sensor_read(uint8_t device_address, sensor_t sensor_type)
{
	float result_f = 0.0f;

	switch (sensor_type) {
		case HUMIDITY_SENSOR:
			break;
		case PRESSURE_SENSOR:
			break;
		case LIGHT_SENSOR:
			break;
		default:
			result_f = -1.0f; // unknown sensor type
			break;
	}
	return result_f;
}
