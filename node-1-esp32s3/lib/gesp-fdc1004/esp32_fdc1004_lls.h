/*
    Driver code for Capacitive Liquid Level Sensor
    Based on FDC1004 IC from Texas Instruments

    Author: Gabriel Thien

    Inspired by Arduino driver written by Ashwin Whitchurch (Protocentral)
*/
#pragma once

// #include <driver/i2c.h>
#include <driver/i2c_master.h>
#include "esp_log.h"
#include <freertos/FreeRTOS.h>

// #include "MovingAverage.h"

#include "communication.h"

#define FDC_TAG "FDC1004"

#define FDC_SLAVE_ADDRESS 0b1010000

#define FDC1004_100HZ (0x1)
#define FDC1004_200HZ (0x2)
#define FDC1004_400HZ (0x3)
#define FDC1004_IS_RATE(x) (FDC1004_100HZ <= x && x <= FDC1004_400HZ)

#define FDC1004_CAPDAC_MAX (0x1F)

#define FDC1004_IS_CHANNEL(x) (0x0 <= x && x < 0x4)

#define FDC1004_IS_CONFIG_ADDRESS(a) (0x08 <= a && a <= 0x0B)

#define FDC1004_IS_MSB_ADDRESS(a) (a % 2 == 0 && 0x00 <= a && a <= 0x06)
#define FDC1004_IS_LSB_ADDRESS(a) (a % 2 == 1 && 0x01 <= a && a <= 0x07)

#define FDC_REGISTER (0x0C)
#define FDC_DEVICE_ID_REG (0xFF)

#define ATTOFARADS_UPPER_WORD (457) // number of attofarads for each 8th most lsb (lsb of the upper 16 bit half-word)
#define FEMTOFARADS_CAPDAC (3028)   // number of femtofarads for each lsb of the capdac

#define FDC1004_UPPER_BOUND ((int16_t)0x4000)
#define FDC1004_LOWER_BOUND (-1 * FDC1004_UPPER_BOUND)

#define GAIN_CAL 1
#define OFFSET_CAL -10

const uint8_t config_address[4] = {0x08, 0x09, 0x0A, 0x0B};
const uint8_t msb_addresses[4] = {0x00, 0x02, 0x04, 0x06};
const uint8_t lsb_addresses[4] = {0x01, 0x03, 0x05, 0x07};
const uint8_t offset_registers[4] = {0x0D, 0x0E, 0x0F, 0x10};
const uint8_t gain_registers[4] = {0x11, 0x12, 0x13, 0x14};

// Calibration Parameters
#define REF_BASELINE 1.80 // can be replaced with environment later
#define REF_FULL 2.4      // can be replaced with environment later
#define LEV_BASELINE 6.28

#define FORECAST_NUM_INCREMENTS 20

// Increase if undershooting, Decrease if overshooting
#define CORRECTION_MULTIPLIER 1.00
#define CORRECTION_OFFSET -10

#define CALIBRATION_FREQ 5000 // frequency of self calibration (ms)

#define ENV_CHANNEL 1 // CIN1
#define LEV_CHANNEL 2 // CIN2
#define REF_CHANNEL 3 // CIN3
#define LNV_CHANNEL 4

// Measurement Output
struct fdc1004_channel
{
    // Shouldn't be changed
    i2c_master_dev_handle_t slave_handle;

    uint8_t channel;
    uint8_t rate;
    uint8_t config_address;
    uint8_t msb_address;
    uint8_t lsb_address;
    uint8_t offset_register;
    uint8_t gain_register;

    // Utility
    // moving_average_t ma;

    // Continuously changed
    uint16_t raw_msb;
    uint16_t raw_lsb;
    int capdac;
    float raw_value;
    float value;
};
typedef struct fdc1004_channel* fdc_channel_t;

// Level Calculator Struct
typedef struct level_calculator
{
    float current_delta; // Current delta that the sensor is calibrated for

    // Correction Values (calculated and used in level predictions)
    float correction_gain;
    float correction_offset;

    // Result Values (Constantly updated)
    float ref_value;
    float lev_value;
    float env_value;

    // Channel Objects
    i2c_master_bus_handle_t master_bus;
    i2c_master_dev_handle_t slave_handle;

    // Channels
    fdc_channel_t ref_channel;
    fdc_channel_t lev_channel;
    fdc_channel_t env_channel;
} level_calculator;
typedef level_calculator *level_calc_t;

/**
 * @brief Software reset the FDC1004. Reads the device ID for response from the FDC as well.
 *
 * @param slave_handle I2C handle to the FDC1004
 *
 * @return ESP_OK
 */
esp_err_t fdc_reset(i2c_master_dev_handle_t slave_handle);

/**
 * @brief Frees associated memory with pointer
 *
 * @param channel_obj Pointer to channel struct
 *
 * @return ESP_OK if all good.
 */
esp_err_t del_channel(fdc_channel_t channel_obj);

/**
 * @brief Triggers and updates measurements of the channel struct
 *
 * @param level_calc Pointer to level calculator
 *
 * @return ESP_OK if good, ESP_ERR_INVLD_ARG if there is mismatch data
 */
esp_err_t update_measurements(level_calc_t level_calc);

/**
 * @brief Updates the capdac associated to the channel
 *
 * @param channel_obj Pointer to channel struct
 *
 * @return ESP_OK if good, ESP_ERR_INVLD_ARG if there is mismatch data
 */
esp_err_t update_capdac(fdc_channel_t channel_obj);

/**
 * @brief Initialises a level calculator struct for storing all computation data related to levels
 *
 * @param master_bus I2C master bus handle
 * 
 * @return Pointer to level_calc_t struct, NULL if failed
 */
level_calc_t init_fdc1004(i2c_master_bus_handle_t master_bus);

/**
 * @brief Force calibrates the level calculator linear correction
 *
 * @param level level_t struct pointer
 *
 * @return ESP_OK if good, ESP_ERR_INVLD_ARG if there is mismatch data
 */
esp_err_t calibrate(level_calc_t level);

/**
 * @brief Calculates the current predicted level through linear correction
 *
 * @param level level_t struct pointer
 *
 * @return unsigned integer
 */
uint8_t calculate_level(level_calc_t level);

void fdc1004_main(void *pvParameters);