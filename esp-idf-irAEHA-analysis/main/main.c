/* AEHA remote infrared RMT example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "ir_tools.h"

static const char *TAG = "AEHA";

static rmt_channel_t ir_rx_channel = RMT_CHANNEL_0;

/**
 * @brief RMT Receive Task
 *
 */
static void example_ir_rx_task(void *arg)
{
	int8_t ndata = 0;
	uint8_t data[10];
	size_t length = 0;
	bool repeat = false;
	RingbufHandle_t rb = NULL;
	rmt_item32_t *items = NULL;

	rmt_config_t rmt_rx_config = RMT_DEFAULT_CONFIG_RX(CONFIG_EXAMPLE_RMT_RX_GPIO, ir_rx_channel);
	rmt_config(&rmt_rx_config);
	rmt_driver_install(ir_rx_channel, 1000, 0);
	ir_parser_config_t ir_parser_config = IR_PARSER_DEFAULT_CONFIG((ir_dev_t)ir_rx_channel);
	//ir_parser_config.flags |= IR_TOOLS_FLAGS_PROTO_EXT; // Using extended IR protocols (RC5 have extended version)
	ir_parser_t *ir_parser = NULL;
	ir_parser = ir_parser_rmt_new_aeha(&ir_parser_config);

	//get RMT RX ringbuffer
	rmt_get_ringbuf_handle(ir_rx_channel, &rb);
	assert(rb != NULL);
	// Start receive
	rmt_rx_start(ir_rx_channel, true);
	while (1) {
		items = (rmt_item32_t *) xRingbufferReceive(rb, &length, portMAX_DELAY);
		if (items) {
			length /= 4; // one RMT = 4 Bytes
			if (ir_parser->input(ir_parser, items, length) == ESP_OK) {
				if (ir_parser->get_scan_code(ir_parser, length, &ndata, data, &repeat) == ESP_OK) {
					if (repeat) {
                        ESP_LOGI(TAG, "Scan success (repeat)");
                    } else {
                        ESP_LOGI(TAG, "Scan success");
                    }
					for(int i=0;i<ndata;i++) {
						ESP_LOGI(TAG, "Scan Data[%d]: 0x%02x", i, data[i]);
					}
				}
			}
			//after parsing the data, return spaces to ringbuffer.
			vRingbufferReturnItem(rb, (void *) items);
		}
	}

	// Never reach here
	ir_parser->del(ir_parser);
	rmt_driver_uninstall(ir_rx_channel);
	vTaskDelete(NULL);
}

#if 0
/**
 * @brief RMT Transmit Task
 *
 * Not use in this project
 */
static void example_ir_tx_task(void *arg)
{
	uint32_t customer = 0x10;
	uint32_t cmd = 0x20;
	rmt_item32_t *items = NULL;
	size_t length = 0;
	ir_builder_t *ir_builder = NULL;

	rmt_config_t rmt_tx_config = RMT_DEFAULT_CONFIG_TX(CONFIG_EXAMPLE_RMT_TX_GPIO, example_tx_channel);
	rmt_tx_config.tx_config.carrier_en = true;
	rmt_config(&rmt_tx_config);
	rmt_driver_install(example_tx_channel, 0, 0);
	ir_builder_config_t ir_builder_config = IR_BUILDER_DEFAULT_CONFIG((ir_dev_t)example_tx_channel);
	//ir_builder_config.flags |= IR_TOOLS_FLAGS_PROTO_EXT; // Using extended IR protocols (RC5 have extended version)
	ir_builder = ir_builder_rmt_new_aeha(&ir_builder_config);
	while (1) {
		vTaskDelay(pdMS_TO_TICKS(2000));
		ESP_LOGI(TAG, "Send command 0x%x to customer 0x%x", cmd, customer);
		// Send new key code
		ESP_ERROR_CHECK(ir_builder->build_frame(ir_builder, customer, cmd));
		ESP_ERROR_CHECK(ir_builder->get_result(ir_builder, &items, &length));
		//To send data according to the waveform items.
		rmt_write_items(example_tx_channel, items, length, false);
		// Send repeat code
		vTaskDelay(pdMS_TO_TICKS(ir_builder->repeat_period_ms));
		ESP_ERROR_CHECK(ir_builder->build_repeat_frame(ir_builder));
		ESP_ERROR_CHECK(ir_builder->get_result(ir_builder, &items, &length));
		rmt_write_items(example_tx_channel, items, length, false);
		cmd++;
	}
	ir_builder->del(ir_builder);
	rmt_driver_uninstall(example_tx_channel);
	vTaskDelete(NULL);
}
#endif

void app_main(void)
{
	xTaskCreate(example_ir_rx_task, "ir_rx_task", 2048, NULL, 10, NULL);
#if 0
	xTaskCreate(example_ir_tx_task, "ir_tx_task", 2048, NULL, 10, NULL);
#endif
}
