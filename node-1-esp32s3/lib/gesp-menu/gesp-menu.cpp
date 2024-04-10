#include "gesp-menu.h"
#include "string.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    Menu::Menu(SSD1306 &display) : display(display), program_count(0), programs{}
    {
        // Print on screen a gabelab welcome message
        display.clear_all();
        display.print_text_on_line("Welcome to Gabe's Menu", LINE_0);
    }

    esp_err_t Menu::add_program(const char *program_name)
    {
        size_t length = strlen(program_name);
        if (length > MAX_PROGRAM_NAME_LEN)
        {
            ESP_LOGE(MENU_TAG, "Program name too long");
            return ESP_ERR_INVALID_ARG;
        }
        if (program_count = MAX_NUM_PROGRAMS)
        {
            ESP_LOGE(MENU_TAG, "Max number of programs reached.");
            return ESP_ERR_INVALID_ARG;
        }

        program_t new_program;
        strncpy(new_program.program_name, program_name, length);
        programs[program_count++] = new_program;

        return ESP_OK;
    }

    esp_err_t Menu::cursor_down()
    {
        return ESP_OK;
    }

    esp_err_t Menu::cursor_up()
    {
        return ESP_OK;
    }

    esp_err_t Menu::program_select()
    {
        return ESP_OK;
    }

    esp_err_t Menu::program_end()
    {
        return ESP_OK;
    }

#ifdef __cplusplus
}
#endif
