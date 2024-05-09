#ifndef MENU_UI_H
#define MENU_UI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <freertos/FreeRTOS.h>
// #include "freertos/task.h"
#include "driver/gpio.h"
#include <driver/i2c_master.h>
// #include "gesp-ssd1306.h"
#include "esp-ssd1306.h"
#include "iot_button.h"
#include "esp_err.h"
#include "esp_log.h"

#include "gled_strip.h"

// I2C Configuration
#define SSD1306_I2C_PORT I2C_NUM_0

#define MAX_NUM_PROGRAMS 4
#define MAX_PROGRAM_NAME_LEN 10
#define MAX_NUM_BUTTONS 4

#define MENU_TAG "Gabe's Menu"

// button_handle_t button_handles[4];

    // System Button Context Handler
    typedef struct button_context
    {
        uint8_t curr_program;
        // 0 for Main Menu, 1 -> MAX_NUM_PROGRAMS + 1 for programs
        // Function pointer for registering IO buttons
        // Function pointer for deregistering IO buttons
    } button_context_t;

    // Programs
    typedef struct program
    {
        uint8_t program_id;
        TaskHandle_t program_task;
        char program_name[MAX_PROGRAM_NAME_LEN];
        void (*program_main)(void *);
    } program_t;

    // Menu Task Peripheral Params
    typedef struct menu_task_params
    {
        i2c_master_bus_handle_t master_handle;
        button_handle_t button_handles[4];
    } menu_task_params_t;

    // Built for SSD1306 screens
    class Menu
    {
    public:
        /**
         * @brief Initialises a new MenuUI instance. Loads existing tasks from flash memory. Starts SSD1306 device.
         */
        Menu(i2c_master_bus_handle_t bus, button_handle_t buttons[]);

        /**
         * @brief Adds a program to the menu UI.
         */
        esp_err_t add_program(const char *program_name, void (*program_main)(void *));

        /**
         * @brief Moves cursor to next program.
         */
        void cursor_down();

        /**
         * @brief Moves cursor to previous program.
         */
        void cursor_up();

        /**
         * @brief Selects current program & calls an initialisation function.
         */
        void program_select();

        /**
         * @brief Selects current program & calls an end function.
         */
        void program_end();

        void register_menu_buttons();

        void deregister_menu_buttons(button_handle_t buttons[]);

        /**
         * @brief Destroys MenuUI instance.
         */
        ~Menu();

    private:
        ssd1306_t display;
        button_handle_t buttons[4];
        uint8_t curr_program;
        uint8_t program_count;
        uint8_t cursor_pos;
        TaskHandle_t suspended_tasks[MAX_NUM_PROGRAMS]{};
        program_t programs[MAX_NUM_PROGRAMS]{};
    };

    void menu_main(void *pvParameter);

#ifdef __cplusplus
}
#endif

#endif