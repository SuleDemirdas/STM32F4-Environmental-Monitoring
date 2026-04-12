/**
 * @file bmp180.h
 * @brief Driver interface for the BMP180 barometric pressure and temperature sensor.
 *
 * The BMP180 uses a fixed I2C address of 0x77 and requires a one-time read of
 * 11 factory calibration coefficients stored in non-volatile memory. These
 * coefficients are used in a multi-step integer compensation algorithm specified
 * by Bosch to convert raw ADC values into temperature (°C) and pressure (Pa).
 *
 * The oversampling setting (oss) controls the trade-off between conversion time
 * and noise: 0 = ultra-low power, 3 = ultra-high resolution.
 *
 * Platform independence is achieved through injected I2C and delay function
 * pointers, identical in style to the AHT20 and BH1750 drivers.
 *
 * @author Şule Nur Demirdaş
 * @date   April 2026
 */

#ifndef INC_SENSORS_INC_BMP180_H_
#define INC_SENSORS_INC_BMP180_H_

#include "stdint.h"
#include "stddef.h"

/** @brief Function pointer type for platform I2C read. */
typedef int8_t (*BMP180_I2C_Read_Func)(uint8_t dev_addr, uint8_t reg_addr,
                                        uint8_t *data, uint16_t len);
/** @brief Function pointer type for platform I2C write. */
typedef int8_t (*BMP180_I2C_Write_Func)(uint8_t dev_addr, uint8_t reg_addr,
                                         uint8_t *data, uint16_t len);
/** @brief Function pointer type for platform millisecond delay. */
typedef void   (*BMP180_Delay_Func)(uint32_t ms);

/**
 * @brief BMP180 factory calibration coefficients.
 *
 * Read once in BMP180_Init() from the sensor's EEPROM (registers 0xAA–0xBF).
 * Used by BMP180_calc_temperature() and BMP180_calc_pressure().
 */
typedef struct
{
    int16_t  AC1; /**< Calibration coefficient AC1. */
    int16_t  AC2; /**< Calibration coefficient AC2. */
    int16_t  AC3; /**< Calibration coefficient AC3. */
    uint16_t AC4; /**< Calibration coefficient AC4. */
    uint16_t AC5; /**< Calibration coefficient AC5. */
    uint16_t AC6; /**< Calibration coefficient AC6. */
    int16_t  B1;  /**< Calibration coefficient B1. */
    int16_t  B2;  /**< Calibration coefficient B2. */
    int16_t  MB;  /**< Calibration coefficient MB. */
    int16_t  MC;  /**< Calibration coefficient MC. */
    int16_t  MD;  /**< Calibration coefficient MD. */
} BMP180_CalibrationData_t;

/**
 * @brief BMP180 driver handle.
 *
 * Populate i2c_read, i2c_write, delay_ms and oss before calling BMP180_Init().
 * After a successful BMP180_Read() the results are in temperature_C and
 * pressure_Pa. The b5 field is an intermediate compensation value shared
 * between the temperature and pressure calculation steps.
 */
typedef struct {
    BMP180_I2C_Read_Func     i2c_read;    /**< Platform I2C read function pointer. */
    BMP180_I2C_Write_Func    i2c_write;   /**< Platform I2C write function pointer. */
    BMP180_Delay_Func        delay_ms;    /**< Platform delay function pointer. */
    uint8_t                  oss;         /**< Oversampling setting (0–3). */
    BMP180_CalibrationData_t calib;       /**< Factory calibration coefficients. */
    int32_t                  b5;          /**< Intermediate temperature compensation value. */
    float                    temperature_C; /**< Last measured temperature (°C). */
    float                    pressure_Pa;  /**< Last measured pressure (Pa). */
} BMP180_HandleTypeDef;

/* ---------- Constants ---------- */
#define BMP180_CALIB_DATA_SIZE          22   /**< Size of the calibration data block in bytes. */
#define BMP180_ADDRESS                  0xEE /**< 8-bit I2C address (0x77 << 1). */
#define BMP180_REG_CALIB_DATA_START     0xAA /**< First calibration register address. */
#define BMP180_REG_CALIB_DATA_END       0xBF /**< Last calibration register address. */
#define BMP180_REG_OUT_XLSB             0xF8 /**< ADC output XLSB register. */
#define BMP180_REG_OUT_LSB              0xF7 /**< ADC output LSB register. */
#define BMP180_REG_OUT_MSB              0xF6 /**< ADC output MSB register. */
#define BMP180_REG_CTRL_MEAS            0xF4 /**< Control/measurement register. */
#define BMP180_REG_SOFT_RST             0xE0 /**< Soft-reset register. */
#define BMP180_REG_CHIP_ID              0xD0 /**< Chip ID register (expected value: 0x55). */
#define BMP180_MEASURE_TEMP             0x0E /**< ctrl_meas value to start a temperature read. */
#define BMP180_MEASURE_PRESSURE         0x14 /**< ctrl_meas value to start a pressure read. */

/**
 * @brief Bit-field overlay for the BMP180 ctrl_meas register (0xF4).
 */
typedef union
{
    uint8_t all; /**< Raw register byte. */
    struct {
        uint8_t measure : 5; /**< Measurement command (MEASURE_TEMP or MEASURE_PRESSURE). */
        uint8_t sco     : 1; /**< Start-of-conversion bit; set to 1 to trigger measurement. */
        uint8_t oss     : 2; /**< Oversampling setting (0–3). */
    } bits;
} BMP180_CtrlMeas_t;

/**
 * @brief Initialises the BMP180 sensor and reads calibration data.
 *
 * Verifies the chip ID (0x55) and reads all 11 calibration coefficients from
 * the sensor's EEPROM into dev->calib.
 *
 * @param[in,out] dev Pointer to a BMP180_HandleTypeDef with function pointers set.
 * @return  0   on success.
 * @return -1   if a function pointer is NULL, the chip ID read fails, or
 *              calibration data cannot be read.
 * @return -2   if the chip ID value is not 0x55.
 */
int8_t BMP180_Init(BMP180_HandleTypeDef *dev);

/**
 * @brief Reads both temperature and pressure from the BMP180.
 *
 * Calls BMP180_get_ut(), BMP180_calc_temperature(), BMP180_get_up() and
 * BMP180_calc_pressure() in sequence. Results are stored in dev->temperature_C
 * and dev->pressure_Pa.
 *
 * @param[in,out] dev Pointer to an initialised BMP180_HandleTypeDef.
 * @return  0   on success.
 * @return -1   if the uncompensated temperature read fails.
 * @return -2   if the uncompensated pressure read fails.
 */
int8_t BMP180_Read(BMP180_HandleTypeDef *dev);

/**
 * @brief Reads only the temperature from the BMP180.
 * @param[in,out] dev Pointer to an initialised BMP180_HandleTypeDef.
 * @return  0 on success, -1 on I2C error.
 */
int8_t BMP180_Read_Temperature(BMP180_HandleTypeDef *dev);

/**
 * @brief Triggers and reads the raw (uncompensated) temperature ADC value.
 * @param[in,out] dev       Pointer to an initialised BMP180_HandleTypeDef.
 * @param[out]    ut_result Pointer to store the raw temperature value.
 * @return  0 on success, -1 on I2C error.
 */
int8_t BMP180_get_ut(BMP180_HandleTypeDef *dev, int32_t *ut_result);

/**
 * @brief Triggers and reads the raw (uncompensated) pressure ADC value.
 * @param[in,out] dev       Pointer to an initialised BMP180_HandleTypeDef.
 * @param[out]    up_result Pointer to store the raw pressure value.
 * @return  0 on success, -1 on I2C error.
 */
int8_t BMP180_get_up(BMP180_HandleTypeDef *dev, int32_t *up_result);

/**
 * @brief Converts a raw temperature ADC value to degrees Celsius.
 *
 * Uses the Bosch compensation algorithm. Also computes and stores dev->b5
 * which is required by BMP180_calc_pressure().
 *
 * @param[in,out] dev Pointer to an initialised BMP180_HandleTypeDef.
 * @param[in]     ut  Raw uncompensated temperature value from BMP180_get_ut().
 */
void BMP180_calc_temperature(BMP180_HandleTypeDef *dev, int32_t ut);

/**
 * @brief Converts a raw pressure ADC value to Pascals.
 *
 * Implements the full Bosch pressure compensation algorithm.
 * BMP180_calc_temperature() must be called first to populate dev->b5.
 *
 * @param[in,out] dev Pointer to an initialised BMP180_HandleTypeDef.
 * @param[in]     up  Raw uncompensated pressure value from BMP180_get_up().
 */
void BMP180_calc_pressure(BMP180_HandleTypeDef *dev, int32_t up);

#endif /* INC_SENSORS_INC_BMP180_H_ */
