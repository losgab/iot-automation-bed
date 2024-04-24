#ifndef MENU_UI_H
#define MENU_UI_H

#include <driver/gpio.h>
// #include "gesp-ssd1306.h"
#include "esp-ssd1306.h"
#include "esp_err.h"
#include "esp_log.h"

#define MAX_NUM_PROGRAMS 4
#define MAX_PROGRAM_NAME_LEN 12

#define MENU_TAG "Gabe's Menu"

// Programs
typedef struct program
{
    uint8_t program_id;
    TaskHandle_t program_task;
    char program_name[MAX_PROGRAM_NAME_LEN];
    void (*program_start)(void);
    void (*program_end)(void);
} program_t;

// Built for SSD1306 screens
class Menu
{
public:
    /**
     * @brief Initialises a new MenuUI instance. Loads existing tasks from flash memory. Starts SSD1306 device.
     */
    Menu();

    /**
     * @brief Adds a program to the menu UI.
     */
    esp_err_t add_program(const char *program_name);

    /**
     * @brief Moves cursor to next program.
     */
    esp_err_t cursor_down();

    /**
     * @brief Moves cursor to previous program.
     */
    esp_err_t cursor_up();

    /**
     * @brief Selects current program & calls an initialisation function.
     */
    esp_err_t program_select();

    /**
     * @brief Selects current program & calls an end function.
     */
    esp_err_t program_end();

    /**
     * @brief Destroys MenuUI instance.
     */
    ~Menu();

private:
    SSD1306 display;
    uint8_t curr_programs;
    uint8_t program_count;
    BaseType_t suspended_tasks[MAX_NUM_PROGRAMS]{};
    program_t programs[MAX_NUM_PROGRAMS]{};
};
#endif