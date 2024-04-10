/*
    Driver code for Capacitive Liquid Level Sensor
    Based on FDC1004 IC from Texas Instruments

    Author: Gabriel Thien

    Inspired by Arduino driver written by Ashwin Whitchurch (Protocentral)
*/

#include <driver/i2c.h>
#include "freertos/timers.h"
#include "esp_log.h"

#include "MovingAverage.h"

#define FDC_TAG "FDC1004"

#define FDC_SLAVE_ADDRESS 0b1010000

#define FDC1004_100HZ (0x1)
#define FDC1004_200HZ (0x2)
#define FDC1004_400HZ (0x3)
#define FDC1004_IS_RATE(x) (FDC1004_100HZ <= x && x <= FDC1004_400HZ)

#define FDC1004_CAPDAC_MAX (0x1F)

#define FDC1004_CHANNEL_MIN (0x0)
#define FDC1004_CHANNEL_MAX (0x3)
#define FDC1004_IS_CHANNEL(x) (FDC1004_CHANNEL_MIN <= x && x <= FDC1004_CHANNEL_MAX)

#define FDC1004_IS_CONFIG_ADDRESS(a) (0x08 <= a && a <= 0x0B)

#define FDC1004_IS_MSB_ADDRESS(a) (a % 2 == 0 && 0x00 <= a && a <= 0x06)
#define FDC1004_IS_LSB_ADDRESS(a) (a % 2 == 1 && 0x01 <= a && a <= 0x07)

#define FDC_REGISTER (0x0C)
#define FDC_DEVICE_ID_REG (0xFF)

#define ATTOFARADS_UPPER_WORD (457) // number of attofarads for each 8th most lsb (lsb of the upper 16 bit half-word)
#define FEMTOFARADS_CAPDAC (3028)   // number of femtofarads for each lsb of the capdac

#define FDC1004_UPPER_BOUND ((int16_t)0x4000)
#define FDC1004_LOWER_BOUND (-1 * FDC1004_UPPER_BOUND)

#define GAIN_CAL 0.2
#define OFFSET_CAL -0.5

static const uint8_t config[] = {0x08, 0x09, 0x0A, 0x0B};
static const uint8_t msb_addresses[] = {0x00, 0x02, 0x04, 0x06};
static const uint8_t lsb_addresses[] = {0x01, 0x03, 0x05, 0x07};
static const uint8_t offset_registers[] = {0x0D, 0x0E, 0x0F, 0x10};
static const uint8_t gain_registers[] = {0x11, 0x12, 0x13, 0x14};

// Calibration Parameters
#define REF_BASELINE 1.80 // can be replaced with environment later
#define REF_FULL 2.4      // can be replaced with environment later
#define LEV_BASELINE 6.28

#define FORECAST_NUM_INCREMENTS 20

// Increase if undershooting, Decrease if overshooting
#define CORRECTION_MULTIPLIER 1.00
#define CORRECTION_OFFSET -10

#define CALIBRATION_FREQ 5000 // frequency of self calibration (ms)

#define REF_CHANNEL 1
#define LEV_CHANNEL 2
#define ENV_CHANNEL 3
#define LNV_CHANNEL 4

// Measurement Output
typedef struct fdc1004_channel
{
    // Shouldn't be changed
    i2c_port_t port;
    uint8_t channel;
    uint8_t rate;
    uint8_t config_address;
    uint8_t msb_address;
    uint8_t lsb_address;
    uint8_t offset_register;
    uint8_t gain_register;

    // Utility
    moving_average_t ma;

    // Continuously changed
    uint16_t raw_msb;
    uint16_t raw_lsb;
    int capdac;
    float raw_value;
    float value;
} fdc1004_channel;
typedef fdc1004_channel *fdc_channel_t;

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
    i2c_port_t port;
    fdc_channel_t ref_channel;
    fdc_channel_t lev_channel;
    fdc_channel_t env_channel;
} level_calculator;
typedef level_calculator *level_calc_t;

/**
 * @brief Software reset the FDC1004. Reads the device ID for response from the FDC as well.
 *
 * @param port I2C port number
 *
 * @return ESP_OK
 */
esp_err_t fdc_reset(i2c_port_t port);

/**
 * @brief Frees associated memory with pointer
 *
 * @param channel_obj Pointer to channel struct
 *
 * @return ESP_OK if all good.
 */
esp_err_t del_channel(fdc_channel_t channel_obj);

/**
 * @brief Uses I2C interface to read data at a particular address
 *
 * @param i2c_port_num I2C port number
 * @param reg_address Address of the register in the FDC1004 to be read
 * @param ret_data Pointer to store the data read from the specified register
 *
 * @return ESP_OK if good, else error code
 */
esp_err_t read_register(i2c_port_t i2c_port_num, uint8_t reg_address, uint16_t *ret_data);

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
 * @param void
 * @param port I2C port number
 *
 * @return Pointer to level_calc_t struct, NULL if failed
 */
level_calc_t init_level_calculator(i2c_port_t port);

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