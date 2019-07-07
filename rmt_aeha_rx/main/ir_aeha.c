#include "ir_aeha.h"

static const char* TAG = "AEHA";

/*
 * @brief Build register value of waveform for AEHA one data bit
 */
void aeha_fill_item_level(rmt_item32_t* item, int high_us, int low_us)
{
    item->level0 = 1;
    item->duration0 = (high_us) / 10 * RMT_TICK_10_US;
    item->level1 = 0;
    item->duration1 = (low_us) / 10 * RMT_TICK_10_US;
}

/*
 * @brief Generate AEHA header value: active 3.4ms + negative 1.7ms
 */
void aeha_fill_item_header(rmt_item32_t* item)
{
    aeha_fill_item_level(item, AEHA_HEADER_HIGH_US, AEHA_HEADER_LOW_US);
}

/*
 * @brief Generate AEHA data bit 1: positive 0.425ms + negative 1.275ms
 */
void aeha_fill_item_bit_one(rmt_item32_t* item)
{
    aeha_fill_item_level(item, AEHA_BIT_ONE_HIGH_US, AEHA_BIT_ONE_LOW_US);
}

/*
 * @brief Generate AEHA data bit 0: positive 0.425ms + negative 0.425ms
 */
void aeha_fill_item_bit_zero(rmt_item32_t* item)
{
    aeha_fill_item_level(item, AEHA_BIT_ZERO_HIGH_US, AEHA_BIT_ZERO_LOW_US);
}

/*
 * @brief Generate AEHA end signal: positive 0.425ms + negatiove 8ms
 */
void aeha_fill_item_end(rmt_item32_t* item)
{
    aeha_fill_item_level(item, AEHA_TRAILER_HIGH_US, AEHA_TRAILER_LOW_US);
}

/*
 * @brief Check whether duration is around target_us
 */
bool aeha_check_in_range(int duration_ticks, int target_us, int margin_us)
{
#if _DEBUG_
    uint16_t middle = AEHA_ITEM_DURATION(duration_ticks);
    uint16_t high = target_us + margin_us;
    uint16_t low = target_us - margin_us;
    ESP_LOGI(TAG, "duration_ticks=%d[RMT]-->%d[us]", duration_ticks,middle);
    ESP_LOGI(TAG, "target_us=%d[us] margin_us=%d[us]",target_us,margin_us);
    ESP_LOGI(TAG, "range(high)=%d[us] range(low)=%d[us]", high,low);
#endif

    if(( AEHA_ITEM_DURATION(duration_ticks) <= (target_us + margin_us))
        && ( AEHA_ITEM_DURATION(duration_ticks) >= (target_us - margin_us))) {
        return true;
    } else {
        return false;
    }
}

/*
 * @brief Check whether duration is ZERO
 */
bool aeha_check_is_zero(int duration_ticks)
{
    if( AEHA_ITEM_DURATION(duration_ticks) == 0 ) {
        return true;
    } else {
        return false;
    }
}

/*
 * @brief Check whether this value represents an AEHA header
 */
bool aeha_header_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && aeha_check_in_range(item->duration0, AEHA_HEADER_HIGH_US, AEHA_BIT_MARGIN)
        && aeha_check_in_range(item->duration1, AEHA_HEADER_LOW_US, AEHA_BIT_MARGIN)) {
        return true;
    }
    return false;
}

/*
 * @brief Check whether this value represents an AEHA trailer
 */
bool aeha_trailer_if(rmt_item32_t* item)
{
#if _DEBUG_
    ESP_LOGI(TAG, "aeha_trailer_if duration0=%d duration1=%d", item->duration0, item->duration1);
#endif
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && aeha_check_in_range(item->duration0, AEHA_TRAILER_HIGH_US, AEHA_BIT_MARGIN)
        && aeha_check_in_range(item->duration1, AEHA_TRAILER_LOW_US, 0)) {
        return true;
    }
    return false;
}

/*
 * @brief Check whether this value represents an AEHA data bit 1
 */
bool aeha_bit_one_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && aeha_check_in_range(item->duration0, AEHA_BIT_ONE_HIGH_US, AEHA_BIT_MARGIN)
        && aeha_check_in_range(item->duration1, AEHA_BIT_ONE_LOW_US, AEHA_BIT_MARGIN)) {
        return true;
    }
    return false;
}


/*
 * @brief Check whether this value represents an AEHA data bit 0
 */
bool aeha_bit_zero_if(rmt_item32_t* item)
{
    if((item->level0 == RMT_RX_ACTIVE_LEVEL && item->level1 != RMT_RX_ACTIVE_LEVEL)
        && aeha_check_in_range(item->duration0, AEHA_BIT_ZERO_HIGH_US, AEHA_BIT_MARGIN)
        && aeha_check_in_range(item->duration1, AEHA_BIT_ZERO_LOW_US, AEHA_BIT_MARGIN)) {
        return true;
    }
    return false;
}


/*
 * @brief Build parity data from customer code.
 */
uint8_t aeha_make_parity(uint16_t customer)
{
    ESP_LOGD(TAG, "aeha_make_parity customer=0x%04x", customer);
    uint8_t bit[4];
    bit[0] = (customer & 0xf000) >> 12;
    bit[1] = (customer & 0x0f00) >> 8;
    bit[2] = (customer & 0x00f0) >> 4;
    bit[3] = (customer & 0x000f) >> 0;
    ESP_LOGD(TAG, "aeha_make_parity bit=%02x-%02x-%02x-%02x", bit[0],bit[1],bit[2],bit[3]);
    uint8_t parity_t = 0;
    parity_t = bit[0] ^ bit[1];
    parity_t = parity_t ^ bit[2];
    parity_t = parity_t ^ bit[3];
    ESP_LOGD(TAG, "aeha_make_parity parity_t=%02x",parity_t);
    return parity_t;
}


/*
 * @brief Parse AEHA waveform to customer code, parity and data.
 */
int aeha_parse_items(rmt_item32_t* item, int item_num, uint16_t* customer, uint8_t* parity, int* data_num, uint8_t* data)
{
    ESP_LOGD(TAG, "aeha_parse_items item_num=%d", item_num);
    if (item_num == 0) return -1;

    //Check Leader
    int w_len = 0;
    //int i = 0, j = 0;
    if(!aeha_header_if(item++)) {
        return -1;
    }
    w_len++;

    //Check Customer Code
    uint16_t customer_t = 0;
    for(int i = 0; i < 16; i++) {
        if(aeha_bit_one_if(item)) {
            customer_t |= (1 << i);
        } else if(aeha_bit_zero_if(item)) {
            customer_t |= (0 << i);
        } else {
            return -1;
        }
        item++;
        w_len++;
    }
    ESP_LOGD(TAG, "customer_t=0x%04x,w_len=%d", customer_t, w_len);
    *customer = customer_t;

    *data_num = 0;
    uint8_t data_t = 0;
    //if(aeha_trailer_if(item)) return w_len;

    //Check Parity
    uint8_t parity_t = 0;
    for(int i = 0; i < 4; i++) {
        ESP_LOGD(TAG, "parity w_len=%d", w_len);
        if(aeha_bit_one_if(item)) {
            parity_t |= (1 << i);
        } else if(aeha_bit_zero_if(item)) {
            parity_t |= (0 << i);
        } else {
            return -1;
        }
        item++;
        w_len++;
    }
    ESP_LOGD(TAG, "parity_t=0x%02x", parity_t);
    uint8_t parity_c = aeha_make_parity(customer_t);
    ESP_LOGD(TAG, "parity_c=0x%02x", parity_c);
    if (parity_t != parity_c) return -1;
    *parity = parity_t;

    //Check 1st data
    data_t = 0;
    for(int i = 0; i < 4; i++) {
        ESP_LOGD(TAG, "1st w_len=%d", w_len);
        if(aeha_bit_one_if(item)) {
            data_t |= (1 << i);
        } else if(aeha_bit_zero_if(item)) {
            data_t |= (0 << i);
        } else {
            return -1;
        }
        item++;
        w_len++;
    }
    ESP_LOGD(TAG, "data[%d]=0x%04x", *data_num, data_t);
    data[(*data_num)++] = data_t;

    //Check data
    while(1) {
        if(w_len == item_num) return -1;
        if(aeha_trailer_if(item)) return w_len;
        data_t = 0;
        for(int i = 0; i < 8; i++) {
            ESP_LOGD(TAG, "data w_len=%d", w_len);
            if(aeha_bit_one_if(item)) {
                data_t |= (1 << i);
            } else if(aeha_bit_zero_if(item)) {
                data_t |= (0 << i);
            } else {
                return -1;
            }
            item++;
            w_len++;
        }
        ESP_LOGD(TAG, "data[%d]=0x%04x", *data_num, data_t);
        data[(*data_num)++] = data_t;
    }
    return w_len;
}

/*
 * @brief Build AEHA waveform.
 */
int aeha_build_items(int channel, rmt_item32_t* item, uint16_t customer, uint8_t parity, int data_num, uint8_t* data)
{
    int ret = 0;
    aeha_fill_item_header(item++);
    ret++;
    for(int j = 0; j < 16; j++) {
        if(customer & 0x1) {
            aeha_fill_item_bit_one(item);
        } else {
            aeha_fill_item_bit_zero(item);
        }
        item++;
        ret++;
        customer >>= 1;
    }

    ESP_LOGI(TAG, "aeha_build_items parity=0x%02x", parity);
    for(int i = 0; i < 4; i++) {
        if(parity & 0x1) {
            aeha_fill_item_bit_one(item);
        } else {
            aeha_fill_item_bit_zero(item);
        }
        item++;
        ret++;
        parity >>= 1;
    }

    for(int i=0; i<data_num; i++) {
        uint8_t data_t = data[i];
        ESP_LOGI(TAG, "aeha_build_items data[%d]=0x%02x", i, data[i]);
	int bits = 8;
	if (i == 0) bits = 4;
        for(int j = 0; j < bits; j++) {
            if(data_t & 0x1) {
                aeha_fill_item_bit_one(item);
            } else {
                aeha_fill_item_bit_zero(item);
            }
            item++;
            ret++;
            data_t >>= 1;
        }
    }
    aeha_fill_item_end(item);
    ret++;
    return ret;
}

/*
 * @brief RMT transmitter initialization
 */
void aeha_tx_init(int RMT_TX_CHANNEL, int RMT_TX_GPIO_NUM)
{
    rmt_config_t rmt_tx;
    rmt_tx.channel = RMT_TX_CHANNEL;
    rmt_tx.gpio_num = RMT_TX_GPIO_NUM;
    rmt_tx.mem_block_num = 1;
    rmt_tx.clk_div = RMT_CLK_DIV;
    rmt_tx.tx_config.loop_en = false;
    rmt_tx.tx_config.carrier_duty_percent = 50;
    rmt_tx.tx_config.carrier_freq_hz = 38000;
    rmt_tx.tx_config.carrier_level = 1;
    rmt_tx.tx_config.carrier_en = RMT_TX_CARRIER_EN;
    rmt_tx.tx_config.idle_level = 0;
    rmt_tx.tx_config.idle_output_en = true;
    rmt_tx.rmt_mode = 0;
    rmt_config(&rmt_tx);
    rmt_driver_install(rmt_tx.channel, 0, 0);
}

/*
 * @brief RMT receiver initialization
 */
void aeha_rx_init(int RMT_RX_CHANNEL, int RMT_RX_GPIO_NUM)
{
    rmt_config_t rmt_rx;
    rmt_rx.channel = RMT_RX_CHANNEL;
    rmt_rx.gpio_num = RMT_RX_GPIO_NUM;
    rmt_rx.clk_div = RMT_CLK_DIV;
    rmt_rx.mem_block_num = 1;
    rmt_rx.rmt_mode = RMT_MODE_RX;
    rmt_rx.rx_config.filter_en = true;
    rmt_rx.rx_config.filter_ticks_thresh = 100;
    rmt_rx.rx_config.idle_threshold = rmt_item32_tIMEOUT_US / 10 * (RMT_TICK_10_US);
    rmt_config(&rmt_rx);
    rmt_driver_install(rmt_rx.channel, 1000, 0);
}
