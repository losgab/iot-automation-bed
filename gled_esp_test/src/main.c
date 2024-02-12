#include <freertos/FreeRTOS.h>
// #include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <driver/gpio.h>

#define SYS_DELAY(x) vTaskDelay(pdMS_TO_TICKS(x))

// LED Strip Configuration
#include "gled_strip.h"
#define NUM_LEDS 2
#define LED_STRIP_PIN GPIO_NUM_42


void app_main(void)
{
    // Initialise LED Strip
    gled_strip_t strip;
    ESP_ERROR_CHECK(gled_strip_new(&strip, NUM_LEDS, LED_STRIP_PIN));

    gled_strip_set_colour(&strip, RED);
}