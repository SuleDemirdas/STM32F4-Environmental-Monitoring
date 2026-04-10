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

int8_t AHT20_Read(AHT20_HandleTypeDef *dev) {
    uint8_t cmd_buffer[3];
    uint8_t rx_buffer[7];
    uint32_t raw_humidity = 0;
    uint32_t raw_temperature = 0;

    if (dev->i2c_read == NULL || dev->i2c_write == NULL || dev->delay_ms == NULL) {
        return -1;
    }

    cmd_buffer[0] = AHT20_CMD_TRIGGER;
    cmd_buffer[1] = 0x33;
    cmd_buffer[2] = 0x00;

    if (dev->i2c_write(AHT20_ADDRESS, 0, cmd_buffer, 3) != 0) {
        return -2;
    }


    dev->delay_ms(80);

    if (dev->i2c_read(AHT20_ADDRESS, 0, rx_buffer, 7) != 0) {
        return -3;
    }

    if ((rx_buffer[0] & 0x80) != 0) {
        return -4;
    }

    raw_humidity = ((uint32_t)rx_buffer[1] << 12) | ((uint32_t)rx_buffer[2] << 4) | (rx_buffer[3] >> 4);

    raw_temperature = (((uint32_t)(rx_buffer[3] & 0x0F)) << 16) | ((uint32_t)rx_buffer[4] << 8) | rx_buffer[5];

    dev->humidity = ((float)raw_humidity / 1048576.0f) * 100.0f;
    dev->temperature = (((float)raw_temperature / 1048576.0f) * 200.0f) - 50.0f;

    // Not: rx_buffer[6] içerisinde CRC verisi var, şimdilik atlıyoruz.

    return 0;
}
