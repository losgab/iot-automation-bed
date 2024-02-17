#include "Button.h"

void update_button(button_t *button)
{
    uint8_t reading = !gpio_get_level(button->pin);
    button->pushed = (!button->last_state && reading);
    button->last_state = reading;
}

bool was_pushed(button_t *button)
{
    return button->pushed;
}

esp_err_t create_button(uint8_t pin, uint8_t is_input_pullup, button_t *ret_button)
{
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    if (is_input_pullup)
        gpio_pullup_en(pin);

    ret_button->pin = pin;
    ret_button->last_state = 0;
    ret_button->pushed = 0;

    ret_button->update_button = update_button;
    ret_button->was_pushed = was_pushed;
    return ESP_OK;
}
