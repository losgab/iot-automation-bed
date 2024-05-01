
#ifdef __cplusplus
extern "C"
{
#endif

#include "gesp-system.h"
}

static void menu_button1_cb(void *arg, void *data)
{
    Menu *menu = (Menu *)data;
    menu->cursor_up();
}

static void menu_button2_cb(void *arg, void *data)
{
    Menu *menu = (Menu *)data;
    menu->cursor_down();
}

static void menu_button3_cb(void *arg, void *data)
{
    Menu *menu = (Menu *)data;
    menu->program_select();
}

static void menu_button4_cb(void *arg, void *data)
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
    menu_task_params_t *params = (menu_task_params_t *)pvParameter;
    i2c_master_bus_handle_t master_bus = params->master_handle;

    Menu menu(master_bus);

    button_handles[0] = params->button_handles[0];
    button_handles[1] = params->button_handles[1];
    button_handles[2] = params->button_handles[2];
    button_handles[3] = params->button_handles[3];
    register_menu_buttons(menu, button_handles);

    // Add programs to Menu here
    menu.add_program("LEDs", led_strip_main);
    // Add Servo Control Program
    // Add Stepper Control Program
    // Add FDC1004 program

    while (1)
    {
        vTaskDelay(100);
    }
}

#ifdef __cplusplus
}
#endif