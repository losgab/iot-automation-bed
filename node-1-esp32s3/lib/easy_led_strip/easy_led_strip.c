#include "easy_led_strip.h"

const uint8_t palette[6][3] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {255, 255, 0},
    {0, 255, 255},
    {255, 0, 255},
};

void led_strip_set_colour(led_strip_handle_t strip, uint8_t num_leds, const uint8_t colour[])
{
    for (uint8_t a = 0; a < num_leds; a++)
    {
        // printf("Setting LED %d to %d, %d, %d\n", a, colour[0], colour[1], colour[2]);
        led_strip_set_pixel(strip, a, colour[0], colour[1], colour[2]);
    }
    led_strip_refresh(strip);
}
