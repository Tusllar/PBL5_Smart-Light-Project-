// #include <stdio.h>
// #include "esp_log.h"

// #include "freertos/FreeRTOS.h"
// #include "freertos/semphr.h"
// #include "freertos/task.h"

// #include "iot_button.h"
// #include "light_driver.h"

// #include DEVELOPMENT_BOARD
// #include "app_priv.h"
// #include "led_strip.h"

// #define TAG "app_driver"

// static bool g_output_state = true;
// static led_strip_handle_t led_strip = NULL;

// typedef struct {
//     bool power_on;
//     int current_color;
//     int brightness;
// } led_state_t;

// static led_state_t led_state = {true, 0, 128};

// static void apply_led_state(void)
// {
//     if (!led_state.power_on || led_state.current_color == 10) {
//         led_strip_clear(led_strip);
//         led_strip_refresh(led_strip);
//         return;
//     }

//     int r = 0, g = 0, b = 0;
//     switch (led_state.current_color) {
//         case 0: r = led_state.brightness; break;
//         case 1: g = led_state.brightness; break;
//         case 2: b = led_state.brightness; break;
//         case 3: r = g = b = led_state.brightness; break;
//         case 4: r = g = led_state.brightness; break;
//         case 5: g = b = led_state.brightness; break;
//         case 6: r = b = led_state.brightness; break;
//         case 7: r = led_state.brightness; g = led_state.brightness / 2; break;
//         case 8: r = led_state.brightness; g = led_state.brightness / 12; b = led_state.brightness / 2; break;
//         case 9: r = led_state.brightness / 2; b = led_state.brightness / 2; break;
//     }

//     led_strip_set_pixel(led_strip, 0, r, g, b);
//     led_strip_refresh(led_strip);
// }

// static void push_btn_cb(void *arg)
// {
//     led_state.power_on = !led_state.power_on;
//     apply_led_state();
// }

// static void push_btn_cb_change_color(void *arg)
// {
//     led_state.current_color = (led_state.current_color + 1) % 11;
//     apply_led_state();
// }

// static void push_btn_brightness_cb(void *arg)
// {
//     static int step = 5;  // mỗi bước tăng/giảm
//     if (!led_state.power_on) return;

//     led_state.brightness += step;

//     if (led_state.brightness >= 255) {
//         led_state.brightness = 255;
//         step = -step;   // đảo chiều -> bắt đầu giảm
//     } else if (led_state.brightness <= 0) {
//         led_state.brightness = 0;
//         step = -step;   // đảo chiều -> bắt đầu tăng
//     }

//     apply_led_state();
//     vTaskDelay(pdMS_TO_TICKS(20)); // delay nhỏ để thấy hiệu ứng dimming mượt
// }


// void app_driver_init()
// {
//     button_config_t btn_cfg = {
//         .type = BUTTON_TYPE_GPIO,
//         .gpio_button_config = {
//             .gpio_num     = LIGHT_BUTTON_GPIO,
//             .active_level = LIGHT_BUTTON_ACTIVE_LEVEL,
//         },
//     };
//     button_handle_t btn_handle = iot_button_create(&btn_cfg);
//     if (btn_handle) {
//         iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, push_btn_cb);
//         iot_button_register_cb(btn_handle, BUTTON_DOUBLE_CLICK, push_btn_cb_change_color);
//         iot_button_register_cb(btn_handle, BUTTON_LONG_PRESS_HOLD, push_btn_brightness_cb);
//     }

//     light_driver_config_t driver_config = {
//         .gpio_red        = LIGHT_GPIO_RED,
//         .gpio_green      = LIGHT_GPIO_GREEN,
//         .gpio_blue       = LIGHT_GPIO_BLUE,
//         .gpio_cold       = LIGHT_GPIO_COLD,
//         .gpio_warm       = LIGHT_GPIO_WARM,
//         .fade_period_ms  = LIGHT_FADE_PERIOD_MS,
//         .blink_period_ms = LIGHT_BLINK_PERIOD_MS,
//         .freq_hz         = LIGHT_FREQ_HZ,
//         .clk_cfg         = LEDC_USE_APB_CLK,
//         .duty_resolution = LEDC_TIMER_11_BIT,
//     };
//     ESP_ERROR_CHECK(light_driver_init(&driver_config));
//     led_strip_config_t strip_config = {
//         .strip_gpio_num = 8,
//         .max_leds = 1,
//     };

//     led_strip_rmt_config_t rmt_config = {
//         .resolution_hz = 10000000,
//     };

//     ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
//     led_strip_clear(led_strip);
// }

// int IRAM_ATTR app_driver_set_state(bool state)
// {
//     if (g_output_state != state) {
//         g_output_state = state;
//         if (g_output_state) {
//             light_driver_set_switch_gpio8(true);
//         } else {
//             light_driver_set_switch_gpio8(false);
//         }
//     }
//     return ESP_OK;
// }

// bool app_driver_get_state(void)
// {
//     return g_output_state;
// }
/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include "iot_button.h"
#include "light_driver.h"

#include DEVELOPMENT_BOARD
#include "app_priv.h"

#define TAG "app_driver"

static bool g_output_state = true;

static void push_btn_cb(void *arg)
{
    app_driver_set_state(!g_output_state);
}

void app_driver_init()
{
    /* Configure push button */
    button_config_t btn_cfg = {
        .type = BUTTON_TYPE_GPIO,
        .gpio_button_config = {
            .gpio_num     = LIGHT_BUTTON_GPIO,
            .active_level = LIGHT_BUTTON_ACTIVE_LEVEL,
        },
    };
    button_handle_t btn_handle = iot_button_create(&btn_cfg);
    if (btn_handle) {
        /* Register a callback for a button short press event */
        iot_button_register_cb(btn_handle, BUTTON_SINGLE_CLICK, push_btn_cb);
    }

    /**
     * @brief Light driver initialization
     */
    light_driver_config_t driver_config = {
        .gpio_red        = LIGHT_GPIO_RED,
        .gpio_green      = LIGHT_GPIO_GREEN,
        .gpio_blue       = LIGHT_GPIO_BLUE,
        .gpio_cold       = LIGHT_GPIO_COLD,
        .gpio_warm       = LIGHT_GPIO_WARM,
        .fade_period_ms  = LIGHT_FADE_PERIOD_MS,
        .blink_period_ms = LIGHT_BLINK_PERIOD_MS,
        .freq_hz         = LIGHT_FREQ_HZ,
        .clk_cfg         = LEDC_USE_APB_CLK,
        .duty_resolution = LEDC_TIMER_11_BIT,
    };
    ESP_ERROR_CHECK(light_driver_init(&driver_config));
    light_driver_set_switch(true);
}

int IRAM_ATTR app_driver_set_state(bool state)
{
    if (g_output_state != state) {
        g_output_state = state;
        if (g_output_state) {
            // light on
            ESP_LOGI(TAG, "Light ON");
            light_driver_set_switch(true);
        } else {
            // light off
            ESP_LOGI(TAG, "Light OFF");
            light_driver_set_switch(false);
        }
    }
    return ESP_OK;
}

bool app_driver_get_state(void)
{
    return g_output_state;
}
