/*
 * aht20.c
 *
 *  Created on: Apr 9, 2026
 *      Author: Şule Nur Demirdaş
 */


#include "aht20.h"

int8_t AHT20_Init(AHT20_HandleTypeDef *dev) {
    uint8_t status = 0;
    uint8_t cmd_buffer[3];

    if (dev->i2c_read == NULL || dev->i2c_write == NULL || dev->delay_ms == NULL) {
        return -1;
    }
    dev->delay_ms(40);

    cmd_buffer[0] = AHT20_CMD_STATUS;
    if (dev->i2c_write(AHT20_ADDRESS, 0, cmd_buffer, 1) != 0) {
        return -2;
    }
	if (dev->i2c_read(AHT20_ADDRESS, 0, &status, 1) != 0) {
        return -3;
    }

    if ((status & 0x08) == 0) {
        cmd_buffer[0] = AHT20_CMD_INIT;
        cmd_buffer[1] = 0x08;
        cmd_buffer[2] = 0x00;

        if (dev->i2c_write(AHT20_ADDRESS, 0, cmd_buffer, 3) != 0) {
            return -4;
        }

        dev->delay_ms(10);
    }

    return 0;
}
