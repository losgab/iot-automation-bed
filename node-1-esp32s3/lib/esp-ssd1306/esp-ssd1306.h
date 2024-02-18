/**
 * SSD1306 Driver Library for displaying text through ESP-IDF
 *
 * Transmits commands to SSD1306 controller based from datasheet
 * Includes UTF8x8 font support
 *
 * Goals
 * - Provide support for 128x64 or 128x32 OLED displays
 * - Provide support for both I2C and SPI controlled OLED displays
 *
 * @author Gabriel Thien (https://github.com/losgab)
 * Inspiration from yanbe (https://github.com/yanbe/ssd1306-esp-idf-i2c/tree/master)
 */
#pragma once

#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_check.h"
#include "string.h"

#ifndef MAIN_SSD1366_H_
#define MAIN_SSD1366_H_

// GSSD1306 defines
#define SSD1306_TAG "SSD1306"
#define MAX_CHARACTERS_PER_LINE 16

#define MAX_PAGES 8

// Following definitions are bollowed from
// http://robotcantalk.blogspot.com/2015/03/interfacing-arduino-with-ssd1306-driven.html

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

typedef struct ssd1306_display
{
    i2c_port_t port;

    /**
     * @brief Clear entire display. Zeroes all memory.
     *
     * @param self Pointer to ssd1306_t struct
     *
     * @return void
     */
    void (*clear_display)(struct ssd1306_display *device);

    /**
     * @brief Clears a single line on the display.
     *
     * @param self Pointer to ssd1306_t struct
     * @param line Line to clear (0 - MAX_ROWS)
     *
     * @return void
     */
    void (*clear_line)(struct ssd1306_display *device, line_num_t line);

    /**
     * @brief Writes text onto specified line on the display.
     * 
     * @param self Pointer to ssd1306_t struct
     * @param text Text to display
     * @param line Line to display text on (0 - MAX_ROWS)
     * 
     * @return void, needs to return invalid 
    */
    esp_err_t (*print_text_on_line)(struct ssd1306_display *device, const char *text, line_num_t line);

} ssd1306_t;

/**
 * @brief Initializes SSD1306 display. Gabriel's library for the ESP-IDF through I2C.
 * 
 * @param port I2C port to use
 * @param ret_ssd1306_device Pointer to ssd1306_t struct
 * 
 * @return esp_err_t ESP_OK if successful, otherwise error code
*/
esp_err_t gesp_ssd1306_init(i2c_port_t port, ssd1306_t *ret_ssd1306_device);

/**
 * @brief Initialize SSD1306 display
 *
 * @return void
 */
void ssd1306_init();

/**
 * @brief Display text on SSD1306 display
 *
 * @param pvParameters from TaskCreate
 *
 * @return void
 */

void task_ssd1306_display_pattern(void *ignore);

void task_ssd1306_contrast(void *ignore);

void task_ssd1306_scroll(void *ignore);

#endif /* MAIN_SSD1366_H_ */