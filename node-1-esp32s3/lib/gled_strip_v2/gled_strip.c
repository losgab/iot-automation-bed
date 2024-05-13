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

static void button1_cb(void *arg, void *data)
{
    led_strip_handle_t strip = *(led_strip_handle_t *)data;
    led_strip_set_colour(strip, NUM_LEDS, RED);
}

static void button2_cb(void *arg, void *data)
{
    led_strip_handle_t strip = *(led_strip_handle_t *)data;
    led_strip_set_colour(strip, NUM_LEDS, GREEN);
}

static void button3_cb(void *arg, void *data)
{
    led_strip_handle_t strip = *(led_strip_handle_t *)data;
    led_strip_set_colour(strip, NUM_LEDS, BLUE);
}

static void button4_cb(void *arg, void *data)
{
    led_strip_handle_t strip = *(led_strip_handle_t *)data;
    led_strip_clear(strip);
    printf("LED Strip demo stopped. Resources freed.\n");
    led_strip_del(strip);
}

void led_strip_main(void *pvParameter)
{
    button_handle_t *buttons = (button_handle_t *)pvParameter;

    // Initialise LED Strips
    led_strip_handle_t strip1;
    ESP_ERROR_CHECK(create_led_strip_device(GPIO_NUM_42, 2, &strip1));
    led_strip_clear(strip1);

    iot_button_register_cb(buttons[0], BUTTON_PRESS_DOWN, button1_cb, &strip1);
    iot_button_register_cb(buttons[1], BUTTON_PRESS_DOWN, button2_cb, &strip1);
    iot_button_register_cb(buttons[2], BUTTON_PRESS_DOWN, button3_cb, &strip1);
    iot_button_register_cb(buttons[3], BUTTON_PRESS_DOWN, button4_cb, &strip1);

    printf("LED Strip demo started\n");

    while (1)
    {
        vTaskDelay(10);
    }
}
