/*
    Buttons for ESP32

    Convenience library for observing button state changes.
    - Detects when a button was pushed
    - Detects if a button is held down (not implemented yet)

    Written by Gabriel Thien 2023
*/
#pragma once
#include <driver/gpio.h>
#include <esp_err.h>

typedef struct Button
{
    uint8_t pin;
    uint8_t last_state;
    uint8_t pushed;

    // uint8_t pressed;

    /**
     * @brief Reads & updates the state of the button
     *
     * @return void
     */
    void (*update_button)(struct Button *button);

    /**
     * @brief Returns whether button was pushed since last clock cycle
     *
     * @return true if button was pushed, false if not.
     */
    bool (*was_pushed)(struct Button *button);
} button_t;

/**
 * @brief Initialises a button attached to the specified pin
 *
 * @param pin GPIO pin that button is attached to
 * @param is_input_pullup 1 -> INPUT_PULLUP, 0 for external
 *
 * @return ESP_OK if successful, ESP_ERR_INVALID_ARG if invalid pin
 */
esp_err_t create_button(uint8_t pin, uint8_t is_input_pullup, button_t *ret_button);

// /**
//  * @brief Returns whether button is being held down
//  *
//  * @param button struct Button *
//  *
//  * @return 1 if held down, 0 if not.
//  */
// uint8_t is_pressed(button_t button);