/*
 * i2c_core.c
 *
 *  Created on: Apr 6, 2026
 *      Author: Şule Nur Demirdaş
 */


#include "i2c_core.h"

extern I2C_HandleTypeDef hi2c3;

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

uint8_t I2C_ScanDeviceAddress(void)
{
	uint8_t device_address_u8 = 0x00;

	for(int i = 0; i < 256; i++ )
	{
		if(HAL_I2C_IsDeviceReady(&hi2c3, device_address_u8, 3, 100) == HAL_OK)
		{
			return device_address_u8;
		}
		device_address_u8++;
	}
	return 0xFF;
}
