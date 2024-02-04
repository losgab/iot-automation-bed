/**
 * Mini Library for simplifying setting up common communication protocols 
 * for the ESP32 in ESP-IDF
 * 
 * Supports
 * - I2C
 * 
 * @author Gabriel Thien (https://github.com/losgab)
*/
#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <driver/uart.h>

/**
 * @brief Macro Function for shortcutting setting up I2C communication
 * 
 * @param mode I2C Mode, Master or Slave
 * @param port I2C Port number
 * @param sda GPIO number for SDA
 * @param scl GPIO number for SCL
 * 
 * @return esp_err_t ESP-IDF Error Code
*/
esp_err_t i2c_init(i2c_mode_t mode, i2c_port_t port, gpio_num_t sda, gpio_num_t scl);

// /**
//  * @brief Macro Function for shortcutting setting up UART communication
//  * 
//  * @param 
// */
// esp_err_t uart_init(uart_mode_t mode, uart_port_t port, int baudrate, int tx_pin, int rx_pin, int rts_pin, int cts_pin);