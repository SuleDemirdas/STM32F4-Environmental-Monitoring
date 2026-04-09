/*
 * aht20.h
 *
 *  Created on: Apr 9, 2026
 *      Author: Şule Nur Demirdaş
 */

#ifndef INC_SENSORS_INC_AHT20_H_
#define INC_SENSORS_INC_AHT20_H_

#include "stdint.h"
#include "stddef.h"

#define AHT20_ADDRESS		(0x38 << 1)
#define AHT20_CMD_STATUS	0x71
#define AHT20_CMD_INIT		0xBE
#define AHT20_CMD_TRIGGER	0xAC

typedef int8_t (*AHT20_I2C_Read_Func)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef int8_t (*AHT20_I2C_Write_Func)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef void   (*AHT20_Delay_Func)(uint32_t ms);

typedef struct {
	AHT20_I2C_Read_Func  i2c_read;
	AHT20_I2C_Write_Func i2c_write;
	AHT20_Delay_Func     delay_ms;
    float humidity;
    float temperature;
} AHT20_HandleTypeDef;

int8_t AHT20_Init(AHT20_HandleTypeDef *dev);
int8_t AHT20_Read(AHT20_HandleTypeDef *dev);

#endif /* INC_SENSORS_INC_AHT20_H_ */
