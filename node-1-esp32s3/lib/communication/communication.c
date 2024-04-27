#include "communication.h"

unsigned int write_buffer_len = 0;
uint8_t write_buffer[BUFF_LEN] = {0};

esp_err_t i2c_master_init(i2c_port_t port, gpio_num_t sda, gpio_num_t scl, i2c_master_bus_handle_t *ret_handle)
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
	i2c_clear_write_buffer();
	return ESP_OK;
}

void i2c_clear_write_buffer()
{
	memset(write_buffer, 0, BUFF_LEN);
	write_buffer_len = 0;
}

void i2c_write_byte(const uint8_t byte)
{
	if (write_buffer_len == BUFF_LEN)
	{
		ESP_LOGE("communication: i2c_add_write_buffer", "Buffer is full");
		return;
	}
	write_buffer[write_buffer_len] = byte;
	write_buffer_len++;
}

void i2c_write_bytes(const uint8_t *bytes, const int len)
{
	for (int i = 0; i < len; i++)
	{
		if (write_buffer_len == BUFF_LEN)
		{
			ESP_LOGE("communication: i2c_add_write_buffer", "Buffer is full");
			return;
		}
		write_buffer[write_buffer_len + i] = bytes[i];
	}
	write_buffer_len = write_buffer_len + len;
}

esp_err_t i2c_transmit_write_buffer(i2c_master_dev_handle_t slave_handle)
{
	esp_err_t esp_rc;
	
	esp_rc = i2c_master_transmit(slave_handle, write_buffer, write_buffer_len, 100);

	if (esp_rc != ESP_OK)
	{
		ESP_LOGE("communication: i2c_transmit_write_buffer", "Error transmitting buffer: %d", esp_rc);
		return esp_rc;
	}
	else
		i2c_clear_write_buffer();

	return ESP_OK;
}