/**
 * @file aht20.h
 * @brief Driver interface for the AHT20 humidity and temperature sensor.
 *
 * The AHT20 communicates over I2C at a fixed address of 0x38. The driver
 * is platform-independent: the caller injects the I2C read/write and delay
 * function pointers at initialisation time, so the same code runs on any
 * MCU that provides those three primitives.
 *
 * Typical usage:
 * @code
 * AHT20_HandleTypeDef haht20;
 * haht20.i2c_read  = my_i2c_read;
 * haht20.i2c_write = my_i2c_write;
 * haht20.delay_ms  = my_delay;
 * AHT20_Init(&haht20);
 *
 * AHT20_Read(&haht20);
 * float rh   = haht20.humidity;     // % RH
 * float degC = haht20.temperature;  // °C
 * @endcode
 *
 * @author Şule Nur Demirdaş
 * @date   April 2026
 */

#ifndef INC_SENSORS_INC_AHT20_H_
#define INC_SENSORS_INC_AHT20_H_

#include "stdint.h"
#include "stddef.h"

/** @brief AHT20 I2C base address (7-bit = 0x38, shifted left for HAL). */
#define AHT20_ADDRESS     (0x38 << 1)

/** @brief Command byte: read status register. */
#define AHT20_CMD_STATUS  0x71

/** @brief Command byte: initialise / calibrate the sensor. */
#define AHT20_CMD_INIT    0xBE

/** @brief Command byte: trigger a humidity + temperature measurement. */
#define AHT20_CMD_TRIGGER 0xAC

/**
 * @brief Function pointer type for platform I2C read operations.
 *
 * @param dev_addr  8-bit device address (HAL left-shifted format).
 * @param reg_addr  Register address; 0 for raw master-receive.
 * @param data      Buffer to store received bytes.
 * @param len       Number of bytes to read.
 * @return 0 on success, non-zero on error.
 */
typedef int8_t (*AHT20_I2C_Read_Func)(uint8_t dev_addr, uint8_t reg_addr,
                                       uint8_t *data, uint16_t len);

/**
 * @brief Function pointer type for platform I2C write operations.
 *
 * @param dev_addr  8-bit device address (HAL left-shifted format).
 * @param reg_addr  Register address; 0 for raw master-transmit.
 * @param data      Buffer containing bytes to send.
 * @param len       Number of bytes to write.
 * @return 0 on success, non-zero on error.
 */
typedef int8_t (*AHT20_I2C_Write_Func)(uint8_t dev_addr, uint8_t reg_addr,
                                        uint8_t *data, uint16_t len);

/**
 * @brief Function pointer type for platform millisecond delay.
 * @param ms Delay in milliseconds.
 */
typedef void (*AHT20_Delay_Func)(uint32_t ms);

/**
 * @brief AHT20 driver handle.
 *
 * Populate i2c_read, i2c_write and delay_ms before calling AHT20_Init().
 * After a successful AHT20_Read() call the results are available in
 * the humidity and temperature fields.
 */
typedef struct {
    AHT20_I2C_Read_Func  i2c_read;   /**< Platform I2C read function pointer. */
    AHT20_I2C_Write_Func i2c_write;  /**< Platform I2C write function pointer. */
    AHT20_Delay_Func     delay_ms;   /**< Platform delay function pointer. */
    float humidity;                  /**< Last measured relative humidity (% RH). */
    float temperature;               /**< Last measured temperature (°C). */
} AHT20_HandleTypeDef;

/**
 * @brief Initialises the AHT20 sensor.
 *
 * Sends a status request and, if the calibration bit is not set, issues the
 * initialisation command. Requires a 40 ms power-on delay before calling.
 *
 * @param[in,out] dev Pointer to an AHT20_HandleTypeDef with function pointers set.
 * @return  0   on success.
 * @return -1   if any function pointer is NULL.
 * @return -2   if the status write fails.
 * @return -3   if the status read fails.
 * @return -4   if the initialisation command write fails.
 */
int8_t AHT20_Init(AHT20_HandleTypeDef *dev);

/**
 * @brief Triggers a measurement and reads humidity and temperature from the AHT20.
 *
 * Sends the trigger command, waits 80 ms for conversion, reads 7 bytes, and
 * converts the raw 20-bit values to physical units stored in dev->humidity
 * and dev->temperature. The CRC byte (rx_buffer[6]) is currently not verified.
 *
 * @param[in,out] dev Pointer to an initialised AHT20_HandleTypeDef.
 * @return  0   on success.
 * @return -1   if any function pointer is NULL.
 * @return -2   if the trigger command write fails.
 * @return -3   if the data read fails.
 * @return -4   if the busy flag in the status byte is set.
 */
int8_t AHT20_Read(AHT20_HandleTypeDef *dev);

#endif /* INC_SENSORS_INC_AHT20_H_ */
