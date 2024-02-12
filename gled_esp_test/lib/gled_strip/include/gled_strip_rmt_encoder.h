/**
 * RMT Encoder  for LED Strip Library for ESP-IDF
 *
 * @author Gabriel Thien 2024
 */
#pragma once

#include <stdint.h>
#include <driver/rmt_encoder.h>
#include <driver/rmt_types.h>
#include "gled_strip_rmt.h"

#define RMT_ENCODER_TAG "RMT_ENCODER"

typedef struct
{
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    rmt_encode_state_t state;
    rmt_symbol_word_t reset_code;
} gled_strip_rmt_encoder;

/**
 * @brief Initialises a new RMT Encoder for interpreting LED strip signals into RMT frames
 *
 * @param ret_encoder Pointer to the encoder handle to initialise
 *
 * @return ESP_OK on success, otherwise an error code
 */
esp_err_t gled_strip_new_rmt_encoder(struct gled_strip_rmt_device *ret_device);
