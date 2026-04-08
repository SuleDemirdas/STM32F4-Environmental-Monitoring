/*
 * bmp180.c
 *
 *  Created on: Apr 6, 2026
 *      Author: Şule Nur Demirdaş
 */


#include "bmp180.h"


int8_t BMP180_Init(BMP180_HandleTypeDef *dev) {
    uint8_t chip_id = 0;
    if (dev->i2c_read == NULL || dev->i2c_write == NULL || dev->delay_ms == NULL) {
        return -1;
    }

    if (dev->i2c_read(BMP180_ADDRESS, BMP180_REG_CHIP_ID, &chip_id, 1) != 0) {
        return -1;
    }

    if (chip_id != 0x55)
    {
    	return -2;
    }

	uint8_t buffer_calib_data_u8[BMP180_CALIB_DATA_SIZE] = {0};

	if(dev->i2c_read(BMP180_ADDRESS, BMP180_REG_CALIB_DATA_START, buffer_calib_data_u8, 22) == 0)
	{
		dev->calib.AC1 = (buffer_calib_data_u8[0] << 8) | buffer_calib_data_u8[1];
		dev->calib.AC2 = (buffer_calib_data_u8[2] << 8) | buffer_calib_data_u8[3];
		dev->calib.AC3 = (buffer_calib_data_u8[4] << 8) | buffer_calib_data_u8[5];
		dev->calib.AC4 = (buffer_calib_data_u8[6] << 8) | buffer_calib_data_u8[7];
		dev->calib.AC5 = (buffer_calib_data_u8[8] << 8) | buffer_calib_data_u8[9];
		dev->calib.AC6 = (buffer_calib_data_u8[10] << 8) | buffer_calib_data_u8[11];
		dev->calib.B1  = (buffer_calib_data_u8[12] << 8) | buffer_calib_data_u8[13];
		dev->calib.B2  = (buffer_calib_data_u8[14] << 8) | buffer_calib_data_u8[15];
		dev->calib.MB  = (buffer_calib_data_u8[16] << 8) | buffer_calib_data_u8[17];
		dev->calib.MC  = (buffer_calib_data_u8[18] << 8) | buffer_calib_data_u8[19];
		dev->calib.MD  = (buffer_calib_data_u8[20] << 8) | buffer_calib_data_u8[21];
	}
	else
	{
		return -1;
	}

    return 0;
}

int8_t BMP180_Read(BMP180_HandleTypeDef *dev)
{
	int32_t ut = 0;
	int32_t up = 0;

	if (BMP180_get_ut(dev, &ut) != 0)
	{
		return -1;
	}
	BMP180_calc_temperature(dev, ut);

	if (BMP180_get_up(dev, &up) != 0)
	{
		return -2;
	}
	BMP180_calc_pressure(dev, up);
	return 0;
}

int8_t BMP180_Read_Temperature(BMP180_HandleTypeDef *dev)
{
	int32_t ut = 0;
	if (BMP180_get_ut(dev, &ut) != 0)
	{
		return -1;
	}
	BMP180_calc_temperature(dev, ut);
	return 0;
}

int8_t BMP180_get_ut(BMP180_HandleTypeDef *dev, int32_t *ut_result)
{
	BMP180_CtrlMeas_t ctrlMeas;
	ctrlMeas.bits.measure = BMP180_MEASURE_TEMP;
	ctrlMeas.bits.oss = dev->oss;
	ctrlMeas.bits.sco = 1;

	uint8_t read_buffer[2] = {0};

	if(dev->i2c_write(BMP180_ADDRESS, BMP180_REG_CTRL_MEAS, &ctrlMeas.all, 1) != 0)
	{
		return -1;
	}
	switch (dev->oss) {
		case 0:
			dev->delay_ms(5);
			break;
		case 1:
			dev->delay_ms(8);
			break;
		case 2:
			dev->delay_ms(14);
			break;
		case 3:
			dev->delay_ms(26);
			break;
		default:
			dev->delay_ms(5);
			break;
	}
	if(dev->i2c_read(BMP180_ADDRESS, BMP180_REG_OUT_MSB,read_buffer, 2) == 0)
	{
		*ut_result = (read_buffer[0] << 8) | read_buffer[1];
		return 0;
	}
	return -1;
}

int8_t BMP180_get_up(BMP180_HandleTypeDef *dev, int32_t *up_result)
{
	BMP180_CtrlMeas_t ctrlMeas;
	ctrlMeas.bits.measure = BMP180_MEASURE_PRESSURE;
	ctrlMeas.bits.oss = dev->oss;
	ctrlMeas.bits.sco = 1;

	uint8_t read_buffer[3] = {0};

	if(dev->i2c_write(BMP180_ADDRESS, BMP180_REG_CTRL_MEAS, &ctrlMeas.all, 1) != 0)
	{
		return -1;
	}
	switch (dev->oss) {
		case 0:
			dev->delay_ms(5);
			break;
		case 1:
			dev->delay_ms(8);
			break;
		case 2:
			dev->delay_ms(14);
			break;
		case 3:
			dev->delay_ms(26);
			break;
		default:
			dev->delay_ms(5);
			break;
	}
	if(dev->i2c_read(BMP180_ADDRESS, BMP180_REG_OUT_MSB, read_buffer, 3) == 0)
	{
		*up_result = ((read_buffer[0] << 16) | (read_buffer[1] << 8) | read_buffer[2]) >> (8 - dev->oss);
		return 0;
	}
	return -1;
}

void BMP180_calc_temperature(BMP180_HandleTypeDef *dev, int32_t ut)
{
	int32_t X1, X2;
	float temp_celsius;
	X1 = ((ut - dev->calib.AC6) * dev->calib.AC5) >> 15;
	X2 = (dev->calib.MC << 11) / (X1 + dev->calib.MD);
	dev->b5 = X1 + X2;

	int32_t T = (dev->b5 + 8) >> 4;
	temp_celsius = T / 10.0f;

	dev->temperature_C = temp_celsius;
}

void BMP180_calc_pressure(BMP180_HandleTypeDef *dev, int32_t up)
{
	int32_t X1, X2, X3, B3, B6, p;
	uint32_t B4, B7;
	float pressure_pa;

	B6 = dev->b5 - 4000;

	// Calculating B3
	X1 = (dev->calib.B2 * ((B6 * B6) >> 12)) >> 11;
	X2 = (dev->calib.AC2 * B6) >> 11;
	X3 = X1 + X2;
	B3 = ((((int32_t)dev->calib.AC1 * 4 + X3) << dev->oss) + 2) / 4;

	// Calculating B4
	X1 = (dev->calib.AC3 * B6) >> 13;
	X2 = (dev->calib.B1 * ((B6 * B6) >> 12)) >> 16;
	X3 = ((X1 + X2) + 2) >> 2;
	B4 = (dev->calib.AC4 * (uint32_t)(X3 + 32768)) >> 15;

	// Calculating B7
	B7 = ((uint32_t)up - B3) * (50000 >> dev->oss);

	if (B7 < 0x80000000) {
		p = (B7 * 2) / B4;
	} else {
		p = (B7 / B4) * 2;
	}

	X1 = (p >> 8) * (p >> 8);
	X1 = (X1 * 3038) >> 16;
	X2 = (-7357 * p) >> 16;
	p = p + ((X1 + X2 + 3791) >> 4);

	pressure_pa = (float)p;

	dev->pressure_Pa = pressure_pa;
}

