#include "font8x8_basic.h"
#include "esp-ssd1306.h"

void ssd1306_main(void *pvParameters)
{
	while (1)
	{
		// Loop forever
	}

	vTaskDelete(NULL);
}

// For standalone use, call this function to initialize the task.
TaskHandle_t ssd1306_task_init()
{
	TaskHandle_t handle;
	// 0 is lowest priority
	xTaskCreate(ssd1306_main, "ssd1306_main", 4096, NULL, 1, &handle);
	return handle;
}

void ssd1306_init()
{
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

	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
	if (espRc == ESP_OK)
	{
		ESP_LOGI(SSD1306_TAG, "OLED configured successfully");
	}
	else
	{
		ESP_LOGE(SSD1306_TAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
}

void task_ssd1306_display_text(void *pvParameters)
{
	char *text = "Hello world!\nGood Stuff!\n";
	uint8_t text_len = strlen(text);

	i2c_cmd_handle_t cmd;

	uint8_t cur_page = 0;

	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
	i2c_master_write_byte(cmd, 0x00, true); // reset column
	i2c_master_write_byte(cmd, 0x10, true);
	i2c_master_write_byte(cmd, 0xB0 | cur_page, true); // reset page

	i2c_master_stop(cmd);
	i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
	i2c_cmd_link_delete(cmd);

	for (uint8_t i = 0; i < text_len; i++)
	{
		if (text[i] == '\n')
		{
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
			i2c_master_write_byte(cmd, 0x00, true); // reset column
			i2c_master_write_byte(cmd, 0x10, true);
			i2c_master_write_byte(cmd, 0xB0 | ++cur_page, true); // increment page

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
		else
		{
			cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);

			i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
			i2c_master_write(cmd, (uint8_t *)(font8x8_basic[(uint8_t)text[i]]), 8, true);

			i2c_master_stop(cmd);
			i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
	}

	vTaskDelete(NULL);
}

void task_ssd1306_display_pattern(void *ignore)
{
	i2c_cmd_handle_t cmd;

	for (uint8_t i = 0; i < 8; i++)
	{
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		for (uint8_t j = 0; j < 128; j++)
		{
			i2c_master_write_byte(cmd, 0xFF >> (j % 8), true);
		}
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}

	vTaskDelete(NULL);
}

void ssd1306_clear_display(ssd1306_t *device)
{
	i2c_cmd_handle_t cmd;

	uint8_t zero[128] = {0};
	for (uint8_t i = 0; i < MAX_PAGES; i++)
	{
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
		i2c_master_write_byte(cmd, 0xB0 | i, true);

		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
		i2c_master_write(cmd, zero, 128, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(device->port, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
}

void task_ssd1306_contrast(void *ignore)
{
	i2c_cmd_handle_t cmd;

	uint8_t contrast = 0;
	uint8_t direction = 1;
	while (true)
	{
		cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
		i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);
		i2c_master_write_byte(cmd, OLED_CMD_SET_CONTRAST, true);
		i2c_master_write_byte(cmd, contrast, true);
		i2c_master_stop(cmd);
		i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		vTaskDelay(1 / portTICK_PERIOD_MS);

		contrast += direction;
		if (contrast == 0xFF)
		{
			direction = -1;
		}
		if (contrast == 0x0)
		{
			direction = 1;
		}
	}
	vTaskDelete(NULL);
}

void task_ssd1306_scroll(void *ignore)
{
	esp_err_t espRc;

	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);

	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

	i2c_master_write_byte(cmd, 0x29, true); // vertical and horizontal scroll (p29)
	i2c_master_write_byte(cmd, 0x00, true);
	i2c_master_write_byte(cmd, 0x00, true);
	i2c_master_write_byte(cmd, 0x07, true);
	i2c_master_write_byte(cmd, 0x01, true);
	i2c_master_write_byte(cmd, 0x3F, true);

	i2c_master_write_byte(cmd, 0xA3, true); // set vertical scroll area (p30)
	i2c_master_write_byte(cmd, 0x20, true);
	i2c_master_write_byte(cmd, 0x40, true);

	i2c_master_write_byte(cmd, 0x2F, true); // activate scroll (p29)

	i2c_master_stop(cmd);
	espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
	if (espRc == ESP_OK)
	{
		ESP_LOGI(SSD1306_TAG, "Scroll command succeeded");
	}
	else
	{
		ESP_LOGE(SSD1306_TAG, "Scroll command failed. code: 0x%.2X", espRc);
	}

	i2c_cmd_link_delete(cmd);

	vTaskDelete(NULL);
}

void ssd1306_clear_line(ssd1306_t *device, line_num_t line)
{
	i2c_cmd_handle_t cmd;

	// Sets the cursor to start of appropriate line
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
	i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
	i2c_master_write_byte(cmd, 0xB0 | line, true);
	i2c_master_stop(cmd);
	i2c_master_cmd_begin(device->port, cmd, 10 / portTICK_PERIOD_MS);
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
		i2c_master_cmd_begin(device->port, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
	}
}

esp_err_t ssd1306_print_text_on_line(ssd1306_t *device, const char *text, line_num_t line)
{
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
	i2c_master_cmd_begin(device->port, cmd, 10 / portTICK_PERIOD_MS);
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
			i2c_master_cmd_begin(device->port, cmd, 10 / portTICK_PERIOD_MS);
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
			i2c_master_cmd_begin(device->port, cmd, 10 / portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
		}
	}
	return ESP_OK;
}

esp_err_t gesp_ssd1306_init(i2c_port_t port, ssd1306_t *ret_ssd1306_device)
{
	esp_err_t espRc = ESP_OK;
	ESP_RETURN_ON_FALSE(port < I2C_NUM_MAX, ESP_ERR_INVALID_ARG, SSD1306_TAG, "Invalid I2C port");

	ret_ssd1306_device->port = port;
	ret_ssd1306_device->clear_display = ssd1306_clear_display;
	ret_ssd1306_device->clear_line = ssd1306_clear_line;
	ret_ssd1306_device->print_text_on_line = ssd1306_print_text_on_line;

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
	{
		ESP_LOGI(SSD1306_TAG, "OLED configured successfully");
	}
	else
	{
		ESP_LOGE(SSD1306_TAG, "OLED configuration failed. code: 0x%.2X", espRc);
	}
	i2c_cmd_link_delete(cmd);
	return espRc;
}
