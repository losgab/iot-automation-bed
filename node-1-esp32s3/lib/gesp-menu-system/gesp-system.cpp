#ifdef __cplusplus
extern "C"
{
#endif

#include "gesp-system.h"

    void menu_main(void *pvParameter)
    {
        menu_peripherals_t *params = (menu_peripherals_t *)pvParameter;
        // i2c_master_bus_handle_t master_bus = params->master_handle;úú

        Menu menu = Menu(params->display, params->button_handles);

        // register_menu_buttons(menu, params->button_handles);

        // Add programs to Menu here
        menu.add_program("LEDs", led_strip_main); // LED  Strip Program
                                                  // Add Servo Control Program
                                                  // Add Stepper Control Program
                                                  // Add FDC1004 program

        while (1) // Button context switching
        {
            // If queue is not empty, process the button context switch
            vTaskDelay(10);
        }
    }

#ifdef __cplusplus
}
#endif