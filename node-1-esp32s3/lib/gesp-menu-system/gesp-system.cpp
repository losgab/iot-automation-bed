
#ifdef __cplusplus
extern "C"
{
#include "gesp-system.h"
#endif

    static void menu_button1_cb(void *arg, void *data)
    {
        Menu *menu = (Menu *)data;
        return menu->cursor_up();
    }

    static void menu_button2_cb(void *arg, void *data)
    {
        Menu *menu = (Menu *)data;
        return menu->cursor_down();
    }

    static void menu_button3_cb(void *arg, void *data)
    {
        Menu *menu = (Menu *)data;
        return menu->program_select();
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

        register_menu_buttons(menu, params->button_handles);

        // Add programs to Menu here
        // menu.add_program();

        // program_t leds = {
        //     .program_id = 0,
        //     .program_task = NULL,
        //     .program_name = "LEDs",
        //     .program_start = NULL,
        //     .program_end = NULL,
        // };

        while (1)
        {
            // printf("bruh\n");
            vTaskDelay(100);
        }
    }

#ifdef __cplusplus
}
#endif