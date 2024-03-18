/*
    Driver code for Capacitive Liquid Level Sensor
    Based on FDC1004 IC from Texas Instruments
    
    Author: Gabriel Thien
    
    Inspired by Arduino driver written by Ashwin Whitchurch (Protocentral)
*/

#include <driver/i2c.h>
#include "freertos/timers.h"

#include "MovingAverage.h"

#define FDC_SLAVE_ADDRESS 0b1010000

#define FDC1004_100HZ (0x01)
#define FDC1004_200HZ (0x02)
#define FDC1004_400HZ (0x03)
#define FDC1004_IS_RATE(x) (FDC1004_100HZ <= x && x <= FDC1004_400HZ)

#define FDC1004_CAPDAC_MAX (0x1F)

#define FDC1004_CHANNEL_MAX (0x03)
#define FDC1004_IS_CHANNEL(x) (x <= FDC1004_CHANNEL_MAX)

#define FDC1004_IS_CONFIG_ADDRESS(a) (0x08 <= a && a <= 0x0B)

#define FDC1004_IS_MSB_ADDRESS(a) (a % 2 == 0 && 0x00 <= a && a <= 0x06)
#define FDC1004_IS_LSB_ADDRESS(a) (a % 2 == 1 && 0x01 <= a && a <= 0x07)

#define FDC_DEVICE_ID_REG (0xFF)
#define FDC_REGISTER (0x0C)

#define ATTOFARADS_UPPER_WORD (457) //number of attofarads for each 8th most lsb (lsb of the upper 16 bit half-word)
#define FEMTOFARADS_CAPDAC (3028) //number of femtofarads for each lsb of the capdac

#define FDC1004_UPPER_BOUND ((int16_t) 0x4000)
#define FDC1004_LOWER_BOUND (-1 * FDC1004_UPPER_BOUND)

static const uint8_t config[] = {0x08, 0x09, 0x0A, 0x0B};
static const uint8_t msb_addresses[] = {0x00, 0x02, 0x04, 0x06};
static const uint8_t lsb_addresses[] = {0x01, 0x03, 0x05, 0x07};

// Calibration Parameters
#define REF_BASELINE 1.80 // can be replaced with environment later
#define REF_FULL 2.4 // can be replaced with environment later
#define LEV_BASELINE 6.28

#define FORECAST_NUM_INCREMENTS 20

// Increase if undershooting, Decrease if overshooting
#define CORRECTION_MULTIPLIER 1.00
#define CORRECTION_OFFSET -10

#define CALIBRATION_FREQ 5000 // frequency of self calibration (ms)

#define REF_CHANNEL 0
#define LEV_CHANNEL 1
#define ENV_CHANNEL 2
#define LNV_CHANNEL 3

// Measurement Output
typedef struct fdc1004_channel
{
    // Shouldn't be changed
    i2c_port_t i2c_port_num;
    uint8_t channel;
    uint8_t rate;
    uint8_t config_address;
    uint8_t msb_address;
    uint8_t lsb_address;

    // Utility
    moving_average_t ma;

    // Continuously changed
    int raw_msb;
    int raw_lsb;
    int capdac;
    float raw_value;
    float value;
} fdc1004_channel;
typedef fdc1004_channel* fdc_channel_t;

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
    fdc_channel_t ref_channel;
    fdc_channel_t lev_channel;
    fdc_channel_t env_channel;
} level_calculator;
typedef level_calculator* level_calc_t;

/**
 * @brief Software reset the FDC1004. Reads the device ID for response from the FDC as well.
 * 
 * @param i2c_port_num I2C port number
 * 
 * @return ESP_OK
*/
esp_err_t fdc_reset(i2c_port_t i2c_port_num);

/**
 * @brief Initialises a channel struct for storing all data associated with channel readings
 * 
 * @param i2c_port_num I2C port number to initialise the channel on
 * @param channel Channel number on the FDC1004 to use for readings
 * @param rate Rate at which measurements are taken on the FDC1004
 * 
 * @return Pointer to fdc_channel struct, NULL if failed because out of memory or some other internal error
*/
fdc_channel_t init_channel(i2c_port_t i2c_port_num, uint8_t channel, uint8_t rate);

/**
 * @brief Frees associated memory with pointer
 * 
 * @param channel_obj Pointer to channel struct
 * 
 * @return ESP_OK if all good.
*/
esp_err_t del_channel(fdc_channel_t channel_obj);

/**
 * @brief Validates the fields in the channel struct
 * 
 * @param channel_obj Pointer to channel struct
 *
 * @return ESP_OK if good, ESP_ERR_INVLD_ARG if there is mismatch data
*/
esp_err_t validate_channel_obj(fdc_channel_t channel_obj);

/**
 * @brief Uses I2C interface to read data at a particular address
 * 
 * @param i2c_port_num I2C port number
 * @param reg_address Address of the register in the FDC1004 to be read
 * @param ret_data Pointer to store the data read from the specified register
 * 
 * @return ESP_OK if good, else error code
*/
esp_err_t read_register(i2c_port_t i2c_port_num, uint8_t reg_address, uint8_t *ret_data);

/**
 * @brief Configures by building and configuring 16 bit configuration for config registers
 * 
 * @param channel_obj Pointer to channel struct
 * 
 * @return ESP_OK if good, ESP_ERR_INVLD_ARG if there is mismatch data
*/
esp_err_t configure_single_measurement(fdc_channel_t channel_obj);

/**
 * @brief Triggers and updates measurements of the channel struct
 * 
 * @param channel_obj Pointer to channel struct
 * 
 * @return ESP_OK if good, ESP_ERR_INVLD_ARG if there is mismatch data
*/
esp_err_t update_measurement(fdc_channel_t channel_obj);

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
 * 
 * @return Pointer to level_calc_t struct, NULL if failed
*/
level_calc_t init_level_calculator();

/**
 * @brief Handles the function call back for the timer interrupt
 * 
 * @param xTimer The parameter to be passed to the callback function
 * 
 * @return void
*/
void timer_callback(TimerHandle_t xTimer);

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

/**
 * @brief Rounds a float to 2 decimal places
 * 
 * @param value Value be rounded
 * 
 * @return float
*/
float round_2dp(float value);

/**
 * @brief Rounds to the nearest multiple of 5
 * 
 * @param value Value to be rounded
 * @param multiple Multiple to which to be rounded
 * 
 * @return unsigned integer
*/
uint8_t round_nearest_multiple(float value, uint8_t multiple);