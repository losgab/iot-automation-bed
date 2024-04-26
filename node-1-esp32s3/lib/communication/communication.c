#include "communication.h"

esp_err_t i2c_master_init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl, i2c_master_bus_handle_t ret_handle)
{
	i2c_master_bus_config_t i2c_mst_config = {
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.i2c_port = port,
		.scl_io_num = scl,
		.sda_io_num = sda,
		.glitch_ignore_cnt = 7,
		.flags.enable_internal_pullup = true,
	};

	ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, ret_handle));
	
	return ESP_OK;
}