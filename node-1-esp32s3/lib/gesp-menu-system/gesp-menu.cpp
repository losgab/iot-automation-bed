#include "gesp-system.h"
#include "string.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    Menu::Menu(i2c_master_bus_handle_t bus)
    {
        gesp_ssd1306_init(bus, &display);

        curr_program = 0; // Change these ater
        program_count = 3;

        cursor_pos = 2; // Initial Cursor position

        display.clear_display(&display);
        display.print_text_on_line(&display, "Gabe's System", LINE_0);
        display.print_text_on_line(&display, ">1.Program 1", LINE_2);
        display.print_text_on_line(&display, " 2.Program 2", LINE_3);
        display.print_text_on_line(&display, " 3.Program 3", LINE_4);
    }

    esp_err_t Menu::add_program(const char *program_name)
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

        program_t new_program;
        new_program.program_id = program_count;
        new_program.program_task = NULL;
        strncpy(new_program.program_name, program_name, length);
        programs[program_count++] = new_program;

        // Add pointers to program task

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

#ifdef __cplusplus
}
#endif
