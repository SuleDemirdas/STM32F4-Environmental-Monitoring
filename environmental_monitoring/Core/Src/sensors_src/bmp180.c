/*
 * bmp180.c
 *
 *  Created on: Apr 6, 2026
 *      Author: Şule Nur Demirdaş
 */


#include "bmp180.h"


extern I2C_HandleTypeDef hi2c3;

BMP180_CalibrationData_t calib_data_t;

int8_t BMP180_get_cal_param(void)
{
	uint8_t buffer_calib_data_u8[BMP180_CALIB_DATA_SIZE] = {0};

	for( int i = 0; i < BMP180_CALIB_DATA_SIZE; i++ )
	{
		uint8_t data = 0;
		if(HAL_I2C_Mem_Read(&hi2c3, BMP180_ADDRESS, BMP180_REG_CALIB_DATA_START + i, 1, &data, 1, 100) == HAL_OK)
		{
			buffer_calib_data_u8[i] = data;
		}
		else
		{
			return -1;
		}
	}
	calib_data_t.AC1 = (buffer_calib_data_u8[0] << 8) | buffer_calib_data_u8[1];
	calib_data_t.AC2 = (buffer_calib_data_u8[2] << 8) | buffer_calib_data_u8[3];
	calib_data_t.AC3 = (buffer_calib_data_u8[4] << 8) | buffer_calib_data_u8[5];
	calib_data_t.AC4 = (buffer_calib_data_u8[6] << 8) | buffer_calib_data_u8[7];
	calib_data_t.AC5 = (buffer_calib_data_u8[8] << 8) | buffer_calib_data_u8[9];
	calib_data_t.AC6 = (buffer_calib_data_u8[10] << 8) | buffer_calib_data_u8[11];
	calib_data_t.B1  = (buffer_calib_data_u8[12] << 8) | buffer_calib_data_u8[13];
	calib_data_t.B2  = (buffer_calib_data_u8[14] << 8) | buffer_calib_data_u8[15];
	calib_data_t.MB  = (buffer_calib_data_u8[16] << 8) | buffer_calib_data_u8[17];
	calib_data_t.MC  = (buffer_calib_data_u8[18] << 8) | buffer_calib_data_u8[19];
	calib_data_t.MD  = (buffer_calib_data_u8[20] << 8) | buffer_calib_data_u8[21];

	return 1;
}

int8_t BMP180_get_ut(int32_t *ut_result)
{
	BMP180_CtrlMeas_t ctrlMeas;
	ctrlMeas.bits.measure = BMP180_MEASURE_TEMP;
	ctrlMeas.bits.sco = 1;
	ctrlMeas.bits.oss = 0;

	uint8_t read_buffer[2] = {0};

	if(HAL_I2C_Mem_Write(&hi2c3, BMP180_ADDRESS, BMP180_REG_CTRL_MEAS, 1, &ctrlMeas.all, 1, 100) != HAL_OK)
	{
		return -1;
	}
	HAL_Delay(5);
	if(HAL_I2C_Mem_Read(&hi2c3, BMP180_ADDRESS, BMP180_REG_OUT_MSB, 1, read_buffer, 2, 100) == HAL_OK)
	{
		*ut_result = (read_buffer[0] << 8) | read_buffer[1];
		return 0;
	}
	return -1;
}

int8_t BMP180_get_up(int32_t *up_result)
{
	BMP180_CtrlMeas_t ctrlMeas;
	ctrlMeas.bits.measure = BMP180_MEASURE_PRESSURE;
	ctrlMeas.bits.sco = 1;
	ctrlMeas.bits.oss = 0;

	uint8_t read_buffer[3] = {0};

	if(HAL_I2C_Mem_Write(&hi2c3, BMP180_ADDRESS, BMP180_REG_CTRL_MEAS, 1, &ctrlMeas.all, 1, 100) != HAL_OK)
	{
		return -1;
	}
	HAL_Delay(5);
	if(HAL_I2C_Mem_Read(&hi2c3, BMP180_ADDRESS, BMP180_REG_OUT_MSB, 1, read_buffer, 3, 100) == HAL_OK)
	{
		*up_result = ((read_buffer[0] << 16) | (read_buffer[1] << 8) | read_buffer[2]) >> (8 - ctrlMeas.bits.oss);
		return 0;
	}
	return -1;
}
