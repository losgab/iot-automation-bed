#ifndef MENU_UI_H
#define MENU_UI_H

#include <driver/gpio.h>
#include "gesp-ssd1306.h"

// Programs
typedef struct {
    uint8_t program_id;
    char program_name[16];
} program_t;

// Built for SSD1306 screens
class MenuUI
{
    public:
        /**
         * @brief Initialises a new MenuUI instance. Loads existing tasks from flash memory. Starts SSD1306 device.
        */
        MenuUI();

        /**
         * @brief Destroys MenuUI instance. 
        */
        ~MenuUI();
    private:
        
};
#endif