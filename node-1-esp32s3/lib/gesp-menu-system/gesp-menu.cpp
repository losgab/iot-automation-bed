#include "gesp-system.h"
#include "string.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    Menu::Menu(i2c_master_bus_handle_t bus, button_handle_t button_handles[])
    {
        gesp_ssd1306_init(bus, &display);

        memcpy(buttons, button_handles, MAX_NUM_BUTTONS * sizeof(button_handle_t));

        curr_program = 0; // Change these ater
        program_count = 3;

        cursor_pos = 2; // Initial Cursor position

        display.clear_display(&display);
        display.print_text_on_line(&display, "Gabe's System", LINE_0);
        display.print_text_on_line(&display, ">1.Program 1", LINE_2);
        display.print_text_on_line(&display, " 2.Program 2", LINE_3);
        display.print_text_on_line(&display, " 3.Program 3", LINE_4);
    }

    esp_err_t Menu::add_program(const char *program_name, void (*program_main)(void *))
    {
        size_t length = strlen(program_name);
        if (length + 1 > MAX_PROGRAM_NAME_LEN)
        {
            ESP_LOGE(MENU_TAG, "Program name too long");
            return ESP_ERR_INVALID_ARG;
        }
        if (program_count == MAX_NUM_PROGRAMS)
        {
            ESP_LOGE(MENU_TAG, "Max number of programs reached.");
            return ESP_ERR_INVALID_ARG;
        }

        programs[program_count].program_id = program_count;
        programs[program_count].program_task = 0;
        strncpy(programs[program_count].program_name, program_name, length);
        // programs[program_count].program_name = program_name;
        programs[program_count].program_main = program_main; // Program Main Function
        program_count++;

        return ESP_OK;
    }

    void Menu::cursor_down()
    {
        display.print_8x8basic(&display, ' ', (line_num_t)cursor_pos, 0);
        cursor_pos = (cursor_pos == program_count + 1) ? 2 : cursor_pos + 1;
        display.print_8x8basic(&display, '>', (line_num_t)cursor_pos, 0);
    }

    void Menu::cursor_up()
    {
        display.print_8x8basic(&display, ' ', (line_num_t)cursor_pos, 0);
        cursor_pos = (cursor_pos == 2) ? program_count + 1 : cursor_pos - 1;
        display.print_8x8basic(&display, '>', (line_num_t)cursor_pos, 0);
    }

    void Menu::program_select()
    {
        if (curr_program == cursor_pos - 1)
        {
            display.print_8x8basic(&display, ' ', (line_num_t)cursor_pos, 120);
            curr_program = 0;
            // Switch Menu IO context to program
            deregister_menu_buttons(buttons);
            // Unregister buttons and start associated init function of task
            // unregister_menu_buttons();
            // Start associated init function of task
        }
        else
        {
            display.print_8x8basic(&display, '*', (line_num_t)cursor_pos, 120);
            curr_program = cursor_pos - 1;
            // End associated init function of task
        }
    }

    void Menu::program_end()
    {
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

    esp_err_t deregister_menu_buttons(button_handle_t buttons[])
    {
        iot_button_unregister_cb(buttons[0], BUTTON_PRESS_DOWN);
        iot_button_unregister_cb(buttons[1], BUTTON_PRESS_DOWN);
        iot_button_unregister_cb(buttons[2], BUTTON_PRESS_DOWN);
        iot_button_unregister_cb(buttons[3], BUTTON_PRESS_DOWN);
        return ESP_OK;
    }

#ifdef __cplusplus
}
#endif
