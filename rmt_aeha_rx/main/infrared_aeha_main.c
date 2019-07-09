/* AEHA remote infrared RMT example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/rmt.h"

#include "ir_aeha.h"

static const char* TAG = "AEHA";

#define RMT_TX_CHANNEL    1     /*!< RMT channel for transmitter */
#define RMT_TX_GPIO_NUM  17     /*!< GPIO number for transmitter signal */
#define RMT_RX_CHANNEL    0     /*!< RMT channel for receiver */
#define RMT_RX_GPIO_NUM  19     /*!< GPIO number for receiver */


/**
 * @brief RMT receiver demo, this task will print each received AEHA data.
 *
 */
static void rmt_example_aeha_rx_task()
{
    int channel = RMT_RX_CHANNEL;
    aeha_rx_init(RMT_RX_CHANNEL, RMT_RX_GPIO_NUM);
    RingbufHandle_t rb = NULL;
    //get RMT RX ringbuffer
    rmt_get_ringbuf_handle(channel, &rb);
    rmt_rx_start(channel, 1);
    while(rb) {
        size_t rx_size = 0;
        //try to receive data from ringbuffer.
        //RMT driver will push all the data it receives to its ringbuffer.
        //We just need to parse the value and return the spaces of ringbuffer.
        rmt_item32_t* item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, 1000);
        ESP_LOGI(TAG, "xRingbufferReceive rx_size=%d", rx_size);
        if(item) {
            ESP_LOGI(TAG, "RMT RCV rx_size=%d", rx_size);
            uint16_t customer;
            uint8_t parity;
            uint8_t data[32];
            int offset = 0;
            int index = 0;
            while(1) {
                //parse data value from ringbuffer.
                //int res = aeha_parse_items(item + offset, rx_size / 4 - offset, &customer, data);
                //int res = aeha_parse_items(item + offset, rx_size / sizeof(rmt_item32_t) - offset, &customer, &parity, &index, data);
                int item_num = rx_size / sizeof(rmt_item32_t) - offset;
                int res = aeha_parse_items(item + offset, item_num, &customer, &parity, &index, data);
                ESP_LOGI(TAG, "RMT RCV offset=%d item_num=%d res=%d", offset, item_num, res);
                if(res > 0) {
                    offset += res + 1;
                    ESP_LOGI(TAG, "RMT RCV --- customer: 0x%04x parity: 0x%02x index: %d", customer, parity, index);
                    for(int i=0; i<index; i++) {
                        ESP_LOGI(TAG, "RMT RCV --- data[%d]: 0x%02x", i, data[i]);
                    }
                } else {
                    break;
                }
            }
            //after parsing the data, return spaces to ringbuffer.
            ESP_LOGI(TAG, "vRingbufferReturnItem");
            vRingbufferReturnItem(rb, (void*) item);
#if 0
        } else {
            break;
#endif
        }
    }
    vTaskDelete(NULL);
}

/**
 * @brief RMT transmitter demo, this task will periodically send AEHA data.
 *
 */
static void rmt_example_aeha_tx_task()
{
    vTaskDelay(10);
    aeha_tx_init(RMT_TX_CHANNEL, RMT_TX_GPIO_NUM);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    int channel = RMT_TX_CHANNEL;
    uint16_t customer = 0x2002;
    uint8_t parity = 0x00;
    uint8_t data[10];
    data[0] = 0x08;
    data[1] = 0x00;
    data[2] = 0x3d;
    data[3] = 0xbd;
    int data_num = 4;
    //int nec_tx_num = RMT_TX_DATA_NUM;
    for(;;) {
        ESP_LOGI(TAG, "RMT TX DATA");
        size_t size = (sizeof(rmt_item32_t) * (AEHA_DATA_ITEM_NUM + (data_num*8)));
        //each item represent a cycle of waveform.
        rmt_item32_t* item = (rmt_item32_t*) malloc(size);
        //int item_num = NEC_DATA_ITEM_NUM * nec_tx_num;
        memset((void*) item, 0, size);
        //int i, offset = 0;

        //To build a series of waveforms.
        int item_num = aeha_build_items(channel, item, data_num, customer, parity, data);
        //To send data according to the waveform items.
        rmt_write_items(channel, item, item_num, true);
        //Wait until sending is done.
        rmt_wait_tx_done(channel, portMAX_DELAY);
        //before we free the data, make sure sending is already done.
        free(item);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    vTaskDelete(NULL);
}

void app_main()
{
    xTaskCreate(rmt_example_aeha_rx_task, "rmt_rx_task", 2048, NULL, 10, NULL);
    //xTaskCreate(rmt_example_aeha_tx_task, "rmt_tx_task", 2048, NULL, 10, NULL);
}
