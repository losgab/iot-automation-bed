#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
// #include <driver/uart.h>
#include <driver/i2c.h>
#include <driver/gpio.h>

#include "esp_err.h"
#include "esp_log.h"

#include "communication.h"

#define SYS_DELAY(x) vTaskDelay(pdMS_TO_TICKS(x))

// LED Strip Configuration
#include <led_strip.h>
#include "easy_led_strip.h"
#define STRIP_1_PIN GPIO_NUM_41
#define STRIP_1_NUM_LEDS 1
#define STRIP_2_PIN GPIO_NUM_42
#define STRIP_2_NUM_LEDS 2
led_strip_handle_t strip1;
led_strip_handle_t strip2;
led_strip_config_t strip_config1 = {
    .strip_gpio_num = STRIP_1_PIN,
    .max_leds = STRIP_1_NUM_LEDS,
    .led_pixel_format = LED_PIXEL_FORMAT_GRB,
    .led_model = LED_MODEL_WS2812,
};
led_strip_config_t strip_config2 = {
    .strip_gpio_num = STRIP_2_PIN,
    .max_leds = STRIP_2_NUM_LEDS,
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

// Dev Board Button Setup
#include "Button.h"
#define BUTTON_1 GPIO_NUM_36
#define BUTTON_2 GPIO_NUM_35
#define BUTTON_3 GPIO_NUM_34
#define BUTTON_4 GPIO_NUM_33
button_t button1, button2, button3, button4;

// I2C Configuration
#define I2C_1_MASTER_SCL GPIO_NUM_13 // I2C 0 (Left Side)
#define I2C_1_MASTER_SDA GPIO_NUM_14
#define I2C_2_MASTER_SCL GPIO_NUM_9 // I2C 1 (Right Side)
#define I2C_2_MASTER_SDA GPIO_NUM_10

// SSD1306 Display Setup
#include "esp-ssd1306.h"
#define SSD1306_CMD_BITS 8
#define SSD1306_PARAM_BITS 8

void app_main()
{
    // Initialise LED Strips
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config1, &rmt_config, &strip1));
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config2, &rmt_config, &strip2));
    led_strip_clear(strip1);
    led_strip_clear(strip2);

    // Initialise Buttons
    button1 = create_button(BUTTON_1, true);
    button2 = create_button(BUTTON_2, true);
    button3 = create_button(BUTTON_3, true);
    button4 = create_button(BUTTON_4, true);

    i2c_init(I2C_MODE_MASTER, I2C_NUM_0, I2C_1_MASTER_SDA, I2C_1_MASTER_SCL);

    ssd1306_init();
    // xTaskCreate(&task_ssd1306_display_clear, "ssd1306_display_clear",  2048, NULL, 6, NULL);
    xTaskCreate(&task_ssd1306_display_text, "task_ssd1306_display_text", 4096, NULL, 5, NULL);

    // while (1)
    // {
    //     update_button(button1);
    //     update_button(button2);
    //     update_button(button3);
    //     update_button(button4);
    //     SYS_DELAY(100);
    // }
}