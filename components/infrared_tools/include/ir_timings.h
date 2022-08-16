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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Timings for AEHA protocol
 *
 * T=350-500us (425us type)
 */
#define AEHA_LEADING_CODE_HIGH_US (3400) // 425*8
#define AEHA_LEADING_CODE_LOW_US (1700) // 425*4
#define AEHA_PAYLOAD_ONE_HIGH_US (425)
#define AEHA_PAYLOAD_ONE_LOW_US (1275) // 425*3
#define AEHA_PAYLOAD_ZERO_HIGH_US (425)
#define AEHA_PAYLOAD_ZERO_LOW_US (425)
#define AEHA_REPEAT_CODE_HIGH_US (3400) // 425*8
#define AEHA_REPEAT_CODE_LOW_US (3400) // 425*8
#define AEHA_ENDING_CODE_HIGH_US (425)
//AEHA_ENDING_CODE_LOW_US is constant of 0x7FFF

/**
 * @brief Timings for RC5 protocol
 *
 */
#define RC5_PULSE_DURATION_US (889)

#ifdef __cplusplus
}
#endif
