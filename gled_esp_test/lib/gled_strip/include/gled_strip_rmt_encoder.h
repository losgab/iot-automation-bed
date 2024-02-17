/**
 * RMT Encoder  for LED Strip Library for ESP-IDF
 *
 * @author Gabriel Thien 2024
 */
#pragma once

#include <esp_check.h>

#include <stdint.h>
#include <driver/rmt_encoder.h>
#include <driver/rmt_types.h>
// #include "gled_strip_rmt.h"

#define GLED_STRIP_RMT_DEFAULT_RESOLUTION 10000000 // 10MHz resolution

#define RMT_ENCODER_TAG "RMT_ENCODER"

typedef struct gled_strip_rmt_encoder
{
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    rmt_encode_state_t state;
    rmt_symbol_word_t reset_code;
} gled_strip_rmt_encoder_t;

/**
 * @brief Initialises a new RMT Encoder for interpreting LED strip signals into RMT frames
 *
 * @param ret_encoder Pointer to the encoder handle to initialise
 *
 * @return ESP_OK on success, otherwise an error code
 */
esp_err_t gled_strip_new_rmt_encoder(gled_strip_rmt_encoder_t **strip_encoder);

/**
 * @brief Delete RMT Encoder & Free resources
 *
 * @param strip_encoder GLED RMT Encoder handle
 *
 * @return esp_err_t ESP_OK on success
 */
esp_err_t gled_strip_rmt_encoder_del(rmt_encoder_t *encoder);
