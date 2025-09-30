idf_component_register(SRCS "app_driver.c" "button_adc.c" "button_gpio.c" "iot_button.c"
                        INCLUDE_DIRS include
                        PRIV_REQUIRES  esp_adc driver esp_timer)
