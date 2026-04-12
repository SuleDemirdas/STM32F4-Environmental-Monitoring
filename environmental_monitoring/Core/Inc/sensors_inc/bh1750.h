/**
 * @file bh1750.h
 * @brief Driver interface for the BH1750 ambient light sensor.
 *
 * The BH1750 is a 16-bit I2C ambient light sensor with a configurable
 * measurement mode. This driver supports all six modes defined in the
 * datasheet and is platform-independent through injected function pointers.
 *
 * Typical usage:
 * @code
 * BH1750_HandleTypeDef hbh1750;
 * hbh1750.address   = BH1750_ADDRESS_GND;
 * hbh1750.mode      = BH1750_ONE_TIME_H_RES_MODE;
 * hbh1750.i2c_read  = my_i2c_read;
 * hbh1750.i2c_write = my_i2c_write;
 * hbh1750.delay_ms  = my_delay;
 * BH1750_Init(&hbh1750);
 *
 * BH1750_Read(&hbh1750);
 * float lux = hbh1750.lux;
 * @endcode
 *
 * @author Şule Nur Demirdaş
 * @date   April 2026
 */

#ifndef INC_SENSORS_INC_BH1750_H_
#define INC_SENSORS_INC_BH1750_H_

#include "stdint.h"
#include "stddef.h"

/** @brief Function pointer type for platform I2C read. */
typedef int8_t (*BH1750_I2C_Read_Func)(uint8_t dev_addr, uint8_t reg_addr,
                                        uint8_t *data, uint16_t len);
/** @brief Function pointer type for platform I2C write. */
typedef int8_t (*BH1750_I2C_Write_Func)(uint8_t dev_addr, uint8_t reg_addr,
                                         uint8_t *data, uint16_t len);
/** @brief Function pointer type for platform millisecond delay. */
typedef void   (*BH1750_Delay_Func)(uint32_t ms);

/**
 * @brief BH1750 driver handle.
 *
 * Populate all fields before calling BH1750_Init(). After a successful
 * BH1750_Read() the result is available in the lux field.
 */
typedef struct {
    BH1750_I2C_Read_Func  i2c_read;  /**< Platform I2C read function pointer. */
    BH1750_I2C_Write_Func i2c_write; /**< Platform I2C write function pointer. */
    BH1750_Delay_Func     delay_ms;  /**< Platform delay function pointer. */
    uint8_t address; /**< 8-bit I2C device address (use BH1750_ADDRESS_GND or _VCC). */
    uint8_t mode;    /**< Measurement mode command byte (see mode constants below). */
    float   lux;     /**< Last measured illuminance in lux. */
} BH1750_HandleTypeDef;

/* ---------- Address constants ---------- */
#define BH1750_ADDRESS_GND  (0x23 << 1) /**< I2C address when ADDR pin = GND. */
#define BH1750_ADDRESS_VCC  (0x5C << 1) /**< I2C address when ADDR pin = VCC. */
#define BH1750_ADDRESS       BH1750_ADDRESS_GND /**< Default address alias. */

/* ---------- Measurement mode commands ---------- */
#define BH1750_CONTINIOUS_H_RES_MODE   0x10 /**< Continuous high-res mode  (1 lx,  ~120 ms). */
#define BH1750_CONTINIOUS_H_RES_MODE2  0x11 /**< Continuous high-res mode2 (0.5 lx, ~120 ms). */
#define BH1750_CONTINIOUS_L_RES_MODE   0x13 /**< Continuous low-res mode   (4 lx,  ~16 ms). */
#define BH1750_ONE_TIME_H_RES_MODE     0x20 /**< One-time high-res mode    (1 lx,  ~120 ms). */
#define BH1750_ONE_TIME_H_RES_MODE2    0x21 /**< One-time high-res mode2   (0.5 lx, ~120 ms). */
#define BH1750_ONE_TIME_L_RES_MODE     0x23 /**< One-time low-res mode     (4 lx,  ~16 ms). */

/* ---------- Power / reset commands ---------- */
#define BH1750_PWR_DOWN  0x00 /**< Power-down command. */
#define BH1750_PWR_ON    0x01 /**< Power-on command. */
#define BH1750_RST       0x07 /**< Reset data register command. */

/* ---------- Measurement timing (ms) ---------- */
#define MEASUREMENT_TIME_H_RES_MODE   120 /**< Typical conversion time for H-res modes. */
#define MEASUREMENT_TIME_H_RES_MODE2  120 /**< Typical conversion time for H-res2 mode. */
#define MEASUREMENT_TIME_L_RES_MODE    16 /**< Typical conversion time for L-res mode. */

/** @brief Lux conversion coefficient: raw / BH1750_LUX_COEFF = lux. */
#define BH1750_LUX_COEFF 1.2f

/**
 * @brief Powers on the BH1750 and sends the initial measurement mode command.
 *
 * @param[in,out] dev Pointer to a BH1750_HandleTypeDef with all fields set.
 * @return  0   on success.
 * @return -1   if a required function pointer is NULL.
 * @return -2   if an I2C write fails.
 */
int8_t BH1750_Init(BH1750_HandleTypeDef *dev);

/**
 * @brief Reads the latest illuminance measurement from the BH1750.
 *
 * For one-time modes the function re-sends the mode command and waits for
 * conversion before reading. For continuous modes it reads immediately.
 * The result is stored in dev->lux.
 *
 * @param[in,out] dev Pointer to an initialised BH1750_HandleTypeDef.
 * @return  0   on success.
 * @return -2   if an I2C write or read fails.
 */
int8_t BH1750_Read(BH1750_HandleTypeDef *dev);

#endif /* INC_SENSORS_INC_BH1750_H_ */
