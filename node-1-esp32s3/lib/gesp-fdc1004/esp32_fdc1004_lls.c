#include "esp32_fdc1004_lls.h"

esp_err_t check_fdc1004(i2c_port_t port)
{
    uint16_t data;
    read_register(port, FDC_DEVICE_ID_REG, &data);
    if (data != 0x1004)
    {
        printf("FDC1004 not detected!\n");
        return ESP_ERR_NOT_FOUND;
    }
    return ESP_OK;
}

esp_err_t fdc_reset(i2c_port_t port)
{
    esp_err_t error;
    uint8_t reset[3];
    reset[0] = FDC_REGISTER;
    reset[1] = 0x80;
    reset[2] = 0;


    error = i2c_master_write_to_device(port, FDC_SLAVE_ADDRESS, reset, sizeof(reset), pdMS_TO_TICKS(200));
    if (error != ESP_OK)
    {
        ESP_LOGE(FDC_TAG, "RESET ERROR | Code: 0x%.2X", error);
        return error;
    }
    
    uint16_t reset_status;
    for (uint8_t a = 0; a < 5; a++)
    {
        vTaskDelay(pdMS_TO_TICKS(50));
        read_register(port, FDC_REGISTER, &reset_status);
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

fdc_channel_t init_channel(i2c_port_t port, uint8_t channel, uint8_t rate)
{
    if (!FDC1004_IS_CHANNEL(channel))
        return NULL;

    if (!FDC1004_IS_RATE(rate))
        return NULL;

    fdc_channel_t new_channel = malloc(sizeof(fdc1004_channel));

    // Checks for failed memory allocation
    if (new_channel == NULL)
    {
        printf("Memory allocation for new channel failed!\n");
        return NULL;
    }

    // Assigning fields
    new_channel->port = port;
    new_channel->channel = channel;
    new_channel->rate = rate;

    new_channel->config_address = config[channel];
    new_channel->msb_address = msb_addresses[channel];
    new_channel->lsb_address = lsb_addresses[channel];
    new_channel->offset_register = offset_registers[channel];
    new_channel->gain_register = gain_registers[channel];

    new_channel->ma = init_moving_average();

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

esp_err_t read_register(i2c_port_t i2c_port_num, uint8_t reg_address, uint16_t *ret_data)
{
    esp_err_t error;
    uint8_t data[2] = {0};

    // Specify which register to read with pointer byte
    error = i2c_master_write_to_device(i2c_port_num, FDC_SLAVE_ADDRESS, &reg_address, sizeof(reg_address), pdMS_TO_TICKS(100));
    if (error != ESP_OK)
    {
        ESP_LOGE(FDC_TAG, "SPECIFY POINTER REGISTER ERROR | Code: 0x%.2X", error);
        return error;
    }
    // Sends read command for reading the current value in the register stored in the pointer register
    error = i2c_master_read_from_device(i2c_port_num, FDC_SLAVE_ADDRESS, data, 2, pdMS_TO_TICKS(100));
    if (error != ESP_OK)
    {
        ESP_LOGE(FDC_TAG, "READ REGISTER ERROR | Code: 0x%.2X", error);
        return error;
    }

    *ret_data = ((uint16_t)(data[0]) << 8) | (uint16_t)data[1];
    return ESP_OK;
}

esp_err_t configure_channel(fdc_channel_t channel_obj)
{
    esp_err_t error;
    uint8_t config[3];
    uint8_t gain[3];
    uint8_t offset[3];

    // Build 16 bit configuration
    // printf("Channel: %d\n", channel_obj->channel);
    uint16_t configuration = (uint16_t)(channel_obj->channel) << 13; // CHA
    configuration |= 0x1000;                                         // CAPDAC
    configuration |= 0x0300;                                         // CAPDAC Value

    config[0] = channel_obj->config_address;
    config[1] = (uint8_t)(configuration >> 8);
    config[2] = (uint8_t)(configuration);
    error = i2c_master_write_to_device(channel_obj->port, FDC_SLAVE_ADDRESS, config, sizeof(config), pdMS_TO_TICKS(200));
    if (error != ESP_OK)
        ESP_LOGE(FDC_TAG, "CONFIG ERROR | Code: 0x%.2X", error);

    // Configure gain
    int16_t integer_part = (uint16_t)(GAIN_CAL);
    uint8_t decimal_part = GAIN_CAL - integer_part;
    uint16_t encoded_gain = integer_part << 14;
    uint16_t encoded_decimal = (uint16_t)(decimal_part * 16383);
    encoded_gain |= encoded_decimal;

    gain[0] = channel_obj->gain_register;
    gain[1] = (uint8_t)(encoded_gain >> 8);
    gain[2] = (uint8_t)(encoded_gain);
    error = i2c_master_write_to_device(channel_obj->port, FDC_SLAVE_ADDRESS, gain, sizeof(gain), pdMS_TO_TICKS(200));
    if (error != ESP_OK)
        ESP_LOGE(FDC_TAG, "GAIN CONFIG ERROR | Code: 0x%.2X", error);

    // Configure offset
    integer_part = (uint16_t)(OFFSET_CAL);
    decimal_part = OFFSET_CAL - integer_part;
    uint16_t encoded_offset = integer_part << 14;
    encoded_decimal = (uint16_t)(decimal_part * 2047);
    encoded_offset |= encoded_decimal;

    offset[0] = channel_obj->offset_register;
    offset[1] = (uint8_t)(encoded_offset >> 8);
    offset[2] = (uint8_t)(encoded_offset);
    error = i2c_master_write_to_device(channel_obj->port, FDC_SLAVE_ADDRESS, offset, sizeof(offset), pdMS_TO_TICKS(200));
    if (error != ESP_OK)
        ESP_LOGE(FDC_TAG, "OFFSET CONFIG ERROR | Code: 0x%.2X", error);
    return ESP_OK;
}

esp_err_t update_measurement(fdc_channel_t channel_obj)
{
    uint16_t done_status;
    uint8_t trigger[3];

    // Build trigger for 16 bit register 0x0C
    uint16_t trigger_config = 0x0000;
    trigger_config |= (uint16_t)(channel_obj->rate) << 10;         // Sample Rate
    trigger_config |= (uint16_t)(1 << (7 - channel_obj->channel)); // Measurement channel

    // Write trigger command
    done_status = 0;
    read_register(channel_obj->port, FDC_REGISTER, &done_status);
    printf("Done Status Pre Trigger: %d\n", done_status);
    trigger[0] = FDC_REGISTER;
    trigger[1] = (uint8_t)(trigger_config >> 8);
    trigger[2] = (uint8_t)(trigger_config);
    ESP_ERROR_CHECK(i2c_master_write_to_device(channel_obj->port, FDC_SLAVE_ADDRESS, trigger, sizeof(trigger), pdMS_TO_TICKS(50)));
    done_status = 0;
    read_register(channel_obj->port, FDC_REGISTER, &done_status);
    printf("Done Status Post Trigger: %d\n", done_status);
    // error = i2c_master_write_to_device(channel_obj->port, FDC_SLAVE_ADDRESS, trigger, sizeof(trigger), pdMS_TO_TICKS(50));
    // if (error != ESP_OK)
    // {
    //     printf("Trigger failed! %d\n", error);
    //     return error;
    // }

    // Wait for measurement to complete
    vTaskDelay(pdMS_TO_TICKS(50));

    done_status = 0;
    read_register(channel_obj->port, FDC_REGISTER, &done_status);
    printf("Done Status Post Result: %d\n", done_status);
    // Check measurement done status
    if (done_status & (uint16_t)(1 << (7 - channel_obj->channel)))
    {
        // Measurement Done!
        uint16_t raw_msb = 0;
        uint16_t raw_lsb = 0;
        ESP_ERROR_CHECK(read_register(channel_obj->port, channel_obj->msb_address, &raw_msb));
        ESP_ERROR_CHECK(read_register(channel_obj->port, channel_obj->lsb_address, &raw_lsb));
        channel_obj->raw_msb = raw_msb;
        channel_obj->raw_lsb = raw_lsb;

        // int32_t measurement_value = ((int16_t)raw_msb << 8) | (int16_t)raw_lsb;
        int32_t raw_measurement_value = ((int32_t)raw_msb << 8) | ((int32_t)raw_lsb >> 8);
        printf("Raw value: %ld\n", raw_measurement_value);
        printf("Capacitance: %.2f pF\n", (float)(raw_measurement_value >> 16) / 8);
        printf("========================================\n");
        channel_obj->raw_value = (float)((raw_measurement_value >> 16) / 1000);
    }

    // Calculate capacitance
    // int32_t capacitance = (int32_t)ATTOFARADS_UPPER_WORD * (int32_t)raw_measurement_value; // in attofarads
    // capacitance /= 1000;                                                               // in femtofarads
    // capacitance += (int32_t)FEMTOFARADS_CAPDAC * (int32_t)(channel_obj->capdac);

    // Update capdac
    // update_capdac(channel_obj);

    // Store value

    // Update moving average
    // moving_average_enqueue(channel_obj->ma, (float)capacitance);

    // Update value
    // channel_obj->value = get_moving_average(channel_obj->ma) / 1000;

    return ESP_OK;
}

esp_err_t update_measurements(level_calc_t level_calc)
{
    esp_err_t esp_rc = check_fdc1004(level_calc->port);
    if (esp_rc != ESP_OK)
        return esp_rc;

    configure_channel(level_calc->ref_channel);
    // configure_channel(level_calc->lev_channel);
    // configure_channel(level_calc->env_channel);

    vTaskDelay(pdMS_TO_TICKS(1000));

    // Update all readings on all channels
    printf("REF PAD: \n");
    update_measurement(level_calc->ref_channel);
    printf("LEV PAD: \n");
    update_measurement(level_calc->lev_channel);
    // printf("ENV PAD:\n");
    // update_measurement(level_calc->env_channel);
    // update_measurement(level_calc->lnv_channel);

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

void timer_callback(TimerHandle_t xTimer)
{
    level_calc_t level_calc = (level_calc_t)pvTimerGetTimerID(xTimer);

    calibrate(level_calc);

    // printf("Interrupt! Calibration triggered!\n");
}

level_calc_t init_level_calculator(i2c_port_t port)
{
    level_calc_t new_calc = malloc(sizeof(level_calculator));

    // Initial calibration
    calibrate(new_calc);

    TimerHandle_t timer = xTimerCreate("MyTimer",                       // Timer name
                                       pdMS_TO_TICKS(CALIBRATION_FREQ), // Timer period in milliseconds (e.g., 1000 ms for 1 second)
                                       pdTRUE,                          // Auto-reload the timer
                                       (void *)new_calc,                // Timer parameters
                                       timer_callback);                 // Timer callback function

    xTimerStart(timer, 0);

    new_calc->ref_value = 0;
    new_calc->lev_value = 0;
    new_calc->env_value = 0;

    new_calc->port = port;
    new_calc->ref_channel = init_channel(port, REF_CHANNEL - 1, FDC1004_100HZ);
    new_calc->lev_channel = init_channel(port, LEV_CHANNEL - 1, FDC1004_100HZ);
    new_calc->env_channel = init_channel(port, ENV_CHANNEL - 1, FDC1004_100HZ);

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
    esp_err_t esp_rc = check_fdc1004(level->port);
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
