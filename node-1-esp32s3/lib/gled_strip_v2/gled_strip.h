/*
    Convenience Library for Addressable LED Strips & Colours

    @author Gabriel Thien 2024
*/
#pragma once

#include <led_strip.h>
#include <driver/gpio.h>

#define MAX_COLOURS 6
#define CHANNELS 3

typedef enum colour // Colours
{
    RED,     // 255, 0, 0
    GREEN,   // 0, 255, 0
    BLUE,    // 0, 0, 255
    YELLOW,  // 255, 255, 0
    AQUA,    // 0, 255, 255
    MAGENTA, // 255, 0, 255
} colour_t;

/**
 * @brief Convenience function for setting colour of the LED strip. Auto refreshes.
 *
 * @param strip LED strip handle
 * @param num_leds Number of LEDs
 * @param colour Colour of the LEDs from enumeration
 *
 * @return void
 */
void led_strip_set_colour(led_strip_handle_t strip, uint8_t num_leds, colour_t colour);

/**
 * @brief Convenience function for creating RGB LED strip device. Requires declaration of LED strip handle.
 * 
 * @param pin GPIO pin number 
 * @param num_leds Number of LEDs
 * @param ret_strip LED strip handle
 * 
 * @return ESP_OK on success
*/
esp_err_t create_led_strip_device(gpio_num_t pin, uint8_t num_leds, led_strip_handle_t *ret_strip);
