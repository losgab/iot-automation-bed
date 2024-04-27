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

#include <string.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include "driver/i2c_master.h"
// #include <driver/uart.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#define BUFF_LEN 1024

/**
 * @brief Macro Function for shortcutting setting up I2C Master communication
 *
 * @param port I2C Port number
 * @param sda GPIO number for SDA
 * @param scl GPIO number for SCL
 * @param ret_handle Pointer to I2C Master Bus handle
 *
 * @return esp_err_t Error code
 */
esp_err_t i2c_master_init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl, i2c_master_bus_handle_t *ret_handle);

void i2c_clear_write_buffer();

void i2c_write_byte(const uint8_t byte);

void i2c_write_bytes(const uint8_t *bytes, const int len);

void i2c_write_zero(const int len);

esp_err_t i2c_transmit_write_buffer(i2c_master_dev_handle_t slave_handle);

// /**
//  * @brief Macro Function for shortcutting setting up UART communication
//  *
//  * @param
// */
// esp_err_t uart_init(uart_mode_t mode, uart_port_t port, int baudrate, int tx_pin, int rx_pin, int rts_pin, int cts_pin);