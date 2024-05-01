#include "esp32_fdc1004_lls.h"

esp_err_t read_register(i2c_master_dev_handle_t slave, uint8_t reg_address, uint16_t *ret_data)
{
    esp_err_t error;
    uint8_t data[2] = {0};
    uint8_t pointer_address = reg_address;

    // Specify which register to read with pointer byte
    i2c_clear_write_buffer();
    i2c_write_byte(pointer_address);
    error = i2c_transmit_write_buffer(slave);
    if (error != ESP_OK)
    {
        ESP_LOGE(FDC_TAG, "SPECIFY POINTER REGISTER ERROR | Code: 0x%.2X", error);
        return error;
    }
    // Sends read command for reading the current value in the register stored in the pointer register
    i2c_master_receive(slave, data, sizeof(data), 100);
    if (error != ESP_OK)
    {
        ESP_LOGE(FDC_TAG, "READ REGISTER ERROR | Code: 0x%.2X", error);
        return error;
    }

    // printf("MSB: %d\n", data[0]);
    // printf("LSB: %d\n", data[1] >> 8);
    *ret_data = ((uint16_t)(data[0]) << 8) | (uint16_t)data[1];
    return ESP_OK;
}

esp_err_t check_fdc1004(i2c_master_dev_handle_t slave_handle)
{
    uint16_t data;
    read_register(slave_handle, FDC_DEVICE_ID_REG, &data);
    if (data != 0x1004)
    {
        printf("FDC1004 not detected! Data: 0x%.4X\n", data);
        // printf("FDC1004 not detected!\n");
        return ESP_ERR_NOT_FOUND;
    }
    return ESP_OK;
}

esp_err_t fdc_reset(i2c_master_dev_handle_t slave_handle)
{
    esp_err_t error;

    i2c_clear_write_buffer();
    i2c_write_byte(FDC_REGISTER);
    i2c_write_byte(0x80);
    i2c_write_byte(0);
    
    error = i2c_transmit_write_buffer(slave_handle);
    if (error != ESP_OK)
    {
        ESP_LOGE(FDC_TAG, "RESET ERROR | Code: 0x%.2X", error);
        return error;
    }

    uint16_t reset_status;
    for (uint8_t a = 0; a < 5; a++)
    {
        vTaskDelay(pdMS_TO_TICKS(50));
        read_register(slave_handle, FDC_REGISTER, &reset_status);
        if ((reset_status & 0x80) == 0)
        {
            printf("=======================\n");
            printf("FDC1004 reset complete!\n");
            printf("=======================\n");
            return ESP_OK;
        }
    }
    return ESP_ERR_NOT_FOUND;
}

fdc_channel_t init_channel(i2c_master_dev_handle_t slave_handle, uint8_t channel, uint8_t rate)
{
    if (!FDC1004_IS_RATE(rate))
        return NULL;

    fdc_channel_t new_channel = malloc(sizeof(struct fdc1004_channel));

    if (new_channel == NULL)
    {
        printf("Memory allocation for new channel failed!\n");
        return NULL;
    }

    // Assigning fields
    new_channel->slave_handle = slave_handle;
    new_channel->channel = channel;
    new_channel->rate = rate;

    new_channel->config_address = config_address[channel];
    new_channel->msb_address = msb_addresses[channel];
    new_channel->lsb_address = lsb_addresses[channel];
    new_channel->offset_register = offset_registers[channel];
    new_channel->gain_register = gain_registers[channel];

    // new_channel->ma = init_moving_average();

    new_channel->raw_msb = 0;
    new_channel->raw_lsb = 0;
    new_channel->capdac = 0;
    new_channel->raw_value = 0;
    new_channel->value = 0;
    return new_channel;
}

esp_err_t del_channel(fdc_channel_t channel_obj)
{
    if (channel_obj == NULL)
        return ESP_ERR_INVALID_ARG;
    free(channel_obj);
    return ESP_OK;
}

esp_err_t validate_channel_obj(fdc_channel_t channel_obj)
{
    if (!FDC1004_IS_CHANNEL(channel_obj->channel))
        return ESP_ERR_INVALID_ARG;

    if (!FDC1004_IS_RATE(channel_obj->rate))
        return ESP_ERR_INVALID_ARG;

    if (!FDC1004_IS_CONFIG_ADDRESS(channel_obj->config_address))
        return ESP_ERR_INVALID_ARG;

    if (!FDC1004_IS_MSB_ADDRESS(channel_obj->msb_address))
        return ESP_ERR_INVALID_ARG;

    if (!FDC1004_IS_LSB_ADDRESS(channel_obj->lsb_address))
        return ESP_ERR_INVALID_ARG;

    return ESP_OK;
}

esp_err_t configure_channel(fdc_channel_t channel_obj)
{
    esp_err_t error;
    // uint8_t gain[3];
    // uint8_t offset[3];

    // Build 16 bit configuration
    uint16_t configuration = (uint16_t)(channel_obj->channel) << 13; // CHA
    configuration |= 0x1000;                                         // CAPDAC
    // configuration |= ((uint16_t)0x04) << 10;                         // CHB disable / CAPDAC enable
    // configuration |= 20 << 5;                                        // CAPDAC value

    i2c_clear_write_buffer();
    i2c_write_byte(channel_obj->config_address);
    i2c_write_byte((uint8_t)(configuration >> 8));
    i2c_write_byte((uint8_t)(configuration));
    error = i2c_transmit_write_buffer(channel_obj->slave_handle);

    // error = i2c_master_write_to_device(channel_obj->port, FDC_SLAVE_ADDRESS, config, sizeof(config), pdMS_TO_TICKS(200));
    if (error != ESP_OK)
        ESP_LOGE(FDC_TAG, "CONFIG ERROR | Code: 0x%.2X", error);

    int16_t integer_part;
    uint8_t decimal_part;
    uint16_t encoded_gain;
    uint16_t encoded_decimal;
    uint16_t encoded_offset;

    integer_part = (uint16_t)(GAIN_CAL);
    decimal_part = GAIN_CAL - integer_part;
    encoded_gain = integer_part << 14;
    encoded_decimal = (uint16_t)(decimal_part * (1 << 14));
    encoded_gain |= encoded_decimal;
    encoded_gain = 0x4000;

    i2c_clear_write_buffer();
    i2c_write_byte(channel_obj->gain_register);
    i2c_write_byte((uint8_t)(encoded_gain >> 8));
    i2c_write_byte((uint8_t)(encoded_gain));
    error = i2c_transmit_write_buffer(channel_obj->slave_handle);
    if (error != ESP_OK)
        ESP_LOGE(FDC_TAG, "GAIN CONFIG ERROR | Code: 0x%.2X", error);

    // Configure offset
    integer_part = (uint16_t)(OFFSET_CAL);
    decimal_part = OFFSET_CAL - integer_part;
    encoded_decimal = (int16_t)(decimal_part * (1 << 14));
    if (OFFSET_CAL >= 0)
    {
        encoded_offset = (uint16_t)(integer_part) << 10;
    }
    else
    {
        encoded_offset = ~(uint16_t)(encoded_decimal) << 10;
    }
    encoded_offset |= encoded_decimal;

    i2c_clear_write_buffer();
    i2c_write_byte(channel_obj->offset_register);
    i2c_write_byte((uint8_t)(encoded_offset >> 8));
    i2c_write_byte((uint8_t)(encoded_offset));
    error = i2c_transmit_write_buffer(channel_obj->slave_handle);
    if (error != ESP_OK)
        ESP_LOGE(FDC_TAG, "OFFSET CONFIG ERROR | Code: 0x%.2X", error);
    
    return ESP_OK;
}

esp_err_t update_measurement(fdc_channel_t channel_obj)
{
    uint16_t done_status;

    // Build trigger for 16 bit register 0x0C
    uint16_t trigger_config = 0x0000;
    trigger_config |= 0x0040;         // Sample Rate
    trigger_config |= 0x0080 >> channel_obj->channel; // Measurement channel

    i2c_clear_write_buffer();
    i2c_write_byte(FDC_REGISTER);
    i2c_write_byte((uint8_t)(trigger_config >> 8));
    i2c_write_byte((uint8_t)(trigger_config));
    i2c_transmit_write_buffer(channel_obj->slave_handle);

    // done_status = 0;
    // read_register(channel_obj->port, FDC_REGISTER, &done_status);
    // printf("Done Status Post Trigger: %d\n", done_status);

    // error = i2c_master_write_to_device(channel_obj->port, FDC_SLAVE_ADDRESS, trigger, sizeof(trigger), pdMS_TO_TICKS(50));
    // if (error != ESP_OK)
    // {
    //     printf("Trigger failed! %d\n", error);
    //     return error;
    // }

    vTaskDelay(pdMS_TO_TICKS(100));

    done_status = 0;
    read_register(channel_obj->slave_handle, FDC_REGISTER, &done_status);
    printf("Done Status Post Result: %d\n", done_status);
    done_status &= (0x0008 >> channel_obj->channel);
    // Check measurement done status
    if (done_status > 0)
    // if ((done_status > 0) && (uint16_t)(1 << (channel_obj->channel + 4)))
    {
        // Measurement Done!
        uint16_t raw_msb = 0;
        uint16_t raw_lsb = 0;
        read_register(channel_obj->slave_handle, channel_obj->lsb_address, &raw_lsb);
        read_register(channel_obj->slave_handle, channel_obj->msb_address, &raw_msb);
        channel_obj->raw_msb = raw_msb;
        channel_obj->raw_lsb = raw_lsb;

        // read_register(channel_obj->port, FDC_REGISTER, &done_status);
        // printf("Done Status Post Result Return: %d\n", done_status);

        int32_t raw_measurement_value = ((int32_t)raw_msb << 8) | ((int32_t)raw_lsb >> 8);
        printf("Raw value: %ld\n", raw_measurement_value);
        printf("Capacitance: %.2f pF\n", (float)(raw_measurement_value >> 16) / 8);
        printf("========================================\n");
        channel_obj->raw_value = (float)((raw_measurement_value >> 16) / 8);
    }

    // Calculate capacitance
    // int32_t capacitance = (int32_t)ATTOFARADS_UPPER_WORD * (int32_t)raw_measurement_value; // in attofarads
    // capacitance /= 1000;                                                               // in femtofarads
    // capacitance += (int32_t)FEMTOFARADS_CAPDAC * (int32_t)(channel_obj->capdac);

    // update_capdac(channel_obj);

    // moving_average_enqueue(channel_obj->ma, (float)capacitance);

    // channel_obj->value = get_moving_average(channel_obj->ma) / 1000;

    return ESP_OK;
}

esp_err_t update_measurements(level_calc_t level_calc)
{
    esp_err_t esp_rc = check_fdc1004(level_calc->slave_handle);
    if (esp_rc != ESP_OK)
        return esp_rc;

    configure_channel(level_calc->ref_channel);
    configure_channel(level_calc->lev_channel);
    configure_channel(level_calc->env_channel);

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Update all readings on all channels
    printf("REF PAD: \n");
    update_measurement(level_calc->ref_channel);
    printf("LEV PAD: \n");
    update_measurement(level_calc->lev_channel);
    printf("ENV PAD: \n");
    update_measurement(level_calc->env_channel);

    level_calc->ref_value = level_calc->ref_channel->value;
    level_calc->lev_value = level_calc->lev_channel->value;
    level_calc->env_value = level_calc->env_channel->value;
    // level_calc->env_value = update_measurement(level_calc->lvl_env_channel);
    return ESP_OK;
}

esp_err_t update_capdac(fdc_channel_t channel_obj)
{
    if ((int16_t)(channel_obj->raw_msb) > FDC1004_UPPER_BOUND) // adjust capdac accordingly
    {
        if (channel_obj->capdac < FDC1004_CAPDAC_MAX)
            channel_obj->capdac++;
    }
    else if ((int16_t)(channel_obj->raw_msb) < FDC1004_LOWER_BOUND)
    {
        if (channel_obj->capdac > 0)
            channel_obj->capdac--;
    }
    return ESP_OK;
}

level_calc_t init_fdc1004(i2c_master_bus_handle_t master_bus)
{
    level_calc_t new_calc = malloc(sizeof(level_calculator));

    // Initial calibration
    calibrate(new_calc);

    // TimerHandle_t timer = xTimerCreate("MyTimer",                       // Timer name
    //                                    pdMS_TO_TICKS(CALIBRATION_FREQ), // Timer period in milliseconds (e.g., 1000 ms for 1 second)
    //                                    pdTRUE,                          // Auto-reload the timer
    //                                    (void *)new_calc,                // Timer parameters
    //                                    timer_callback);                 // Timer callback function

    // xTimerStart(timer, 0);

    new_calc->ref_value = 0;
    new_calc->lev_value = 0;
    new_calc->env_value = 0;

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = FDC_SLAVE_ADDRESS,
        .scl_speed_hz = 100000,
    };

    i2c_master_dev_handle_t slave_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(master_bus, &dev_cfg, &slave_handle));

    new_calc->master_bus = master_bus;
    new_calc->slave_handle = slave_handle;
    new_calc->ref_channel = init_channel(slave_handle, REF_CHANNEL - 1, FDC1004_400HZ);
    new_calc->lev_channel = init_channel(slave_handle, LEV_CHANNEL - 1, FDC1004_400HZ);
    new_calc->env_channel = init_channel(slave_handle, ENV_CHANNEL - 1, FDC1004_400HZ);

    configure_channel(new_calc->ref_channel);
    configure_channel(new_calc->lev_channel);
    configure_channel(new_calc->env_channel);

    return new_calc;
}

esp_err_t calibrate(level_calc_t level)
{
    level->current_delta = level->ref_value - level->env_value;
    // level->current_delta = level->ref_value - REF_BASELINE;
    if (level->current_delta == 0)
    {
        printf("Calibration Failed! DELTA 0\n");
    }

    // Calculates the predicted trend
    float forecast_m = 0, forecast_b = 0;
    forecast_m = level->current_delta / 5;
    forecast_b = LEV_BASELINE - forecast_m * 5;

    // printf("(2) Forecast m: %.2f\n", forecast_m);
    // printf("(2) Forecast b: %.2f\n", forecast_b);

    level->correction_gain = 1 / forecast_m;
    level->correction_offset = -1 * level->correction_gain * forecast_b;

    return ESP_OK;
}

uint8_t round_nearest_multiple(float value, uint8_t multiple)
{
    return (int)((value + (multiple / 2)) / multiple) * multiple;
}

uint8_t calculate_level(level_calc_t level)
{
    esp_err_t esp_rc = check_fdc1004(level->slave_handle);
    if (esp_rc != ESP_OK)
        return esp_rc;

    // if (level->ref_value < 0 || level->lev_value < 0 || level->env_value < 0)
    //     printf("ERROR: NEGATIVE VAL!\n");

    // if (level->ref_value < REF_BASELINE - 0.2)
    //     fdc_reset(level->ref_channel->port);

    if (level->lev_value < LEV_BASELINE)
        return 0;

    // Apply linear correction
    float linear_corrected = (level->lev_value * CORRECTION_MULTIPLIER * level->correction_gain) + (CORRECTION_OFFSET + level->correction_offset);

    printf("Linear Corrected: %f\n", linear_corrected);

    return round_nearest_multiple(linear_corrected, 5);
}

void fdc1004_main(void *pvParameter)
{
    esp_err_t esp_rc;

    i2c_master_bus_handle_t bus = *((i2c_master_bus_handle_t *)pvParameter);
    level_calc_t level_sensor = init_fdc1004(bus);

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_rc = update_measurements(level_sensor);
        // if (esp_rc == ESP_OK)
        // {
        //     calculate_level(level_sensor);
        // }
    }
}