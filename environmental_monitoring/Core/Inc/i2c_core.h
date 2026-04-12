/**
 * @file i2c_core.h
 * @brief Sensor-agnostic I2C read abstraction layer.
 *
 * Provides a unified interface for reading from the three supported I2C sensors
 * (AHT20, BMP180, BH1750) through a single function call. The caller identifies
 * the desired measurement with a @ref sensor_t tag; the implementation casts the
 * generic handle pointer to the correct sensor driver type and invokes the
 * appropriate driver read function.
 *
 * Also exposes a bus-scan helper that can locate the first responding device on
 * an I2C bus — useful during bring-up and debugging.
 *
 * @author Şule Nur Demirdaş
 * @date   April 2026
 */

#ifndef INC_I2C_CORE_H_
#define INC_I2C_CORE_H_

#include "stdint.h"
#include "stm32f4xx_hal.h"
#include "main.h"

/**
 * @brief Identifies the physical quantity to be read from a sensor.
 *
 * Passed to i2c_sensor_read() together with the matching sensor handle so
 * that the function can dispatch to the correct driver and extract the
 * right field from the result.
 */
typedef enum
{
    HUMIDITY_SENSOR,    /**< Relative humidity in % RH (AHT20). */
    PRESSURE_SENSOR,    /**< Atmospheric pressure in Pa (BMP180). */
    LIGHT_SENSOR,       /**< Illuminance in lux (BH1750). */
    TEMPERATURE_SENSOR  /**< Ambient temperature in °C (AHT20). */
} sensor_t;

/**
 * @brief Reads a single measurement from the specified I2C sensor.
 *
 * The function triggers a sensor read via the appropriate driver, then
 * returns the requested physical quantity as a float. On error -1.0f is
 * returned.
 *
 * @param[in] device_address 8-bit I2C address of the target device
 *                           (left-shifted by 1 as required by HAL).
 * @param[in] sensor_type    The physical quantity to retrieve; must match
 *                           the sensor pointed to by @p p_handle.
 * @param[in] p_handle       Void pointer to the sensor driver handle
 *                           (AHT20_HandleTypeDef*, BMP180_HandleTypeDef*, or
 *                            BH1750_HandleTypeDef*).
 * @return Measured value as a float, or -1.0f on failure.
 */
float i2c_sensor_read(uint8_t device_address, sensor_t sensor_type,
                      void *p_handle);

/**
 * @brief Scans the I2C bus for the first responding device.
 *
 * Iterates through all 128 possible 7-bit addresses and returns the first
 * one that acknowledges HAL_I2C_IsDeviceReady(). Intended as a diagnostic
 * helper during hardware bring-up.
 *
 * @param[in] hi2c Pointer to the initialised I2C peripheral handle.
 * @return 7-bit address (0x00–0x7F) of the first responding device,
 *         or 0xFF if no device is found.
 */
uint8_t I2C_ScanDeviceAddress(I2C_HandleTypeDef *hi2c);

#endif /* INC_I2C_CORE_H_ */
