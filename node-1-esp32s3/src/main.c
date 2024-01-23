#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
// #include <driver/uart.h>
#include <driver/i2c.h>
#include <driver/gpio.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_lcd_types.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include <lvgl.h>

#define SYS_DELAY(x) vTaskDelay(pdMS_TO_TICKS(x))
// #include "servo.h"

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
#define I2C_1_MASTER_SCL GPIO_NUM_13 // I2C 0 (Right Side)
#define I2C_1_MASTER_SDA GPIO_NUM_14
#define I2C_2_MASTER_SCL GPIO_NUM_9 // I2C 1 (Left Side)
#define I2C_2_MASTER_SDA GPIO_NUM_10
i2c_config_t i2c0_config = {
    .mode = I2C_MODE_MASTER,
    .sda_io_num = I2C_1_MASTER_SDA,
    .scl_io_num = I2C_1_MASTER_SCL,
    .sda_pullup_en = GPIO_PULLUP_ENABLE,
    .scl_pullup_en = GPIO_PULLUP_ENABLE,
    .master.clk_speed = I2C_SCLK_SRC_FLAG_FOR_NOMAL};
// i2c_config_t i2c1_config = {
//     .mode = I2C_MODE_MASTER,
//     .sda_io_num = I2C_2_MASTER_SDA,
//     .scl_io_num = I2C_2_MASTER_SCL,
//     .sda_pullup_en = GPIO_PULLUP_ENABLE,
//     .scl_pullup_en = GPIO_PULLUP_ENABLE,
//     .master.clk_speed = I2C_SCLK_SRC_FLAG_FOR_NOMAL
// };

// SSD1306 Display Setup
#define SSD1306_HW_ADDR 0x3C
#define SSD1306_CMD_BITS 8
#define SSD1306_PARAM_BITS 8
// LCD Setup
esp_lcd_panel_io_handle_t io_handle = NULL;
esp_lcd_panel_io_i2c_config_t io_config = {
    .dev_addr = SSD1306_HW_ADDR,
    .control_phase_bytes = 1, // From SSD1306 datasheet
    .dc_bit_offset = 6,       // From SSD1306 datasheet
    .lcd_cmd_bits = SSD1306_CMD_BITS,
    .lcd_param_bits = SSD1306_PARAM_BITS,
};
// Display Setup
esp_lcd_panel_handle_t panel_handle = NULL;
esp_lcd_panel_dev_config_t panel_config = {
    .bits_per_pixel = 1,
    .reset_gpio_num = -1,
};

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

    // Initialise I2C Channel 0 & 1
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c0_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    // ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_1, &i2c1_config));
    // ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_1, I2C_MODE_MASTER, 0, 0, 0));

    // Initialise SSD1306 LHS Display
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_NUM_0, &io_config, &io_handle)); // Attaches LCD panel configuration to I2C for LHS
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));                    // Creates new SSD1306 panel to use
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    while (1)
    {
        update_button(button1);
        update_button(button2);
        update_button(button3);
        update_button(button4);
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