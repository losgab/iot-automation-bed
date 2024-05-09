    void register_menu_buttons()
    {
        iot_button_register_cb(buttons[0], BUTTON_PRESS_DOWN, menu_button1_cb, &menu);
        iot_button_register_cb(buttons[1], BUTTON_PRESS_DOWN, menu_button2_cb, &menu);
        iot_button_register_cb(buttons[2], BUTTON_PRESS_DOWN, menu_button3_cb, &menu);
        iot_button_register_cb(buttons[3], BUTTON_PRESS_DOWN, menu_button4_cb, &menu);
    }

    void deregister_menu_buttons()
    {
        iot_button_unregister_cb(buttons[0], BUTTON_PRESS_DOWN);
        iot_button_unregister_cb(buttons[1], BUTTON_PRESS_DOWN);
        iot_button_unregister_cb(buttons[2], BUTTON_PRESS_DOWN);
        iot_button_unregister_cb(buttons[3], BUTTON_PRESS_DOWN);
    }
