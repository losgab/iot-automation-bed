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
#define SSD1306_I2C_PORT I2C_NUM_0
ssd1306_t display;

// Servo Setup
// #include "iot_servo.h"

void app_main()
{
    // Initialise LED Strips
    ESP_ERROR_CHECK(create_led_strip_device(STRIP_1_PIN, STRIP_1_NUM_LEDS, &strip1));
    led_strip_set_colour(strip1, STRIP_1_NUM_LEDS, RED);

    // Initialise Buttons
    create_button(BUTTON_1, true, &button1);
    create_button(BUTTON_2, true, &button2);
    create_button(BUTTON_3, true, &button3);
    create_button(BUTTON_4, true, &button4);

    i2c_init(I2C_MODE_MASTER, I2C_NUM_0, I2C_1_MASTER_SDA, I2C_1_MASTER_SCL);
    
    gesp_ssd1306_init(SSD1306_I2C_PORT, &display);
    // ssd1306_init();
    // xTaskCreate(&task_ssd1306_display_clear, "ssd1306_display_clear",  2048, NULL, 6, NULL);
    // xTaskCreate(&task_ssd1306_display_text, "task_ssd1306_display_text", 4096, NULL, 5, NULL);

    int line = 0;

    while (1)
    {
        button1.update_button(&button1);
        button2.update_button(&button2);
        button3.update_button(&button3);
        button4.update_button(&button4);
        SYS_DELAY(100);

        if (button1.was_pushed(&button1))
        {
            char *text = "Test line-------";
            display.print_text_on_line(&display, text, LINE_3);
            // led_strip_set_colour(strip1, STRIP_1_NUM_LEDS, RED);
        }
        else if (button2.was_pushed(&button2))
        {
        }
        else if (button3.was_pushed(&button3))
        {
            display.clear_line(&display, LINE_3);
        }
        else if (button4.was_pushed(&button4))
        {
            display.clear_display(&display);
            // xTaskCreate(&task_ssd1306_display_clear, "task_ssd1306_display_text", 4096, NULL, 5, NULL);
        }
    }
}