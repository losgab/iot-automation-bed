#include "Button.h"

button_t create_button(uint8_t pin, uint8_t is_input_pullup)
{
    button_t new_button = malloc(sizeof(Button));

    gpio_set_direction(pin, GPIO_MODE_INPUT);
    if (is_input_pullup)
        gpio_pullup_en(pin);

    new_button->pin = pin;
    new_button->last_state = 0;
    new_button->pushed = 0;

    return new_button;
}

void update_button(button_t button)
{
    uint8_t reading = !gpio_get_level(button->pin);
    button->pushed = (!button->last_state && reading);
    button->last_state = reading;
}

uint8_t was_pushed(button_t button)
{
    return button->pushed;
}