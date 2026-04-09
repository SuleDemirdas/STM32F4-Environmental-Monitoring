/*
 * bh1750.c
 *
 *  Created on: Apr 8, 2026
 *      Author: Şule Nur Demirdaş
 */


#include "bh1750.h"

int8_t BH1750_Init(BH1750_HandleTypeDef * dev)
{
	uint8_t cmd;
	if (dev->i2c_write == NULL || dev->delay_ms == NULL)
	{
		return -1;
	}

	cmd = BH1750_PWR_ON;
	if (dev->i2c_write(dev->address, 0, &cmd, 1) != 0)
	{
	        return -2;
	}
	dev->delay_ms(10);

	cmd = dev->mode;
	if (dev->i2c_write(dev->address, 0, &cmd, 1) != 0)
	{
		return -2;
	}

	if (dev->mode == BH1750_CONTINIOUS_L_RES_MODE || dev->mode == BH1750_ONE_TIME_L_RES_MODE) {
		dev->delay_ms(24);
	} else {
		dev->delay_ms(180);
	}
	return 0;
}

int8_t BH1750_Read(BH1750_HandleTypeDef *dev) {
    uint8_t buffer[2] = {0};
    uint16_t raw_lux = 0;
    uint8_t cmd;

    if(dev->mode == BH1750_ONE_TIME_L_RES_MODE || dev->mode == BH1750_ONE_TIME_H_RES_MODE || dev->mode == BH1750_ONE_TIME_H_RES_MODE2 )
    {
    	cmd = dev->mode;
		if (dev->i2c_write(dev->address, 0, &cmd, 1) != 0) {
			return -2;
		}
		if (dev->mode == BH1750_ONE_TIME_L_RES_MODE) {
			dev->delay_ms(24);
		} else {
			dev->delay_ms(180);
		}
    }

    if (dev->i2c_read(dev->address, 0, buffer, 2) != 0) {
        return -2;
    }
    raw_lux = (buffer[0] << 8) | buffer[1];

    dev->lux = (float)raw_lux / BH1750_LUX_COEFF;

    return 0;
}
