/**
 * Convenience Library for Addressable LED Strips & Colours
 *
 * - Defines interface functions for entire LED library
 * - Defines initialisation function for LED strip that configures all functionality
 *   including RMT device and RMT encoder.
 *
 *  @author Gabriel Thien 2024
 */
#pragma once

#include "gled_strip_rmt.h"
#include "gled_strip_colours.h"
#include <driver/gpio.h>

#include <esp_err.h>
#include <esp_check.h>

#define GLED_STRIP_TAG "GLED_STRIP"

/**
 * @brief LED Strip pixel format / LED Model
 */
typedef enum
{
    LED_PIXEL_FORMAT_GRB,
    LED_PIXEL_FORMAT_GRBW,
    LED_PIXEL_FORMAT_INVALID
} pixel_format_t;

// SUPPORTS ONLY LED_MODEL_WS2812

/**
 * @brief LED Strip Object & Interface function definition
 */
typedef struct gled_strip
{
    gpio_num_t pin;                      // GPIO Pin used by the LED Strip
    pixel_format_t format;               // Pixel format of the LED Strip
    gled_strip_rmt_device_t *rmt_device; // RMT Device Handle
    gled_strip_rmt_interface *interface; // Interface
} gled_strip_t;

/////////////////////////////////////////////////////////////
//////////////// USERLAND API FUNCTIONS /////////////////////
/////////////////////////////////////////////////////////////

/**
 * @brief Initialises LED Strip
 *
 * @param strip LED strip handle
 * @param pin GPIO Pin used by the LED Strip
 * @param num_leds Max number of LEDS attached on LED strip
 *
 * @return ESP_OK on success, otherwise an error code
 */
esp_err_t gled_strip_new(gled_strip_t *strip, gpio_num_t pin, uint16_t num_leds);

/**
 * @brief Sets colour of the entire LED strip. Also refreshes to immediately show colour.
 *
 * @param strip LED strip handle
 * @param colour Colour from enumeration
 *
 * @return ESP_OK on success, otherwise an error code
 */
esp_err_t gled_strip_set_colour(gled_strip_t *strip, colour_t colour);

/**
 * @brief Interface function for setting colour of specified pixel of the LED strip
 *
 * @param strip LED strip handle
 * @param pixel Pixel number
 * @param colour Colour from enumeration
 *
 * @return ESP_OK on success, otherwise an error code
 */
esp_err_t gled_strip_set_pixel(gled_strip_t *strip, uint16_t pixel, colour_t colour);

/**
 * @brief Convenience function for refreshing LED strip
 *
 * @param strip LED strip handle
 *
 * @return ESP_OK on success, otherwise an error code
 */
esp_err_t gled_strip_refresh(gled_strip_t *strip);

/**
 * @brief Convenience function for turning off LED strip
 *
 * @param strip LED strip handle
 *
 * @return ESP_OK on success, otherwise an error code
 */
esp_err_t gled_strip_clear(gled_strip_t *strip);
