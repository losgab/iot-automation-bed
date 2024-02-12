#include "gled_strip.h"

esp_err_t gled_strip_new(gled_strip_t *strip, gpio_num_t pin, uint16_t num_leds)
{
    strip = malloc(sizeof(gled_strip_t));
    ESP_RETURN_ON_FALSE(strip != NULL, ESP_ERR_NO_MEM, GLED_STRIP_TAG, "No Memory left for LED Strip");

    strip->pin = pin;
    strip->format = LED_PIXEL_FORMAT_GRB;

    ESP_RETURN_ON_ERROR(gled_strip_new_rmt_device(strip->rmt_device, pin, num_leds), GLED_STRIP_TAG, "Create RMT device failed");
    ESP_RETURN_ON_ERROR(gled_strip_new_rmt_interface(strip->interface), GLED_STRIP_TAG, "Link RMT interface failed");
    return ESP_OK;
}

esp_err_t gled_strip_set_colour(gled_strip_t *strip, colour_t colour)
{
    for (uint8_t a = 0; a < strip->rmt_device->num_leds; a++)
    {
        ESP_RETURN_ON_ERROR(strip->interface->set_pixel(strip->rmt_device, a, palette[colour][0], palette[colour][1], palette[colour][2]), GLED_STRIP_TAG, "Set Colour failed");
    }
    return strip->interface->refresh(strip->rmt_device);
}

esp_err_t gled_strip_set_pixel(gled_strip_t *strip, uint16_t pixel, colour_t colour)
{
    uint8_t red = palette[colour][0];
    uint8_t green = palette[colour][1];
    uint8_t blue = palette[colour][2];
    return strip->interface->set_pixel(strip->rmt_device, pixel, red, green, blue);
}

esp_err_t gled_strip_refresh(gled_strip_t *strip)
{
    return strip->interface->refresh(strip->rmt_device);
}

esp_err_t gled_strip_clear(gled_strip_t *strip)
{
    return strip->interface->clear(strip->rmt_device);
}
