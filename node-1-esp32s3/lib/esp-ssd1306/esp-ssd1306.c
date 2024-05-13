#include "font8x8_basic.h"
#include "esp-ssd1306.h"

// void task_ssd1306_scroll(void *ignore)
// {
// 	esp_err_t espRc;

// 	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
// 	i2c_master_start(cmd);

// 	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
// 	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

// 	i2c_master_write_byte(cmd, 0x29, true); // vertical and horizontal scroll (p29)
// 	i2c_master_write_byte(cmd, 0x00, true);
// 	i2c_master_write_byte(cmd, 0x00, true);
// 	i2c_master_write_byte(cmd, 0x07, true);
// 	i2c_master_write_byte(cmd, 0x01, true);
// 	i2c_master_write_byte(cmd, 0x3F, true);

// 	i2c_master_write_byte(cmd, 0xA3, true); // set vertical scroll area (p30)
// 	i2c_master_write_byte(cmd, 0x20, true);
// 	i2c_master_write_byte(cmd, 0x40, true);

// 	i2c_master_write_byte(cmd, 0x2F, true); // activate scroll (p29)

// 	i2c_master_stop(cmd);
// 	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
// 	if (espRc == ESP_OK)
// 	{
// 		ESP_LOGI(SSD1306_TAG, "Scroll command succeeded");
// 	}
// 	else
// 	{
// 		ESP_LOGE(SSD1306_TAG, "Scroll command failed. code: 0x%.2X", espRc);
// 	}

// 	i2c_cmd_link_delete(cmd);

// 	vTaskDelete(NULL);
// }

void ssd1306_clear_display(ssd1306_t *device)
{
	for (uint8_t page = 0; page < MAX_PAGES; page++)
	{
		i2c_clear_write_buffer();
		i2c_write_byte(OLED_CONTROL_BYTE_CMD);
		i2c_write_byte(OLED_SET_PAGE_ADDRESS | page);
		i2c_write_byte(OLED_CONTROL_BYTE_CMD);
		i2c_write_byte(OLED_SET_LWR_COLOUMN_START_ADDR);
		i2c_write_byte(OLED_CONTROL_BYTE_CMD);
		i2c_write_byte(OLED_SET_UPR_COLOUMN_START_ADDR);
		i2c_write_byte(OLED_CONTROL_BYTE_GDDRAM_DATA_STREAM);
		i2c_write_zero(128);
		i2c_transmit_write_buffer(device->slave_handle);
	}
}

void ssd1306_clear_line(ssd1306_t *device, uint8_t line)
{
	i2c_write_byte(OLED_CONTROL_BYTE_CMD_STREAM);
	i2c_write_byte(OLED_SET_PAGE_ADDRESS | line);
	i2c_write_byte(OLED_SET_LWR_COLOUMN_START_ADDR);
	i2c_write_byte(OLED_SET_UPR_COLOUMN_START_ADDR);
	i2c_write_byte(OLED_CONTROL_BYTE_GDDRAM_DATA_STREAM);
	i2c_write_zero(128);
	i2c_transmit_write_buffer(device->slave_handle);

	// i2c_cmd_handle_t cmd;

	// // Sets the cursor to start of appropriate line
	// cmd = i2c_cmd_link_create();
	// i2c_master_start(cmd);
	// i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	// i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
	// i2c_master_write_byte(cmd, 0xB0 | line, true);
	// i2c_master_stop(cmd);
	// i2c_master_cmd_begin(device->port, cmd, 10 / portTICK_PERIOD_MS);
	// i2c_cmd_link_delete(cmd);

	// // Writes the text to the line, clears the rest of the line
	// uint8_t zero[64] = {0};
	// for (uint8_t i = 0; i < MAX_CHARACTERS_PER_LINE; i++)
	// {
	// 	cmd = i2c_cmd_link_create();
	// 	i2c_master_start(cmd);
	// 	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	// 	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_GDDRAM_DATA_STREAM, true);
	// 	i2c_master_write(cmd, zero, 64, true);

	// 	i2c_master_stop(cmd);
	// 	i2c_master_cmd_begin(device->port, cmd, 10 / portTICK_PERIOD_MS);
	// 	i2c_cmd_link_delete(cmd);
	// }
}

esp_err_t ssd1306_print_text_on_line(ssd1306_t *device, const char *text, uint8_t line)
{
	size_t text_len = strlen(text);
	ESP_RETURN_ON_FALSE(text_len <= MAX_CHARACTERS_PER_LINE, ESP_ERR_INVALID_ARG, SSD1306_TAG, "More characters than can fit on one line");
	ESP_RETURN_ON_FALSE(line < MAX_LINES, ESP_ERR_INVALID_ARG, SSD1306_TAG, "Invalid line number");

	i2c_clear_write_buffer();
	i2c_write_byte(OLED_CONTROL_BYTE_CMD);
	i2c_write_byte(OLED_SET_PAGE_ADDRESS | line);
	i2c_write_byte(OLED_CONTROL_BYTE_CMD);
	i2c_write_byte(OLED_SET_LWR_COLOUMN_START_ADDR);
	i2c_write_byte(OLED_CONTROL_BYTE_CMD);
	i2c_write_byte(OLED_SET_UPR_COLOUMN_START_ADDR);
	i2c_write_byte(OLED_CONTROL_BYTE_GDDRAM_DATA_STREAM);

	uint8_t zero[8] = {0};
	for (uint8_t i = 0; i < MAX_CHARACTERS_PER_LINE; i++)
	{
		if (i < text_len)
		{
			i2c_write_bytes((uint8_t *)(font8x8_basic[(uint8_t)text[i]]), 8);
		}
		else
		{
			i2c_write_bytes(zero, 8); // Writes 8x8 bits, 1 char
		}
	}

	i2c_transmit_write_buffer(device->slave_handle);
	return ESP_OK;
}

esp_err_t ssd1306_print_8x8basic(ssd1306_t *device, const char character, uint8_t line, uint8_t col)
{
	ESP_RETURN_ON_FALSE(line < MAX_LINES, ESP_ERR_INVALID_ARG, SSD1306_TAG, "Invalid line number");

	i2c_clear_write_buffer();
	i2c_write_byte(OLED_CONTROL_BYTE_CMD);
	i2c_write_byte(OLED_SET_PAGE_ADDRESS | line);
	i2c_write_byte(OLED_CONTROL_BYTE_CMD);
	i2c_write_byte(OLED_SET_LWR_COLOUMN_START_ADDR | (col & 0x0F));
	i2c_write_byte(OLED_CONTROL_BYTE_CMD);
	i2c_write_byte(OLED_SET_UPR_COLOUMN_START_ADDR | ((col & 0xF0) >> 4));
	i2c_write_byte(OLED_CONTROL_BYTE_GDDRAM_DATA_STREAM);
	i2c_write_bytes((uint8_t *)(font8x8_basic[(uint8_t)character]), 8);

	i2c_transmit_write_buffer(device->slave_handle);

	return ESP_OK;
}

esp_err_t gesp_ssd1306_init(i2c_master_bus_handle_t master_bus, ssd1306_t *ret_ssd1306_device)
{
	esp_err_t esp_rc;

	// Adding SSD1306 device to the I2C bus
	i2c_master_dev_handle_t slave_handle;
	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = OLED_I2C_ADDRESS,
		.scl_speed_hz = 400000,
	};
	ESP_ERROR_CHECK(i2c_master_bus_add_device(master_bus, &dev_cfg, &slave_handle));

	ret_ssd1306_device->bus = master_bus;
	ret_ssd1306_device->slave_handle = slave_handle;
	ret_ssd1306_device->clear_display = ssd1306_clear_display;
	ret_ssd1306_device->clear_line = ssd1306_clear_line;
	ret_ssd1306_device->print_text_on_line = ssd1306_print_text_on_line;
	ret_ssd1306_device->print_8x8basic = ssd1306_print_8x8basic;

	i2c_clear_write_buffer();
	i2c_write_byte(OLED_CONTROL_BYTE_CMD_STREAM);

	i2c_write_byte(OLED_CMD_SET_PAGE_ADDR_MODE);
	i2c_write_byte(OLED_SET_LWR_COLOUMN_START_ADDR);
	i2c_write_byte(OLED_SET_UPR_COLOUMN_START_ADDR);

	i2c_write_byte(OLED_CMD_SET_MUX_RATIO); // 1
	i2c_write_byte(0x3F);

	i2c_write_byte(OLED_CMD_SET_DISPLAY_OFFSET); // 2
	i2c_write_byte(0x00);

	i2c_write_byte(OLED_CMD_SET_DISPLAY_START_LINE); // 3

	i2c_write_byte(OLED_CMD_SET_SEGMENT_REMAP); // 4
	i2c_write_byte(OLED_CMD_SET_COM_SCAN_MODE); // 5

	i2c_write_byte(OLED_CMD_SET_CHARGE_PUMP);
	i2c_write_byte(OLED_CMD_CHARGE_PUMP_ON);

	i2c_write_byte(OLED_CMD_DISPLAY_ON);
	esp_rc = i2c_transmit_write_buffer(slave_handle);

	if (esp_rc == ESP_OK)
	{
		ESP_LOGI(SSD1306_TAG, "OLED configured successfully");
	}
	else
	{
		ESP_LOGE(SSD1306_TAG, "OLED configuration failed. code: 0x%.2X", esp_rc);
	}
	return esp_rc;
}
