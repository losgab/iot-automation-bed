#include "gesp-menu-system.h"
#include "string.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void menu_main(struct ssd1306 *display)
    {
        // Main Menu Loop
        while (1)
        {
            // Display Menu
            display.print_text_on_line(display, "1. Program 1", LINE_1);
            display.print_text_on_line(display, "2. Program 2", LINE_2);
        }

        vTaskDelete();
    }

    TaskHandle_t menu_init()
    {
        TaskHandle_t handle;

        xTaskCreate(menu_main, "menu_main", 4096, NULL, 1, &handle);

        // Assign UI selection buttons
    }

    Menu::Menu(SSD1306 &display) : display(display), program_count(0), programs{}
    {
        display.clear_all();
        display.print_text_on_line("Welcome to Gabe's Menu", LINE_0);

        // Assign UI selection buttons

    }

    esp_err_t Menu::add_program(const char *program_name)
    {
        size_t length = strlen(program_name);
        if (length + 1 > MAX_PROGRAM_NAME_LEN)
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
        new_program.program_id = program_count;
        new_program.program_task = NULL;
        strncpy(new_program.program_name, program_name, length);
        programs[program_count++] = new_program;

        // Add pointers to program task



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
        // Start associated init function of task
        // Get task handle of running program
        // Add task handle to running programs earlier
        return ESP_OK;
    }

    esp_err_t Menu::program_end()
    {
        return ESP_OK;
    }

    void main()
    {
        
    }

#ifdef __cplusplus
}
#endif
