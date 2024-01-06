#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
// #include <driver/uart.h>
// #include <driver/i2c.h>
// #include <driver/gpio.h>

// #include "servo.h"

#define SYS_DELAY(x) vTaskDelay(pdMS_TO_TICKS(x))

#define SERVO_1_GPIO GPIO_NUM_6
#define SERVO_1_PWM_CHANNEL LEDC_CHANNEL_0

// servo_t servo_1 = {
//     .gpio_num = SERVO_1_GPIO,
//     .speed_mode = LEDC_LOW_SPEED_MODE,
//     .channel = SERVO_1_PWM_CHANNEL,
//     .intr_type = LEDC_INTR_DISABLE,
//     .duty = 0,
//     .hpoint = 0};

// LED Strip Configuration
#include <led_strip.h>
#include "easy_led_strip.h"
#define LED_PIN GPIO_NUM_42
#define NUM_LEDS 2
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

// Button Setup
#include "Button.h"
#define BUTTON_0 GPIO_NUM_38
#define BUTTON_1 GPIO_NUM_39
#define BUTTON_2 GPIO_NUM_40
button_t button_0, button_1, button_2;


void app_main()
{
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &strip));
    led_strip_clear(strip);
    
    button_0 = create_button(BUTTON_0, true);
    button_1 = create_button(BUTTON_1, true);
    button_2 = create_button(BUTTON_2, true);

    // servo_init(&servo_1);
    while (1)
    {
        update_button(button_0);
        update_button(button_1);
        update_button(button_2);
        if (was_pushed(button_0))
            led_strip_set_colour(strip, NUM_LEDS, palette[RED]);
        if (was_pushed(button_1))
            led_strip_set_colour(strip, NUM_LEDS, palette[GREEN]);
        if (was_pushed(button_2))
            led_strip_set_colour(strip, NUM_LEDS, palette[BLUE]);
        SYS_DELAY(100);
        
        // Sweep servo from 0 to 180 degrees
        // for (int angle = 0; angle <= 180; angle++)
        // {
        //     servo_set(SERVO_1_PWM_CHANNEL, angle);
        //     SYS_DELAY(10);
        // }
        // SYS_DELAY(200);
    }
}