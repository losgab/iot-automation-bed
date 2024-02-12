/**
 * RMT Backend driver for LED Strip Library for ESP-IDF
 *
 * @author Gabriel Thien 2024
 */
#pragma once

#include <esp_err.h>
#include <driver/rmt_types.h>
#include <driver/rmt_tx.h>
#include <stdint.h>

#include "gled_strip.h"
#include "gled_strip_rmt_encoder.h"

#define GLED_STRIP_RMT_DEFAULT_RESOLUTION 10000000 // 10MHz resolution
#define GLED_STRIP_RMT_DEFAULT_TRANS_QUEUE_SIZE 4

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define LED_STRIP_RMT_DEFAULT_MEM_BLOCK_SYMBOLS 64
#else
#define LED_STRIP_RMT_DEFAULT_MEM_BLOCK_SYMBOLS 48
#endif

#define RMT_DEVICE_TAG "RMT_DEVICE"

// /**
//  * @brief LED Strip RMT configuration
//  */
// typedef struct
// {
//     rmt_channel_handle_t rmt_chan; // RMT Channel Handle
//     rmt_clock_source_t clk_src;    // RMT clock source
//     uint32_t resolution_hz;        // RMT tick resolution, if set to zero, a default resolution (10MHz) will be applied
//     size_t mem_block_symbols;      // How many RMT symbols can one RMT channel hold at one time. Set to 0 will fallback to use the default size.
//     uint8_t with_dma;              // whether to enable the DMA feature
// } gled_strip_rmt_config_t;

/**
 * @brief LED Strip RMT Device Object
 */
typedef struct
{
    gled_strip_rmt_encoder *strip_encoder;  // RMT Strip Encoder Handle
    uint16_t num_leds;                      // Max number of LEDS attached on LED strip
    uint8_t bytes_per_pixel;                // Bytes per pixel
    uint8_t pixel_buffer[3 * MAX_NUM_LEDS]; // Pixel buffer to store pixel values
    struct
    {
        rmt_channel_handle_t rmt_chan; // RMT Channel Handle
        rmt_clock_source_t clk_src;    // RMT clock source
        uint32_t resolution_hz;        // RMT tick resolution, if set to zero, a default resolution (10MHz) will be applied
        size_t mem_block_symbols;      // How many RMT symbols can one RMT channel hold at one time. Set to 0 will fallback to use the default size.
        uint8_t with_dma;              // whether to enable the DMA feature
        uint8_t invert_out;            // Invert output signal
    } rmt_config;
} gled_strip_rmt_device;

/**
 * @brief Function Interface for LED strip
 */
typedef struct
{
    /**
     * Interface definitions are linked by RMT device initialisation
     */

    /**
     * @brief Interface function for setting colour of entire strip
     *
     * @param strip LED strip handle
     * @param index Index of the LED to set
     * @param red   Red value of the LED
     * @param green Green value of the LED
     * @param blue  Blue value of the LED
     *
     * @return ESP_OK on success, otherwise an error code
     */
    esp_err_t (*set_pixel)(gled_strip_rmt_device *rmt_device, uint16_t index, uint8_t red, uint8_t green, uint8_t blue);

    /**
     * @brief Interface function for refreshing LED strip
     *
     * @param strip LED strip handle
     *
     * @return ESP_OK on success, otherwise an error code
     */
    esp_err_t (*refresh)(gled_strip_rmt_device *rmt_device);

    /**
     * @brief Interface function for turning off LED strip
     *
     * @param strip LED strip handle
     *
     * @return ESP_OK on success, otherwise an error code
     */
    esp_err_t (*clear)(gled_strip_rmt_device *rmt_device);

    /**
     * @brief Interface function for deleting gled strip resources
     *
     * @param strip LED strip handle
     *
     * @return ESP_OK on success, otherwise an error code
     */
    esp_err_t (*del)(gled_strip_rmt_device *rmt_device);
} gled_strip_rmt_interface;

/**
 * @brief Initialize RMT device with encoder, pixel control and RMT transmission
 * (ESP-IDF Remote Control)
 *
 * @param strip LED Strip handle
 *
 * @return esp_err_t ESP_OK on success
 */
static esp_err_t gled_strip_new_rmt_device(gled_strip_rmt_device *rmt_device, gpio_num_t pin, uint16_t num_leds);

/**
 * @brief Initialise RMT interface
 * 
 * @param interface Interface handle
 * 
 * @return esp_err_t ESP_OK on success
*/
static esp_err_t gled_strip_new_rmt_interface(gled_strip_rmt_interface *interface);

/**
 * @brief Set pixel colour of the LED strip. Does not refresh.
 * (ESP-IDF Remote Control)
 *
 * @param strip LED Strip handle
 * @param index Pixel index
 * @param red   Red value of the LED
 * @param green Green value of the LED
 * @param blue  Blue value of the LED
 *
 * @return esp_err_t ESP_OK on success
 */
static esp_err_t gled_strip_rmt_set_pixel(gled_strip_rmt_device *rmt_device, uint16_t index, uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Transmit no loop TX configuration to LED strip to refresh LED strip
 * (ESP-IDF Remote Control)
 *
 * @param strip LED Strip handle
 *
 * @return esp_err_t ESP_OK on success
 */
static esp_err_t gled_strip_rmt_refresh(gled_strip_rmt_device *rmt_device);

/**
 * @brief Clear all pixels to no colour & refreshes LED strip
 * (ESP-IDF Remote Control)
 *
 * @param strip LED Strip handle
 *
 * @return esp_err_t ESP_OK on success
 */
static esp_err_t gled_strip_rmt_clear(gled_strip_rmt_device *rmt_device);

/**
 * @brief Delete RMT device
 *
 * @param strip LED Strip handle
 *
 * @return esp_err_t ESP_OK on success
 */
static esp_err_t gled_strip_rmt_del(gled_strip_rmt_device *rmt_device);
