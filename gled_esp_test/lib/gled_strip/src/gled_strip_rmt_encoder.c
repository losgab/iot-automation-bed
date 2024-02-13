#include "gled_strip_rmt_encoder.h"

static size_t gled_strip_rmt_encoder_encode(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    gled_strip_rmt_encoder_t *strip_encoder = __containerof(encoder, gled_strip_rmt_encoder_t, base);
    rmt_encoder_handle_t bytes_encoder = strip_encoder->bytes_encoder;
    rmt_encoder_handle_t copy_encoder = strip_encoder->copy_encoder;
    rmt_encode_state_t session_state = 0;
    rmt_encode_state_t state = 0;
    size_t encoded_symbols = 0;
    switch (strip_encoder->state)
    {
    case 0: // send RGB data
        encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE)
        {
            strip_encoder->state = 1; // switch to next state when current encoding session finished
        }
        if (session_state & RMT_ENCODING_MEM_FULL)
        {
            state |= RMT_ENCODING_MEM_FULL;
            ESP_LOGI(RMT_ENCODER_TAG, "No free space for encoding artifacts (case 0)");
            break;
        }
    case 1: // send reset code
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &strip_encoder->reset_code,
                                                sizeof(strip_encoder->reset_code), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE)
        {
            strip_encoder->state = 0; // back to the initial encoding session
            state |= RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL)
        {
            state |= RMT_ENCODING_MEM_FULL;
            ESP_LOGI(RMT_ENCODER_TAG, "No free space for encoding artifacts (case 1)");
            break;
        }
    case 2:
        ESP_LOGI(RMT_ENCODER_TAG, "No free space for encoding artifacts (case 2)");
        break;
    }
    *ret_state = state;
    return ESP_OK;
}

static esp_err_t gled_strip_rmt_encoder_reset(gled_strip_rmt_encoder_t *strip_encoder)
{
    rmt_encoder_reset(strip_encoder->bytes_encoder);
    rmt_encoder_reset(strip_encoder->copy_encoder);
    strip_encoder->state = 0;
    return ESP_OK;
}

esp_err_t gled_strip_rmt_encoder_del(gled_strip_rmt_encoder_t *strip_encoder)
{
    rmt_del_encoder(strip_encoder->bytes_encoder);
    rmt_del_encoder(strip_encoder->copy_encoder);
    free(strip_encoder);
    return ESP_OK;
}
esp_err_t gled_strip_new_rmt_encoder(gled_strip_rmt_encoder_t *strip_encoder)
{
    // ESP_RETURN_ON_FALSE(ret_device == NULL, ESP_ERR_INVALID_ARG, RMT_ENCODER_TAG, "Encoder Not NULL");

    gled_strip_rmt_encoder_t *new_encoder = malloc(sizeof(gled_strip_rmt_encoder_t));
    ESP_RETURN_ON_FALSE(new_encoder == NULL, ESP_ERR_NO_MEM, RMT_ENCODER_TAG, "No Memory left for RMT encoder");

    new_encoder->base.encode = gled_strip_rmt_encoder_encode;
    new_encoder->base.reset = gled_strip_rmt_encoder_reset;
    new_encoder->base.del = gled_strip_rmt_encoder_del;

    rmt_bytes_encoder_config_t bytes_encoder_config;
    bytes_encoder_config = (rmt_bytes_encoder_config_t){
        .bit0 = {
            .level0 = 1,
            .duration0 = 0.3 * GLED_STRIP_RMT_DEFAULT_RESOLUTION / 1000000, // T0H=0.3us
            .level1 = 0,
            .duration1 = 0.9 * GLED_STRIP_RMT_DEFAULT_RESOLUTION / 1000000, // T0L=0.9us
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = 0.9 * GLED_STRIP_RMT_DEFAULT_RESOLUTION / 1000000, // T1H=0.9us
            .level1 = 0,
            .duration1 = 0.3 * GLED_STRIP_RMT_DEFAULT_RESOLUTION / 1000000, // T1L=0.3us
        },
        .flags.msb_first = 1 // WS2812 transfer bit order: G7...G0R7...R0B7...B0
    };
    ESP_RETURN_ON_ERROR(rmt_new_bytes_encoder(&bytes_encoder_config, &new_encoder->bytes_encoder), RMT_ENCODER_TAG, "create bytes encoder failed");
    rmt_copy_encoder_config_t copy_encoder_config;
    ESP_RETURN_ON_ERROR(rmt_new_copy_encoder(&copy_encoder_config, &new_encoder->copy_encoder), RMT_ENCODER_TAG, "create copy encoder failed");

    uint32_t reset_ticks = GLED_STRIP_RMT_DEFAULT_RESOLUTION / 1000000 * 50 / 2; // reset code duration defaults to 50us
    new_encoder->reset_code = (rmt_symbol_word_t){
        .level0 = 0,
        .duration0 = reset_ticks,
        .level1 = 0,
        .duration1 = reset_ticks,
    };

    strip_encoder = new_encoder;
    return ESP_OK;
}
