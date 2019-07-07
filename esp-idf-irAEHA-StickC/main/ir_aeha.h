#ifndef MAIN_IR_NEC_H_
#define MAIN_IR_NEC_H_

#include "esp_log.h"
#include "driver/rmt.h"

#define RMT_RX_ACTIVE_LEVEL  0   /*!< If we connect with a IR receiver, the data is active low */
#define RMT_TX_CARRIER_EN    1   /*!< Enable carrier for IR transmitter test with IR led */

//#define RMT_TX_CHANNEL    1     /*!< RMT channel for transmitter */
//#define RMT_TX_GPIO_NUM  17     /*!< GPIO number for transmitter signal */
//#define RMT_RX_CHANNEL    0     /*!< RMT channel for receiver */
//#define RMT_RX_GPIO_NUM  19     /*!< GPIO number for receiver */
#define RMT_CLK_DIV      100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */

#define AEHA_HEADER_HIGH_US    3400                         /*!< AEHA protocol header: positive 3.4ms */
#define AEHA_HEADER_LOW_US     1700                         /*!< AEHA protocol header: negative 1.7ms*/
#define AEHA_BIT_ONE_HIGH_US    425                         /*!< AEHA protocol data bit 1: positive 0.425ms */
#define AEHA_BIT_ONE_LOW_US   (1700-AEHA_BIT_ONE_HIGH_US)   /*!< AEHA protocol data bit 1: negative 1.275ms */
#define AEHA_BIT_ZERO_HIGH_US   425                         /*!< AEHA protocol data bit 0: positive 0.425ms */
#define AEHA_BIT_ZERO_LOW_US   (850-AEHA_BIT_ZERO_HIGH_US)  /*!< AEHA protocol data bit 0: negative 0.425ms */
#define AEHA_TRAILER_HIGH_US    425                         /*!< AEHA protocol end: negative 8.00ms */
#define AEHA_TRAILER_LOW_US       0                         /*!< AEHA protocol end: negative 8.00ms */
#define AEHA_BIT_MARGIN         100                         /*!< AEHA parse margin time */

#define AEHA_ITEM_DURATION(d)  ((d & 0x7fff)*10/RMT_TICK_10_US)  /*!< Parse duration time from memory register value */
#define AEHA_DATA_ITEM_NUM  22  /*!< AEHA code item number: leader + customer code(16bit) + parity(4bit) + trailer */
#define RMT_TX_DATA_NUM  100    /*!< AEHA tx test data number */
#define rmt_item32_tIMEOUT_US  9500   /*!< RMT receiver timeout value(us) */

void aeha_fill_item_level(rmt_item32_t* item, int high_us, int low_us);
void aeha_fill_item_header(rmt_item32_t* item);
void aeha_fill_item_bit_one(rmt_item32_t* item);
void aeha_fill_item_bit_zero(rmt_item32_t* item);
void aeha_fill_item_end(rmt_item32_t* item);
bool aeha_check_in_range(int duration_ticks, int target_us, int margin_us);
bool aeha_check_is_zero(int duration_ticks);
bool aeha_header_if(rmt_item32_t* item);
bool aeha_trailer_if(rmt_item32_t* item);
bool aeha_bit_one_if(rmt_item32_t* item);
bool aeha_bit_zero_if(rmt_item32_t* item);
uint8_t aeha_make_parity(uint16_t customer);
int aeha_parse_items(rmt_item32_t* item, int item_num, uint16_t* customer, uint8_t* parity, int* data_num, uint8_t* data);
int aeha_build_items(int channel, rmt_item32_t* item, uint16_t customer, uint8_t parity, int data_num, uint8_t* data);
void aeha_tx_init(int RMT_TX_CHANNEL, int RMT_TX_GPIO_NUM);
void aeha_rx_init(int RMT_RX_CHANNEL, int RMT_RX_GPIO_NUM);

#endif /* MAIN_IR_NEC_H_ */
