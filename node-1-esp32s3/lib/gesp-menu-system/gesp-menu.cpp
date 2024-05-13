#include "gesp-system.h"
#include <string>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    Menu::Menu(ssd1306_t display1, button_handle_t button_handles[]) : display(display1)
    {
        // gesp_ssd1306_init(bus, &display);
        register_menu_buttons(*this, button_handles);

        memcpy(buttons, button_handles, MAX_NUM_BUTTONS * sizeof(button_handle_t));

        curr_program = 0; // Change these ater

        cursor_pos = 2; // Initial Cursor position

        display.clear_display(&display);
        display.print_text_on_line(&display, "Gabe's System", LINE_0);
        display.print_8x8basic(&display, '*', LINE_0, 120);
        display.print_text_on_line(&display, ">1.Program 1", LINE_2);
        display.print_text_on_line(&display, " 2.Program 2", LINE_3);
        display.print_text_on_line(&display, " 3.Program 3", LINE_4);
        program_count = 3;
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

        programs[program_count].handle = NULL;
        strncpy(programs[program_count].program_name, program_name, length);
        // programs[program_count].program_name = program_name;
        programs[program_count].program_main = program_main; // Program Main Function
        program_count++;

        std::string line_text = " " + std::to_string(program_count) + "." + programs[program_count - 1].program_name;

        char arr[MAX_CHARACTERS_PER_LINE];
        strcpy(arr, line_text.c_str());

        display.print_text_on_line(&display, arr, program_count + 1);

        return ESP_OK;
    }

    void Menu::cursor_down()
    {
        display.print_8x8basic(&display, ' ', cursor_pos, 0);
        cursor_pos = (cursor_pos == program_count + 1) ? 2 : cursor_pos + 1;
        display.print_8x8basic(&display, '>', cursor_pos, 0);
    }

    void Menu::cursor_up()
    {
        display.print_8x8basic(&display, ' ', cursor_pos, 0);
        cursor_pos = (cursor_pos == 2) ? program_count + 1 : cursor_pos - 1;
        display.print_8x8basic(&display, '>', cursor_pos, 0);
    }

    void Menu::program_select()
    {
        if (curr_program == cursor_pos - 1) // End program if program already running
        {
            display.print_8x8basic(&display, ' ', cursor_pos, 120);
            // Add program id to task tracker

            vTaskDelete(programs[curr_program].handle);
            programs[curr_program].handle = NULL;
            register_menu_buttons(*this, buttons);
            curr_program = 0;

            // Unregister buttons and start associated init function of task
            // End associated init function of task
        }
        else // Start the program
        {
            display.print_8x8basic(&display, ' ', LINE_0, 120);
            display.print_8x8basic(&display, '*', cursor_pos, 120);
            curr_program = cursor_pos - 2;
            printf("Starting program %d\n", cursor_pos + 1);
            
            deregister_buttons();

            TaskHandle_t handle;
            xTaskCreate(programs[curr_program].program_main, programs[curr_program].program_name, 4096, &buttons, 1, &handle);
            programs[curr_program].handle = handle;
        }
    }

    void Menu::program_end()
    {
        if (curr_program == 0) // Menu is already running
        {
            ESP_LOGE(MENU_TAG, "Menu is already running! Cannot end Menu.");
            return;
        }

        deregister_buttons();
        vTaskDelete(programs[curr_program].handle);
        display.print_8x8basic(&display, ' ', curr_program + 2, 120);
        curr_program = 0; // Back to Menu
        display.print_8x8basic(&display, '*', LINE_0, 120);
        register_menu_buttons(*this, buttons);
    }

    void Menu::deregister_buttons()
    {
        iot_button_unregister_cb(buttons[0], BUTTON_PRESS_DOWN);
        iot_button_unregister_cb(buttons[1], BUTTON_PRESS_DOWN);
        iot_button_unregister_cb(buttons[2], BUTTON_PRESS_DOWN);
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
        Menu *menu = (Menu *)data;
        menu->program_end();
    }

    void register_menu_buttons(Menu &menu, button_handle_t buttons[])
    {
        iot_button_register_cb(buttons[0], BUTTON_PRESS_DOWN, menu_button1_cb, &menu);
        iot_button_register_cb(buttons[1], BUTTON_PRESS_DOWN, menu_button2_cb, &menu);
        iot_button_register_cb(buttons[2], BUTTON_PRESS_DOWN, menu_button3_cb, &menu);
        printf("Number of callbacks: %d\n", iot_button_count_cb(buttons[3]));
        if (iot_button_count_cb(buttons[3]) == 0)
            iot_button_register_cb(buttons[3], BUTTON_PRESS_DOWN, menu_button4_cb, &menu);
    }

#ifdef __cplusplus
}
#endif
