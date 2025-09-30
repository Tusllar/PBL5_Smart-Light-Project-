// Copyright 2020 Espressif Systems (Shanghai) Co. Ltd.
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

#ifndef __BOARD_ESP32C3_DEVKITC_H__
#define __BOARD_ESP32C3_DEVKITC_H__

#define LIGHT_BUTTON_GPIO          9    /* This is the button that is used for toggling the output */
#define LIGHT_BUTTON_ACTIVE_LEVEL  0

/**
 * @brief Light driver Macro
 */
#define LIGHT_GPIO_RED          5
#define LIGHT_GPIO_GREEN        6
#define LIGHT_GPIO_BLUE         8
#define LIGHT_GPIO_COLD         7
#define LIGHT_GPIO_WARM         10
#define LIGHT_FADE_PERIOD_MS    50000     /**< The time from the current state to the next state */
#define LIGHT_BLINK_PERIOD_MS   3000    /**< Period of blinking lights */
#define LIGHT_FREQ_HZ           10000    /**< frequency of ledc signal */

#endif /**< __BOARD_ESP32C3_DEVKITC_H__ */
