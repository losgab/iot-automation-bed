
#ifdef __cplusplus
extern "C" {
#include "gesp-system.h"
#endif

static void menu_button1_cb(void *button, void *data)
{
    Menu *menu = (Menu *)data;
    return menu->cursor_up();
}

static void menu_button2_cb(void *button, void *data)
{
    Menu *menu = (Menu *)data;
    return menu->cursor_down();
}

static void menu_button3_cb(void *button, void *data)
{
    Menu *menu = (Menu *)data;
    return menu->program_select();
}

static void menu_button4_cb(void *button, void *data)
{
    
}

esp_err_t register_menu_buttons(Menu &menu, button_handle_t buttons[])
{
    iot_button_register_cb(buttons[0], BUTTON_PRESS_DOWN, menu_button1_cb, &menu);
    iot_button_register_cb(buttons[1], BUTTON_PRESS_DOWN, menu_button2_cb, &menu);
    iot_button_register_cb(buttons[2], BUTTON_PRESS_DOWN, menu_button3_cb, &menu);
    iot_button_register_cb(buttons[3], BUTTON_PRESS_DOWN, menu_button4_cb, &menu);
    return ESP_OK;
}

esp_err_t unregister_menu_buttons(button_handle_t buttons[])
{
    iot_button_unregister_cb(buttons[0], BUTTON_PRESS_DOWN);
    iot_button_unregister_cb(buttons[1], BUTTON_PRESS_DOWN);
    iot_button_unregister_cb(buttons[2], BUTTON_PRESS_DOWN);
    iot_button_unregister_cb(buttons[3], BUTTON_PRESS_DOWN);
    return ESP_OK;
}

void menu_main(void *pvParameter)
{
    button_handle_t *buttons = (button_handle_t *)pvParameter;

    Menu menu();

    // Add programs to Menu here
    // menu.add_program();

    // program_t leds = {
    //     .program_id = 0,
    //     .program_task = NULL,
    //     .program_name = "LEDs",
    //     .program_start = NULL,
    //     .program_end = NULL,
    // };

    // Initialise button functionality here
    // register_menu_buttons(menu, buttons);
    if (buttons[0] == NULL) {
        printf("Buttons not found!\n");
    }
    // display.clear_display(&display);
    // display.print_text_on_line(&display, "Gabe's System", LINE_0);
    // display.print_text_on_line(&display, " 1.Program 1", LINE_2);
    // display.print_text_on_line(&display, " 2.Program 2", LINE_3);

    // Main Menu Loop
    while (1)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
    
TaskHandle_t menu_init(button_handle_t buttons[])
{
    TaskHandle_t handle = 0;
    xTaskCreate(menu_main, "menu_main", 4096, buttons, 1, &handle);
    return handle;
}

#ifdef __cplusplus
}
#endif