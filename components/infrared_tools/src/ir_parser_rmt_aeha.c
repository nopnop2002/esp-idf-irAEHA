// Copyright 2019 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <inttypes.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include "esp_log.h"
#include "ir_tools.h"
#include "ir_timings.h"
#include "driver/rmt.h"

static const char *TAG = "aeha_parser";
#define AEHA_CHECK(a, str, goto_tag, ret_value, ...)                               \
    do                                                                            \
    {                                                                             \
        if (!(a))                                                                 \
        {                                                                         \
            ESP_LOGE(TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            ret = ret_value;                                                      \
            goto goto_tag;                                                        \
        }                                                                         \
    } while (0)

#define AEHA_DATA_FRAME_RMT_WORDS (34) // Not used in AEHA format due to variable length
#define AEHA_REPEAT_FRAME_RMT_WORDS (2)
#define AEHA_DATA_FRAME_MAX_BYTES (10)

typedef struct {
    ir_parser_t parent;
    uint32_t flags;
    uint32_t leading_code_high_ticks;
    uint32_t leading_code_low_ticks;
    uint32_t repeat_code_high_ticks;
    uint32_t repeat_code_low_ticks;
    uint32_t payload_logic0_high_ticks;
    uint32_t payload_logic0_low_ticks;
    uint32_t payload_logic1_high_ticks;
    uint32_t payload_logic1_low_ticks;
    uint32_t margin_ticks;
    rmt_item32_t *buffer;
    uint32_t cursor;
    int8_t last_ndata;
    uint8_t last_data[AEHA_DATA_FRAME_MAX_BYTES];
    bool repeat;
    bool inverse;
} aeha_parser_t;

static inline bool aeha_check_in_range(uint32_t raw_ticks, uint32_t target_ticks, uint32_t margin_ticks)
{
    return (raw_ticks < (target_ticks + margin_ticks)) && (raw_ticks > (target_ticks - margin_ticks));
}

static bool aeha_parse_head(aeha_parser_t *aeha_parser)
{
	ESP_LOGD(__FUNCTION__, "Start");
    aeha_parser->cursor = 0;
    rmt_item32_t item = aeha_parser->buffer[aeha_parser->cursor];
    bool ret = (item.level0 == aeha_parser->inverse) && (item.level1 != aeha_parser->inverse) &&
               aeha_check_in_range(item.duration0, aeha_parser->leading_code_high_ticks, aeha_parser->margin_ticks) &&
               aeha_check_in_range(item.duration1, aeha_parser->leading_code_low_ticks, aeha_parser->margin_ticks);
    aeha_parser->cursor += 1;
    return ret;
}

static bool aeha_parse_logic0(aeha_parser_t *aeha_parser)
{
	ESP_LOGD(__FUNCTION__, "Start");
    rmt_item32_t item = aeha_parser->buffer[aeha_parser->cursor];
    bool ret = (item.level0 == aeha_parser->inverse) && (item.level1 != aeha_parser->inverse) &&
               aeha_check_in_range(item.duration0, aeha_parser->payload_logic0_high_ticks, aeha_parser->margin_ticks) &&
               aeha_check_in_range(item.duration1, aeha_parser->payload_logic0_low_ticks, aeha_parser->margin_ticks);
    return ret;
}

static bool aeha_parse_logic1(aeha_parser_t *aeha_parser)
{
	ESP_LOGD(__FUNCTION__, "Start");
    rmt_item32_t item = aeha_parser->buffer[aeha_parser->cursor];
    bool ret = (item.level0 == aeha_parser->inverse) && (item.level1 != aeha_parser->inverse) &&
               aeha_check_in_range(item.duration0, aeha_parser->payload_logic1_high_ticks, aeha_parser->margin_ticks) &&
               aeha_check_in_range(item.duration1, aeha_parser->payload_logic1_low_ticks, aeha_parser->margin_ticks);
    return ret;
}

static esp_err_t aeha_parse_logic(ir_parser_t *parser, bool *logic)
{
	ESP_LOGD(__FUNCTION__, "Start");
    esp_err_t ret = ESP_FAIL;
    bool logic_value = false;
    aeha_parser_t *aeha_parser = __containerof(parser, aeha_parser_t, parent);
    if (aeha_parse_logic0(aeha_parser)) {
        logic_value = false;
        ret = ESP_OK;
    } else if (aeha_parse_logic1(aeha_parser)) {
        logic_value = true;
        ret = ESP_OK;
    }
    if (ret == ESP_OK) {
        *logic = logic_value;
    }
    aeha_parser->cursor += 1;
    return ret;
}

static bool aeha_parse_repeat_frame(aeha_parser_t *aeha_parser)
{
	ESP_LOGD(__FUNCTION__, "Start");
    aeha_parser->cursor = 0;
    rmt_item32_t item = aeha_parser->buffer[aeha_parser->cursor];
    bool ret = (item.level0 == aeha_parser->inverse) && (item.level1 != aeha_parser->inverse) &&
               aeha_check_in_range(item.duration0, aeha_parser->repeat_code_high_ticks, aeha_parser->margin_ticks) &&
               aeha_check_in_range(item.duration1, aeha_parser->repeat_code_low_ticks, aeha_parser->margin_ticks);
    aeha_parser->cursor += 1;
    return ret;
}

static esp_err_t aeha_parser_input(ir_parser_t *parser, void *raw_data, uint32_t length)
{
	ESP_LOGD(__FUNCTION__, "Start");
    esp_err_t ret = ESP_OK;
    aeha_parser_t *aeha_parser = __containerof(parser, aeha_parser_t, parent);
    AEHA_CHECK(raw_data, "input data can't be null", err, ESP_ERR_INVALID_ARG);
    aeha_parser->buffer = raw_data;
	ESP_LOGD(__FUNCTION__, "length=%"PRIu32, length);
    // Data Frame costs 2+16*n items and Repeat Frame costs 2 items
    aeha_parser->repeat = false;
    if (length == AEHA_REPEAT_FRAME_RMT_WORDS) {
        aeha_parser->repeat = true;
    }
#if 0
    // Data Frame costs 34 items and Repeat Frame costs 2 items
    if (length == AEHA_DATA_FRAME_RMT_WORDS) {
        aeha_parser->repeat = false;
    } else if (length == AEHA_REPEAT_FRAME_RMT_WORDS) {
        aeha_parser->repeat = true;
    } else {
        ret = ESP_FAIL;
    }
#endif
    return ret;
err:
    return ret;
}

static esp_err_t aeha_parser_get_scan_code(ir_parser_t *parser, uint32_t length, int8_t *ndata, uint8_t *data, bool *repeat)
{
    esp_err_t ret = ESP_FAIL;
    bool logic_value = false;
    aeha_parser_t *aeha_parser = __containerof(parser, aeha_parser_t, parent);
    AEHA_CHECK(length && ndata && repeat, "length, ndata and repeat can't be null", out, ESP_ERR_INVALID_ARG);
	ESP_LOGD(__FUNCTION__, "length=%"PRIu32, length);
    if (aeha_parser->repeat) {
        if (aeha_parse_repeat_frame(aeha_parser)) {
            *ndata = aeha_parser->last_ndata;
            for(int d = 0; d < *ndata; d++) {
                data[d] = aeha_parser->last_data[d];
            }
            *repeat = true;
            ret = ESP_OK;
        }
    } else {
        if (aeha_parse_head(aeha_parser)) {
            *ndata = (length - 2) / 8; // header(2T) + customer(16T) + parity(8T) + data(8T*n)
            if (*ndata > AEHA_DATA_FRAME_MAX_BYTES) {
                ESP_LOGE(TAG, "Scan code is tool long(%d): ", *ndata);
                goto out;
            }
            for (int d = 0; d < *ndata; d++) {
                data[d] = 0;
                for (int i = 0; i < 8; i++) {
                    if (aeha_parse_logic(parser, &logic_value) == ESP_OK) {
                        data[d] |= (logic_value << i);
                    }
                }
                // keep it as potential repeat code
                aeha_parser->last_data[d] = data[d];
            }
            *repeat = false;
            ret = ESP_OK;
        }
    }
out:
    return ret;
}

static esp_err_t aeha_parser_del(ir_parser_t *parser)
{
    aeha_parser_t *aeha_parser = __containerof(parser, aeha_parser_t, parent);
    free(aeha_parser);
    return ESP_OK;
}

ir_parser_t *ir_parser_rmt_new_aeha(const ir_parser_config_t *config)
{
    ir_parser_t *ret = NULL;
    AEHA_CHECK(config, "aeha configuration can't be null", err, NULL);

    aeha_parser_t *aeha_parser = calloc(1, sizeof(aeha_parser_t));
    AEHA_CHECK(aeha_parser, "request memory for aeha_parser failed", err, NULL);

    aeha_parser->flags = config->flags;
    if (config->flags & IR_TOOLS_FLAGS_INVERSE) {
        aeha_parser->inverse = true;
    }

    uint32_t counter_clk_hz = 0;
    AEHA_CHECK(rmt_get_counter_clock((rmt_channel_t)config->dev_hdl, &counter_clk_hz) == ESP_OK,
              "get rmt counter clock failed", err, NULL);
    float ratio = (float)counter_clk_hz / 1e6;
    aeha_parser->leading_code_high_ticks = (uint32_t)(ratio * AEHA_LEADING_CODE_HIGH_US);
    aeha_parser->leading_code_low_ticks = (uint32_t)(ratio * AEHA_LEADING_CODE_LOW_US);
    aeha_parser->repeat_code_high_ticks = (uint32_t)(ratio * AEHA_REPEAT_CODE_HIGH_US);
    aeha_parser->repeat_code_low_ticks = (uint32_t)(ratio * AEHA_REPEAT_CODE_LOW_US);
    aeha_parser->payload_logic0_high_ticks = (uint32_t)(ratio * AEHA_PAYLOAD_ZERO_HIGH_US);
    aeha_parser->payload_logic0_low_ticks = (uint32_t)(ratio * AEHA_PAYLOAD_ZERO_LOW_US);
    aeha_parser->payload_logic1_high_ticks = (uint32_t)(ratio * AEHA_PAYLOAD_ONE_HIGH_US);
    aeha_parser->payload_logic1_low_ticks = (uint32_t)(ratio * AEHA_PAYLOAD_ONE_LOW_US);
    aeha_parser->margin_ticks = (uint32_t)(ratio * config->margin_us);
    aeha_parser->parent.input = aeha_parser_input;
    aeha_parser->parent.get_scan_code = aeha_parser_get_scan_code;
    aeha_parser->parent.del = aeha_parser_del;
    return &aeha_parser->parent;
err:
    return ret;
}
