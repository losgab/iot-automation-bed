#include "gled_strip_rmt.h"
#include <string.h>

esp_err_t gled_strip_new_rmt_device(gled_strip_rmt_device_t *rmt_device, gpio_num_t pin, uint16_t num_leds)
{
    ESP_RETURN_ON_FALSE(rmt_device == NULL, ESP_ERR_INVALID_ARG, RMT_DEVICE_TAG, "Strip Not NULL");

    // Allocate memory for RMT device
    gled_strip_rmt_device_t *new_device = malloc(sizeof(gled_strip_rmt_device_t));
    ESP_RETURN_ON_FALSE(new_device == NULL, ESP_ERR_NO_MEM, RMT_DEVICE_TAG, "No Memory left for RMT device");

    // Create new strip RMT encoder
    new_device->strip_encoder = NULL;
    ESP_RETURN_ON_ERROR(gled_strip_new_rmt_encoder(new_device->strip_encoder), RMT_DEVICE_TAG, "Create LED strip encoder failed");

    new_device->num_leds = num_leds;
    new_device->bytes_per_pixel = 3;

    // RMT Config
    rmt_tx_channel_config_t rmt_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = pin,
        .mem_block_symbols = LED_STRIP_RMT_DEFAULT_MEM_BLOCK_SYMBOLS,
        .resolution_hz = GLED_STRIP_RMT_DEFAULT_RESOLUTION,
        .trans_queue_depth = GLED_STRIP_RMT_DEFAULT_TRANS_QUEUE_SIZE,
        .flags.with_dma = 1,
        .flags.invert_out = 1,
    };
    ESP_RETURN_ON_ERROR(rmt_new_tx_channel(&rmt_chan_config, &new_device->rmt_config.rmt_chan), RMT_DEVICE_TAG, "Create RMT TX channel failed");
    new_device->rmt_config.clk_src = RMT_CLK_SRC_DEFAULT;
    new_device->rmt_config.resolution_hz = GLED_STRIP_RMT_DEFAULT_RESOLUTION;
    new_device->rmt_config.mem_block_symbols = LED_STRIP_RMT_DEFAULT_MEM_BLOCK_SYMBOLS;
    new_device->rmt_config.with_dma = 1;
    new_device->rmt_config.invert_out = 1;

    // Direct RMT device pointer to new device that was just created
    rmt_device = new_device;
    return ESP_OK;
}

esp_err_t gled_strip_rmt_set_pixel(gled_strip_rmt_device_t *rmt_device, uint16_t index, uint8_t red, uint8_t green, uint8_t blue)
{
    ESP_RETURN_ON_FALSE(index < rmt_device->num_leds, ESP_ERR_INVALID_ARG, RMT_DEVICE_TAG, "Given Index out of initialised number of LEDs");
    uint32_t start = index * rmt_device->bytes_per_pixel;
    // In thr order of GRB, as LED strip like WS2812 sends out pixels in this order
    // Only supports RGB pixels for now
    rmt_device->pixel_buffer[start + 0] = green;
    rmt_device->pixel_buffer[start + 1] = red;
    rmt_device->pixel_buffer[start + 2] = blue;
    return ESP_OK;
}

static esp_err_t gled_strip_rmt_refresh(gled_strip_rmt_device_t *rmt_device)
{
    rmt_transmit_config_t tx_conf = {
        .loop_count = 0,
    };

    ESP_RETURN_ON_ERROR(rmt_enable(rmt_device->rmt_config.rmt_chan), RMT_DEVICE_TAG, "enable RMT channel failed");
    ESP_RETURN_ON_ERROR(rmt_transmit(rmt_device->rmt_config.rmt_chan, &rmt_device->strip_encoder->base, rmt_device->pixel_buffer,
                                     rmt_device->num_leds * rmt_device->bytes_per_pixel, &tx_conf),
                        RMT_DEVICE_TAG, "transmit pixels by RMT failed");
    ESP_RETURN_ON_ERROR(rmt_tx_wait_all_done(rmt_device->rmt_config.rmt_chan, -1), RMT_DEVICE_TAG, "flush RMT channel failed");
    ESP_RETURN_ON_ERROR(rmt_disable(rmt_device->rmt_config.rmt_chan), RMT_DEVICE_TAG, "disable RMT channel failed");
    return ESP_OK;
}

static esp_err_t gled_strip_rmt_clear(gled_strip_rmt_device_t *rmt_device)
{
    // Write zero to turn off all leds
    memset(rmt_device->pixel_buffer, 0, rmt_device->num_leds * rmt_device->bytes_per_pixel);
    return gled_strip_rmt_refresh(rmt_device);
}

esp_err_t gled_strip_rmt_del(gled_strip_rmt_device_t *rmt_device)
{
    ESP_RETURN_ON_ERROR(gled_strip_rmt_encoder_del(rmt_device->strip_encoder), RMT_DEVICE_TAG, "delete strip encoder failed");
    ESP_RETURN_ON_ERROR(rmt_del_channel(rmt_device->rmt_config.rmt_chan), RMT_DEVICE_TAG, "delete RMT channel failed");
    free(rmt_device);
    return ESP_OK;
}

esp_err_t gled_strip_new_rmt_interface(gled_strip_rmt_interface *interface)
{
    interface->set_pixel = gled_strip_rmt_set_pixel;
    interface->refresh = gled_strip_rmt_refresh;
    interface->clear = gled_strip_rmt_clear;
    interface->del = gled_strip_rmt_del;
    return ESP_OK;
}
