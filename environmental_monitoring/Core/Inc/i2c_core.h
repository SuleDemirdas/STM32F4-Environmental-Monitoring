/*
 * i2c_core.h
 *
 *  Created on: Apr 6, 2026
 *      Author: Şule Nur Demirdaş
 */

#ifndef INC_I2C_CORE_H_
#define INC_I2C_CORE_H_

#include "stdint.h"
#include "stm32f4xx_hal.h"

typedef enum
{
	HUMIDITY_SENSOR,
	PRESSURE_SENSOR,
	LIGHT_SENSOR
}sensor_t;

float i2c_sensor_read(uint8_t device_address, sensor_t sensor_type);
uint8_t I2C_ScanDeviceAddress(void);

#endif /* INC_I2C_CORE_H_ */
