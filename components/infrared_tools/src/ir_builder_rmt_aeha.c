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
// limitations under the License.#include <stdlib.h>
#include <inttypes.h>
#include <sys/cdefs.h>
#include "esp_log.h"
#include "ir_tools.h"
#include "ir_timings.h"
#include "driver/rmt.h"

static const char *TAG = "aeha_builder";
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

typedef struct {
    ir_builder_t parent;
    uint32_t buffer_size;
    uint32_t cursor;
    uint32_t flags;
    uint32_t leading_code_high_ticks;
    uint32_t leading_code_low_ticks;
    uint32_t repeat_code_high_ticks;
    uint32_t repeat_code_low_ticks;
    uint32_t payload_logic0_high_ticks;
    uint32_t payload_logic0_low_ticks;
    uint32_t payload_logic1_high_ticks;
    uint32_t payload_logic1_low_ticks;
    uint32_t ending_code_high_ticks;
    uint32_t ending_code_low_ticks;
    bool inverse;
    rmt_item32_t buffer[0];
} aeha_builder_t;

static esp_err_t aeha_builder_make_head(ir_builder_t *builder)
{
    aeha_builder_t *aeha_builder = __containerof(builder, aeha_builder_t, parent);
    aeha_builder->cursor = 0;
    aeha_builder->buffer[aeha_builder->cursor].level0 = !aeha_builder->inverse;
    aeha_builder->buffer[aeha_builder->cursor].duration0 = aeha_builder->leading_code_high_ticks;
    aeha_builder->buffer[aeha_builder->cursor].level1 = aeha_builder->inverse;
    aeha_builder->buffer[aeha_builder->cursor].duration1 = aeha_builder->leading_code_low_ticks;
    aeha_builder->cursor += 1;
    return ESP_OK;
}

static esp_err_t aeha_builder_make_logic0(ir_builder_t *builder)
{
    aeha_builder_t *aeha_builder = __containerof(builder, aeha_builder_t, parent);
    aeha_builder->buffer[aeha_builder->cursor].level0 = !aeha_builder->inverse;
    aeha_builder->buffer[aeha_builder->cursor].duration0 = aeha_builder->payload_logic0_high_ticks;
    aeha_builder->buffer[aeha_builder->cursor].level1 = aeha_builder->inverse;
    aeha_builder->buffer[aeha_builder->cursor].duration1 = aeha_builder->payload_logic0_low_ticks;
    aeha_builder->cursor += 1;
    return ESP_OK;
}

static esp_err_t aeha_builder_make_logic1(ir_builder_t *builder)
{
    aeha_builder_t *aeha_builder = __containerof(builder, aeha_builder_t, parent);
    aeha_builder->buffer[aeha_builder->cursor].level0 = !aeha_builder->inverse;
    aeha_builder->buffer[aeha_builder->cursor].duration0 = aeha_builder->payload_logic1_high_ticks;
    aeha_builder->buffer[aeha_builder->cursor].level1 = aeha_builder->inverse;
    aeha_builder->buffer[aeha_builder->cursor].duration1 = aeha_builder->payload_logic1_low_ticks;
    aeha_builder->cursor += 1;
    return ESP_OK;
}

static esp_err_t aeha_builder_make_end(ir_builder_t *builder)
{
    aeha_builder_t *aeha_builder = __containerof(builder, aeha_builder_t, parent);
    aeha_builder->buffer[aeha_builder->cursor].level0 = !aeha_builder->inverse;
    aeha_builder->buffer[aeha_builder->cursor].duration0 = aeha_builder->ending_code_high_ticks;
    aeha_builder->buffer[aeha_builder->cursor].level1 = aeha_builder->inverse;
    aeha_builder->buffer[aeha_builder->cursor].duration1 = aeha_builder->ending_code_low_ticks;
    aeha_builder->cursor += 1;
    aeha_builder->buffer[aeha_builder->cursor].val = 0;
    aeha_builder->cursor += 1;
    return ESP_OK;
}

static esp_err_t aeha_build_frame(ir_builder_t *builder, int8_t ndata, uint8_t *data)
{
    //aeha_builder_t *aeha_builder = __containerof(builder, aeha_builder_t, parent);
    builder->make_head(builder);
    for (int d = 0; d < ndata; d++) {
	    ESP_LOGD(__FUNCTION__, "data[%d]=0x%02x", d, data[d]);
        for (int i = 0; i < 8; i++) {
            if (data[d] & (1 << i)) {
                builder->make_logic1(builder);
            } else {
                builder->make_logic0(builder);
            }
        }
    }
    builder->make_end(builder);
    return ESP_OK;
}

static esp_err_t aeha_build_repeat_frame(ir_builder_t *builder)
{
    aeha_builder_t *aeha_builder = __containerof(builder, aeha_builder_t, parent);
    aeha_builder->cursor = 0;
    aeha_builder->buffer[aeha_builder->cursor].level0 = !aeha_builder->inverse;
    aeha_builder->buffer[aeha_builder->cursor].duration0 = aeha_builder->repeat_code_high_ticks;
    aeha_builder->buffer[aeha_builder->cursor].level1 = aeha_builder->inverse;
    aeha_builder->buffer[aeha_builder->cursor].duration1 = aeha_builder->repeat_code_low_ticks;
    aeha_builder->cursor += 1;
    aeha_builder_make_end(builder);
    return ESP_OK;
}

static esp_err_t aeha_builder_get_result(ir_builder_t *builder, void *result, size_t *length)
{
    esp_err_t ret = ESP_OK;
    aeha_builder_t *aeha_builder = __containerof(builder, aeha_builder_t, parent);
    AEHA_CHECK(result && length, "result and length can't be null", err, ESP_ERR_INVALID_ARG);
    *(rmt_item32_t **)result = aeha_builder->buffer;
    *length = aeha_builder->cursor;
    return ESP_OK;
err:
    return ret;
}

static esp_err_t aeha_builder_del(ir_builder_t *builder)
{
    aeha_builder_t *aeha_builder = __containerof(builder, aeha_builder_t, parent);
    free(aeha_builder);
    return ESP_OK;
}

ir_builder_t *ir_builder_rmt_new_aeha(const ir_builder_config_t *config)
{
    ir_builder_t *ret = NULL;
    AEHA_CHECK(config, "aeha configuration can't be null", err, NULL);
    AEHA_CHECK(config->buffer_size, "buffer size can't be zero", err, NULL);
    ESP_LOGI(__FUNCTION__, "config->buffer_size=%"PRIu32, config->buffer_size);

    uint32_t builder_size = sizeof(aeha_builder_t) + config->buffer_size * sizeof(rmt_item32_t);
    aeha_builder_t *aeha_builder = calloc(1, builder_size);
    AEHA_CHECK(aeha_builder, "request memory for aeha_builder failed", err, NULL);

    aeha_builder->buffer_size = config->buffer_size;
    aeha_builder->flags = config->flags;
    if (config->flags & IR_TOOLS_FLAGS_INVERSE) {
        aeha_builder->inverse = true;
    }

    uint32_t counter_clk_hz = 0;
    AEHA_CHECK(rmt_get_counter_clock((rmt_channel_t)config->dev_hdl, &counter_clk_hz) == ESP_OK,
              "get rmt counter clock failed", err, NULL);
    float ratio = (float)counter_clk_hz / 1e6;
    aeha_builder->leading_code_high_ticks = (uint32_t)(ratio * AEHA_LEADING_CODE_HIGH_US);
    aeha_builder->leading_code_low_ticks = (uint32_t)(ratio * AEHA_LEADING_CODE_LOW_US);
    aeha_builder->repeat_code_high_ticks = (uint32_t)(ratio * AEHA_REPEAT_CODE_HIGH_US);
    aeha_builder->repeat_code_low_ticks = (uint32_t)(ratio * AEHA_REPEAT_CODE_LOW_US);
    aeha_builder->payload_logic0_high_ticks = (uint32_t)(ratio * AEHA_PAYLOAD_ZERO_HIGH_US);
    aeha_builder->payload_logic0_low_ticks = (uint32_t)(ratio * AEHA_PAYLOAD_ZERO_LOW_US);
    aeha_builder->payload_logic1_high_ticks = (uint32_t)(ratio * AEHA_PAYLOAD_ONE_HIGH_US);
    aeha_builder->payload_logic1_low_ticks = (uint32_t)(ratio * AEHA_PAYLOAD_ONE_LOW_US);
    aeha_builder->ending_code_high_ticks = (uint32_t)(ratio * AEHA_ENDING_CODE_HIGH_US);
    aeha_builder->ending_code_low_ticks = 0x7FFF;
    aeha_builder->parent.make_head = aeha_builder_make_head;
    aeha_builder->parent.make_logic0 = aeha_builder_make_logic0;
    aeha_builder->parent.make_logic1 = aeha_builder_make_logic1;
    aeha_builder->parent.make_end = aeha_builder_make_end;
    aeha_builder->parent.build_frame = aeha_build_frame;
    aeha_builder->parent.build_repeat_frame = aeha_build_repeat_frame;
    aeha_builder->parent.get_result = aeha_builder_get_result;
    aeha_builder->parent.del = aeha_builder_del;
    aeha_builder->parent.repeat_period_ms = 110;
    return &aeha_builder->parent;
err:
    return ret;
}
