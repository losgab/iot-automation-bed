#include "gesp-ssd1306.h"
#include "font8x8_basic.h"

#ifdef __cplusplus
extern "C"
{
#endif

	SSD1306::SSD1306(i2c_port_t port)
	{
		ESP_LOGE(SSD1306_TAG, "Initialising SSD1306 OLED Display");

		esp_err_t espRc;
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();

		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

		i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
		i2c_master_write_byte(cmd, 0x14, true);

		i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
		i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

		i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
		i2c_master_stop(cmd);

		espRc = i2c_master_cmd_begin(port, cmd, 10 / portTICK_PERIOD_MS);
		if (espRc == ESP_OK)
			ESP_LOGI(SSD1306_TAG, "OLED configured successfully");
		else
			ESP_LOGE(SSD1306_TAG, "OLED configuration failed. code: 0x%.2X", espRc);
		i2c_cmd_link_delete(cmd);
	}

	esp_err_t SSD1306::print_text_on_line(const char *text, line_num_t line)
	{
		esp_err_t espRc;
		size_t text_len = strlen(text);
		ESP_RETURN_ON_FALSE(text_len <= MAX_CHARACTERS_PER_LINE, ESP_ERR_INVALID_ARG, SSD1306_TAG, "More characters than can fit on one line");
		ESP_RETURN_ON_FALSE(line < MAX_LINES, ESP_ERR_INVALID_ARG, SSD1306_TAG, "Invalid line number");
		i2c_cmd_handle_t cmd;

		// Sets the cursor to start of appropriate line
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | line, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(port, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);

		// Writes the text to the line, clears the rest of the line
		uint8_t zero[8] = {0};
		for (uint8_t i = 0; i < MAX_CHARACTERS_PER_LINE; i++)
		{
			if (i < text_len)
			{
				cmd = i2c_cmd_link_create();
				i2c_master_start(cmd);
				i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

				i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
				i2c_master_write(cmd, (uint8_t *)(font8x8_basic[(uint8_t)text[i]]), 8, true);

				i2c_master_stop(cmd);
				i2c_master_cmd_begin(port, cmd, 10 / portTICK_PERIOD_MS);
				i2c_cmd_link_delete(cmd);
			}
			else
			{
				cmd = i2c_cmd_link_create();
				i2c_master_start(cmd);
				i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

				i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
				i2c_master_write(cmd, zero, 8, true);

				i2c_master_stop(cmd);
				i2c_master_cmd_begin(port, cmd, 10 / portTICK_PERIOD_MS);
				i2c_cmd_link_delete(cmd);
			}
		}
		return ESP_OK;
	}

	esp_err_t SSD1306::clear_all()
	{
		for (uint8_t i = LINE_0; i < MAX_LINES; i++)
		{
			clear_line((line_num_t)i);
		}
		return ESP_OK;
	}

	esp_err_t SSD1306::clear_line(line_num_t line)
	{
		i2c_cmd_handle_t cmd;

		// Sets the cursor to start of appropriate line
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | line, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(port, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);

		// Writes the text to the line, clears the rest of the line
		uint8_t zero[64] = {0};
		for (uint8_t i = 0; i < MAX_CHARACTERS_PER_LINE; i++)
		{
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, zero, 64, true);

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(port, cmd, 10 / portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
		return ESP_OK;
	}

	SSD1306::~SSD1306()
	{
		ESP_LOGE(SSD1306_TAG, "SSD1306 destructor called");
	}

#ifdef __cplusplus
}
#endif
