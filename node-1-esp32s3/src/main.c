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
#include "gled_strip.h"
#define STRIP_1_PIN GPIO_NUM_42
#define STRIP_1_NUM_LEDS 2
led_strip_handle_t strip1;

// #include "Button.h"

// I2C Configuration
#define I2C_1_MASTER_SCL GPIO_NUM_13 // I2C 0 (Left Side)
#define I2C_1_MASTER_SDA GPIO_NUM_14
#define I2C_2_MASTER_SCL GPIO_NUM_9 // I2C 1 (Right Side)
#define I2C_2_MASTER_SDA GPIO_NUM_10

// SSD1306 Display Setup
#include "esp-ssd1306.h"
#define SSD1306_I2C_PORT I2C_NUM_0
ssd1306_t display;

// Servo Setup
// #include "iot_servo.h"

// Dev Board Button Setup
#include "iot_button.h"
#define BUTTON_1 GPIO_NUM_36
#define BUTTON_2 GPIO_NUM_35
#define BUTTON_3 GPIO_NUM_34
#define BUTTON_4 GPIO_NUM_33
#define BUTTON_TAG "Button Tag"

#include "esp32_fdc1004_lls.h"

static void b1_ecb(void *arg, void *data)
{
    led_strip_set_colour(strip1, STRIP_1_NUM_LEDS, RED);
}

static void b2_ecb(void *arg, void *data)
{
    led_strip_set_colour(strip1, STRIP_1_NUM_LEDS, GREEN);
}

static void b3_ecb(void *arg, void *data)
{
    led_strip_set_colour(strip1, STRIP_1_NUM_LEDS, AQUA);
}

static void b4_ecb(void *arg, void *data)
{
    led_strip_set_colour(strip1, STRIP_1_NUM_LEDS, MAGENTA);
}

void button_init()
{
    button_config_t gpio_btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
        .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
        .gpio_button_config = {
            .gpio_num = 0,
            .active_level = 0,
        },
    };

    gpio_btn_cfg.gpio_button_config.gpio_num = BUTTON_1;
    button_handle_t button1 = iot_button_create(&gpio_btn_cfg);
    gpio_btn_cfg.gpio_button_config.gpio_num = BUTTON_2;
    button_handle_t button2 = iot_button_create(&gpio_btn_cfg);
    gpio_btn_cfg.gpio_button_config.gpio_num = BUTTON_3;
    button_handle_t button3 = iot_button_create(&gpio_btn_cfg);
    gpio_btn_cfg.gpio_button_config.gpio_num = BUTTON_4;
    button_handle_t button4 = iot_button_create(&gpio_btn_cfg);
    if (NULL == button1)
        ESP_LOGE(BUTTON_TAG, "Button 1 create failed");
    if (NULL == button2)
        ESP_LOGE(BUTTON_TAG, "Button 2 create failed");
    if (NULL == button3)
        ESP_LOGE(BUTTON_TAG, "Button 3 create failed");
    if (NULL == button4)
        ESP_LOGE(BUTTON_TAG, "Button 4 create failed");
    iot_button_register_cb(button1, BUTTON_PRESS_DOWN, b1_ecb, NULL);
    iot_button_register_cb(button2, BUTTON_PRESS_DOWN, b2_ecb, NULL);
    iot_button_register_cb(button3, BUTTON_PRESS_DOWN, b3_ecb, NULL);
    iot_button_register_cb(button4, BUTTON_PRESS_DOWN, b4_ecb, NULL);
}

void app_main()
{
    // Initialise LED Strips
    ESP_ERROR_CHECK(create_led_strip_device(STRIP_1_PIN, STRIP_1_NUM_LEDS, &strip1));
    led_strip_set_colour(strip1, STRIP_1_NUM_LEDS, GREEN);
    SYS_DELAY(1000);
    led_strip_clear(strip1);

    // Initialise Buttons
    button_init();

    // I2C Setup
    i2c_init(I2C_MODE_MASTER, I2C_NUM_0, I2C_1_MASTER_SDA, I2C_1_MASTER_SCL);
    i2c_init(I2C_MODE_MASTER, I2C_NUM_1, I2C_2_MASTER_SDA, I2C_2_MASTER_SCL);
    gesp_ssd1306_init(SSD1306_I2C_PORT, &display);

    uint8_t data[2] = {0};
    while (1) {
        SYS_DELAY(1000);
        read_register(I2C_NUM_1, FDC_DEVICE_ID_REG, data);
        printf("Device ID: %X %X\n", data[0], data[1]);
    }
}