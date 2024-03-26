#include "esp32_fdc1004_lls.h"

esp_err_t fdc_reset(i2c_port_t i2c_port_num)
{
    uint8_t reset[3];
    reset[0] = FDC_REGISTER;
    reset[1] = 1 << 7;;
    reset[2] = 0;

    uint8_t error = i2c_master_write_to_device(i2c_port_num, FDC_SLAVE_ADDRESS, reset, sizeof(reset), pdMS_TO_TICKS(1000));
    if (error != ESP_OK)
    {
        printf("Software Reset Failed! %d\n", error);
        return error;
    }
    return ESP_OK;
}

fdc_channel_t init_channel(i2c_port_t i2c_port_num, uint8_t channel, uint8_t rate)
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
    new_channel->i2c_port_num = i2c_port_num;
    new_channel->channel = channel;
    new_channel->rate = rate;

    new_channel->config_address = config[channel];
    new_channel->msb_address = msb_addresses[channel];
    new_channel->lsb_address = lsb_addresses[channel];

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

esp_err_t read_register(i2c_port_t i2c_port_num, uint8_t reg_address, uint8_t *ret_data)
{
    esp_err_t error;

    // Specify which register to read with pointer byte
    error = i2c_master_write_to_device(i2c_port_num, FDC_SLAVE_ADDRESS, &reg_address, sizeof(reg_address), pdMS_TO_TICKS(100));
    if (error != ESP_OK)
    {
        printf("Specify read register failed! %d\n", error);
        return error;
    }

    // Sends read command for reading the current value in the register stored in the pointer register
    error = i2c_master_read_from_device(i2c_port_num, FDC_SLAVE_ADDRESS, ret_data, 2, pdMS_TO_TICKS(100));
    if (error != ESP_OK)
    {
        printf("Read byte in register failed! %d\n", error);
        return error;
    }
    return ESP_OK;
}

esp_err_t configure_single_measurement(fdc_channel_t channel_obj)
{
    int8_t error;
    uint8_t config[3];

    // Validation
    if (validate_channel_obj(channel_obj))
        return ESP_ERR_INVALID_ARG;

    // Build 16 bit configuration
    uint16_t configuration = 0;
    configuration = (uint16_t)(channel_obj->channel) << 13; // CHA
    configuration |= ((uint16_t)0x04) << 10;                // CHB disable * CAPDAC enable
    configuration |= (uint16_t)(channel_obj->capdac) << 5;  // Capdac Value

    config[0] = channel_obj->config_address;
    config[1] = (uint8_t)(configuration >> 8);
    config[2] = (uint8_t)(configuration);
    error = i2c_master_write_to_device(channel_obj->i2c_port_num, FDC_SLAVE_ADDRESS, config, sizeof(config), pdMS_TO_TICKS(1000));
    if (error != ESP_OK)
        printf("Configure measurement failed! %d\n", error);

    return ESP_OK;
}

esp_err_t update_measurement(fdc_channel_t channel_obj)
{
    int8_t error;
    uint8_t trigger[3];

    // Validation
    if (validate_channel_obj(channel_obj))
        return ESP_ERR_INVALID_ARG;

    // Build trigger for register 0x0C
    uint16_t trigger_config = 0;
    trigger_config = (uint16_t)(channel_obj->rate) << 10; // Sample Rate
    trigger_config |= 0 << 8;                             // Disable repeat
    trigger_config |= (1 << (7 - channel_obj->channel));

    // Write trigger command
    trigger[0] = FDC_REGISTER;
    trigger[1] = (uint8_t)(trigger_config >> 8);
    trigger[2] = (uint8_t)(trigger_config);
    error = i2c_master_write_to_device(channel_obj->i2c_port_num, FDC_SLAVE_ADDRESS, trigger, sizeof(trigger), pdMS_TO_TICKS(10));
    if (error != ESP_OK)
    {
        printf("Trigger failed! %d\n", error);
        return error;
    }

    // Wait for measurement to complete
    vTaskDelay(pdMS_TO_TICKS(50));

    // Read value & store
    // uint8_t raw_msb = read_register(channel_obj->i2c_port_num, channel_obj->msb_address);
    // uint8_t raw_lsb = read_register(channel_obj->i2c_port_num, channel_obj->lsb_address);
    uint8_t raw_msb = 0;
    uint8_t raw_lsb = 0;
    channel_obj->raw_msb = raw_msb;
    channel_obj->raw_lsb = raw_lsb;

    int16_t measurement_value = ((int16_t)raw_msb << 8) | (int16_t)raw_lsb;

    // Calculate capacitance
    int32_t capacitance = (int32_t)ATTOFARADS_UPPER_WORD * (int32_t)measurement_value; // in attofarads
    capacitance /= 1000;                                                               // in femtofarads
    capacitance += (int32_t)FEMTOFARADS_CAPDAC * (int32_t)(channel_obj->capdac);

    // Update capdac
    // update_capdac(channel_obj);

    // Store value
    channel_obj->raw_value = ((float)capacitance / 1000);

    // Update moving average
    moving_average_enqueue(channel_obj->ma, (float)capacitance);

    // Update value
    channel_obj->value = get_moving_average(channel_obj->ma) / 1000;

    return ESP_OK;
}

esp_err_t update_measurements(level_calc_t level_calc)
{
    // Update all readings on all channels
    update_measurement(level_calc->ref_channel);
    update_measurement(level_calc->lev_channel);
    update_measurement(level_calc->env_channel);
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

level_calc_t init_level_calculator()
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

    new_calc->ref_channel = init_channel(I2C_NUM_0, REF_CHANNEL, FDC1004_100HZ);
    new_calc->lev_channel = init_channel(I2C_NUM_0, LEV_CHANNEL, FDC1004_100HZ);
    new_calc->env_channel = init_channel(I2C_NUM_0, ENV_CHANNEL, FDC1004_100HZ);

    configure_single_measurement(new_calc->ref_channel);
    configure_single_measurement(new_calc->lev_channel);
    configure_single_measurement(new_calc->env_channel);

    return new_calc;
}

void timer_callback(TimerHandle_t xTimer)
{
    level_calc_t level_calc = (level_calc_t)pvTimerGetTimerID(xTimer);

    calibrate(level_calc);

    // printf("Interrupt! Calibration triggered!\n");
}

esp_err_t calibrate(level_calc_t level)
{
    level->current_delta = level->ref_value - level->env_value;
    // level->current_delta = level->ref_value - REF_BASELINE;
    if (level->current_delta == 0)
    {
        printf("Calibration Failed!\n");
    }

    // Calculates the predicted trend
    float forecast_m = 0, forecast_b = 0;
    forecast_m = level->current_delta / 5;
    forecast_b = LEV_BASELINE - forecast_m * 5;

    printf("(2) Forecast m: %.2f\n", forecast_m);
    printf("(2) Forecast b: %.2f\n", forecast_b);

    level->correction_gain = round_2dp(1 / forecast_m);
    level->correction_offset = round_2dp(-1 * level->correction_gain * forecast_b);

    return ESP_OK;
}

uint8_t calculate_level(level_calc_t level)
{
    if (level->ref_value < REF_BASELINE - 0.2)
        fdc_reset(level->ref_channel->i2c_port_num);

    if (level->lev_value < LEV_BASELINE)
        return 0;

    // Apply linear correction
    float linear_corrected = (level->lev_value * CORRECTION_MULTIPLIER * level->correction_gain) + (CORRECTION_OFFSET + level->correction_offset);

    printf("Linear Corrected: %f\n", linear_corrected);

    return round_nearest_multiple(linear_corrected, 5);
}

float round_2dp(float value)
{
    return (float)((int)(value * 100 + .5) / 100);
}

uint8_t round_nearest_multiple(float value, uint8_t multiple)
{
    return (int)((value + (multiple / 2)) / multiple) * multiple;
}
