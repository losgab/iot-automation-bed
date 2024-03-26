#pragma once
/**
 * @file gesp-ssd1306.h
 *
 * Gabriel's OOP Library for the ESP-IDF SSD1306 OLED Display Driver
 *
 * Goals:
 * - Provide a simple line by line ASCII printing interface for the SSD1306 OLED Display Driver
 *   through the I2C and SPI protocols
 * - Provide a FreeRTOS task to handle updating the OLED display
 */

#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_check.h"
#include "string.h"

#define SSD1306_TAG "SSD1306"
#define MAX_CHARACTERS_PER_LINE 16

#define MAX_PAGES 8

// SLA (0x3C) + WRITE_MODE (0x00) =  0x78 (0b01111000)
#define OLED_I2C_ADDRESS 0x3C

// Control byte (Co bit + D/C# bit + 6 zero bits)
#define OLED_CONTROL_BYTE_CMD_SINGLE 0x80
#define OLED_CONTROL_BYTE_CMD_STREAM 0x00
#define OLED_CONTROL_BYTE_DATA_STREAM 0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST 0x81 // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM 0xA4
#define OLED_CMD_DISPLAY_ALLON 0xA5
#define OLED_CMD_DISPLAY_NORMAL 0xA6
#define OLED_CMD_DISPLAY_INVERTED 0xA7
#define OLED_CMD_DISPLAY_OFF 0xAE
#define OLED_CMD_DISPLAY_ON 0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE 0x20 // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE 0x21     // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE 0x22       // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP 0xA1
#define OLED_CMD_SET_MUX_RATIO 0xA8 // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE 0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET 0xD3 // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP 0xDA    // follow with 0x12
#define OLED_CMD_NOP 0xE3                // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV 0xD5 // follow with 0x80
#define OLED_CMD_SET_PRECHARGE 0xD9       // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT 0xDB   // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP 0x8D // follow with 0x14

typedef enum {
    LINE_0,
    LINE_1,
    LINE_2,
    LINE_3,
    LINE_4,
    LINE_5,
    LINE_6,
    LINE_7,
    MAX_LINES
} line_num_t;

namespace gesp_ssd1306
{
    class SSD1306
    {
    public:
        /**
         * @brief Initialises a new SSD1306 display instance.
         */
        SSD1306(i2c_port_t port);

        /**
         * @brief Prints string to specified line on display.
        */
        esp_err_t print_text_on_line(const char *text, line_num_t line);

        /**
         * @brief Clears entire screen.
        */
       esp_err_t clear_all();

        /**
         * @brief Clears entire screen.
        */
       esp_err_t clear_line(line_num_t line);

        /**
         * @brief Destructor
        */
        ~SSD1306();
    };
}