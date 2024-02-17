#include "gled_strip.h"

static uint8_t palette[MAX_COLOURS][CHANNELS] =
    {
        {255, 0, 0},
        {0, 255, 0},
        {0, 0, 255},
        {255, 255, 0},
        {0, 255, 255},
        {255, 0, 255},
};

void led_strip_set_colour(led_strip_handle_t strip, uint8_t num_leds, colour_t colour)
{
    for (uint8_t a = 0; a < num_leds; a++)
    {
        led_strip_set_pixel(strip, a, palette[colour][0], palette[colour][1], palette[colour][2]);
    }
    led_strip_refresh(strip);
}

esp_err_t create_led_strip_device(gpio_num_t pin, uint8_t num_leds, led_strip_handle_t *ret_strip)
{
    // RMT configurations
    led_strip_config_t strip_config = {
        .strip_gpio_num = pin,
        .max_leds = num_leds,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,    // different clock source can lead to different power consumption
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = true,            // whether to enable the DMA feature
    };

    led_strip_new_rmt_device(&strip_config, &rmt_config, ret_strip);
    led_strip_clear(*ret_strip);
    return ESP_OK;
}
