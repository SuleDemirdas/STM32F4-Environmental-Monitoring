/*
 * bh1750.h
 *
 *  Created on: Apr 8, 2026
 *      Author: Şule Nur Demirdaş
 */

#ifndef INC_SENSORS_INC_BH1750_H_
#define INC_SENSORS_INC_BH1750_H_

#include "stdint.h"
#include "stddef.h"

typedef int8_t (*BH1750_I2C_Read_Func)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef int8_t (*BH1750_I2C_Write_Func)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef void   (*BH1750_Delay_Func)(uint32_t ms);

typedef struct {
	BH1750_I2C_Read_Func  i2c_read;
	BH1750_I2C_Write_Func i2c_write;
	BH1750_Delay_Func     delay_ms;
    uint8_t address;
    uint8_t mode;
    float lux;
} BH1750_HandleTypeDef;

#define BH1750_ADDRESS_GND    			(0x23 << 1) // ADDR pin = GND
#define BH1750_ADDRESS_VCC    			(0x5C << 1) // ADDR pin = VCC
#define BH1750_ADDRESS        			BH1750_ADDRESS_GND
#define BH1750_CONTINIOUS_H_RES_MODE	0x10
#define BH1750_CONTINIOUS_H_RES_MODE2	0x11
#define BH1750_CONTINIOUS_L_RES_MODE	0x13
#define BH1750_ONE_TIME_H_RES_MODE		0x20
#define BH1750_ONE_TIME_H_RES_MODE2		0x21
#define BH1750_ONE_TIME_L_RES_MODE		0x23

#define BH1750_PWR_DOWN					0x00
#define BH1750_PWR_ON					0x01
#define BH1750_RST						0x07

#define MEASUREMENT_TIME_H_RES_MODE		120
#define MEASUREMENT_TIME_H_RES_MODE2	120
#define MEASUREMENT_TIME_L_RES_MODE		16

#define BH1750_LUX_COEFF				1.2f

int8_t BH1750_Init(BH1750_HandleTypeDef * dev);
int8_t BH1750_Read(BH1750_HandleTypeDef * dev);
#endif /* INC_SENSORS_INC_BH1750_H_ */
