/*
    Buttons for ESP32

    Convenience library for observing button state changes.
    - Detects when a button was pushed
    - Detects if a button is held down (not implemented yet)

    Written by Gabriel Thien 2023
*/
#include <driver/gpio.h>

typedef struct Button
{
    uint8_t pin;
    uint8_t last_state;
    uint8_t pushed;
    // uint8_t pressed;
} Button;

typedef Button* button_t;

/**
 * @brief Initialises a button attached to the specified pin
 * 
 * @param pin GPIO pin that button is attached to
 * @param is_input_pullup 1 -> INPUT_PULLUP, 0 for external
 * 
 * @return Pointer to struct malloc
*/
button_t create_button(uint8_t pin, uint8_t is_input_pullup);

/**
 * @brief Reads and updates the state of the button
 * 
 * @param button struct Button *
*/
void update_button(button_t button);

/**
 * @brief Returns whether button was pushed
 * 
 * @param button struct Button *
 * 
 * @return 1 if pushed, 0 if not. Works on pulses
*/
uint8_t was_pushed(button_t button);

/**
 * @brief Returns whether button is being held down
 * 
 * @param button struct Button *
 * 
 * @return 1 if held down, 0 if not.
*/
uint8_t is_pressed(button_t button);