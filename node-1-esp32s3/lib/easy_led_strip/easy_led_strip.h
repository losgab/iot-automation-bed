#pragma once

#include <led_strip.h>

enum colour // Colours
{
    RED,     // 255, 0, 0
    GREEN,   // 0, 255, 0
    BLUE,    // 0, 0, 255
    YELLOW,  // 255, 255, 0
    AQUA,    // 0, 255, 255
    MAGENTA, // 255, 0, 255
};

extern const uint8_t palette[6][3];

/**
 * @brief Convenience function for setting colour of the LED strip
 *
 * @param strip LED strip handle
 * @param num_leds Number of LEDs
 * @param colour Colour of the LEDs from enumeration
 *
 * @return void
 */
void led_strip_set_colour(led_strip_handle_t strip, uint8_t num_leds, const uint8_t colour[]);

/* ---EXAMPLE THREE CONFIGURATIONS---

led_strip_handle_t strip;
led_strip_config_t strip_config = {
    .strip_gpio_num = LED_PIN,
    .max_leds = NUM_LEDS,
    .led_pixel_format = LED_PIXEL_FORMAT_GRB,
    .led_model = LED_MODEL_WS2812,
};
led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    .rmt_channel = 0,
#else
    .clk_src = RMT_CLK_SRC_DEFAULT,    // different clock source can lead to different power consumption
    .resolution_hz = 10 * 1000 * 1000, // 10MHz
    .flags.with_dma = false,           // whether to enable the DMA feature
#endif
};

*/