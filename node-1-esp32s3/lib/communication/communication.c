#include "communication.h"

esp_err_t i2c_init(i2c_mode_t mode, i2c_port_t port, gpio_num_t sda, gpio_num_t scl)
{
	i2c_config_t i2c_config = {
		.mode = mode,
		.sda_io_num = sda,
		.scl_io_num = scl,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_SCLK_SRC_FLAG_FOR_NOMAL
	};
	ESP_ERROR_CHECK(i2c_param_config(port, &i2c_config));
	ESP_ERROR_CHECK(i2c_driver_install(port, mode, 0, 0, 0));
    return ESP_OK;
}
