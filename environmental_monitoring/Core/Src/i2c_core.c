/*
 * i2c_core.c
 *
 *  Created on: Apr 6, 2026
 *      Author: Şule Nur Demirdaş
 */


#include "i2c_core.h"

float i2c_sensor_read(uint8_t device_address, sensor_t sensor_type, void *p_handle)
{
	if( p_handle == NULL )
	{
		return -1.0f;
	}
	float result_f = 0.0f;

	switch (sensor_type) {
		case HUMIDITY_SENSOR:
			if(AHT20_Read((AHT20_HandleTypeDef*)p_handle) != 0)
			{
				return -1.0f;
			}
			result_f = ((AHT20_HandleTypeDef*)p_handle)->humidity;
			break;
		case PRESSURE_SENSOR:
			if(BMP180_Read((BMP180_HandleTypeDef*)p_handle) != 0)
			{
				return -1.0f;
			}
			result_f = ((BMP180_HandleTypeDef*)p_handle)->pressure_Pa;
			break;
		case LIGHT_SENSOR:
			if(BH1750_Read((BH1750_HandleTypeDef*)p_handle) != 0)
			{
				return -1.0f;
			}
			result_f = ((BH1750_HandleTypeDef*)p_handle)->lux;
			break;
		case TEMPERATURE_SENSOR:
			if(AHT20_Read((AHT20_HandleTypeDef*)p_handle) != 0)
			{
				return -1.0f;
			}
			result_f = ((AHT20_HandleTypeDef*)p_handle)->temperature;
			break;
		default:
			result_f = -1.0f; // unknown sensor type
			break;
	}
	return result_f;
}

uint8_t I2C_ScanDeviceAddress(I2C_HandleTypeDef *hi2c)
{
	uint8_t device_address_u8 = 0x00;

	for(int i = 0; i < 128; i++ )
	{
		if(HAL_I2C_IsDeviceReady(hi2c, (device_address_u8 << 1), 3, 100) == HAL_OK)
		{
			return device_address_u8;
		}
		device_address_u8++;
	}
	return 0xFF;
}
