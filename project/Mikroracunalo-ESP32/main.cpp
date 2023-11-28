#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "configuration.h"
#include "network_setup.h"

static const char* TAG = "ESP32 Main";

/* Ulazna točka ' extern "C" ' označava da ju mogu zvati drugi programi koji su pisani u C-u */
extern "C" void app_main(void)
{

    ESP_LOGI(TAG, "Started Main.");

    /* Inicijaliziraj mrežu */
    ESP32_network esp32_network;
    esp32_network.setupWiFi();
    esp32_network.setupMQTT();
    /* Inicijaliziraj mrežu */

    while (1) {

        vTaskDelay(10000 / portTICK_PERIOD_MS);

    }


}