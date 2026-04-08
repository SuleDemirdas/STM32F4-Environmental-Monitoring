/*
 * bmp180.h
 *
 *  Created on: Apr 6, 2026
 *      Author: Şule Nur Demirdaş
 */

#ifndef INC_SENSORS_INC_BMP180_H_
#define INC_SENSORS_INC_BMP180_H_

#include "stdint.h"
#include <stddef.h>

typedef int8_t (*BMP180_I2C_Read_Func)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef int8_t (*BMP180_I2C_Write_Func)(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint16_t len);
typedef void   (*BMP180_Delay_Func)(uint32_t ms);

typedef struct
{
	int16_t AC1;
	int16_t AC2;
	int16_t AC3;
	uint16_t AC4;
	uint16_t AC5;
	uint16_t AC6;
	int16_t B1;
	int16_t B2;
	int16_t MB;
	int16_t MC;
	int16_t MD;
}BMP180_CalibrationData_t;

typedef struct {
	BMP180_I2C_Read_Func  i2c_read;
	BMP180_I2C_Write_Func i2c_write;
	BMP180_Delay_Func     delay_ms;
    uint8_t oss;
    BMP180_CalibrationData_t calib;
    int32_t b5;
    float temperature_C;
    float pressure_Pa;
} BMP180_HandleTypeDef;

#define BMP180_CALIB_DATA_SIZE			22
#define BMP180_ADDRESS             		0xEE

#define BMP180_REG_CALIB_DATA_START 	0xAA
#define BMP180_REG_CALIB_DATA_END		0xBF
#define BMP180_REG_OUT_XLSB				0xF8
#define BMP180_REG_OUT_LSB				0xF7
#define BMP180_REG_OUT_MSB				0xF6
#define BMP180_REG_CTRL_MEAS			0xF4
#define BMP180_REG_SOFT_RST				0xE0
#define BMP180_REG_CHIP_ID				0xD0

#define BMP180_MEASURE_TEMP				0x0E
#define BMP180_MEASURE_PRESSURE			0x14

typedef union
{
	uint8_t all;
	struct{
		uint8_t measure : 5;
		uint8_t sco : 1;
		uint8_t oss : 2;
	}bits;
}BMP180_CtrlMeas_t;

int8_t BMP180_Init(BMP180_HandleTypeDef *dev);
int8_t BMP180_Read(BMP180_HandleTypeDef *dev);
int8_t BMP180_get_cal_param(void);
int8_t BMP180_get_ut(BMP180_HandleTypeDef *dev, int32_t *ut_result);
int8_t BMP180_get_up(BMP180_HandleTypeDef *dev, int32_t *up_result);
void BMP180_calc_temperature(BMP180_HandleTypeDef *dev, int32_t ut);
void BMP180_calc_pressure(BMP180_HandleTypeDef *dev, int32_t up);

#endif /* INC_SENSORS_INC_BMP180_H_ */
