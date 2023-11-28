#include "controller.h"
#include "configuration.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char* TAG = "ESP32 Lights Control";

/* Varijable koje spremaju stanje LED */
int led[3] = {0};
/* Varijable koje spremaju stanje LED */

/* Specifične funkcije za ovaj uređaj */

/* Salji stanja LED-ica */
void send_LED_states(esp_mqtt_client_handle_t client) {

    /* Šalji povratnu informaciju trenutnog stanja */
    if(led[0] == 1) esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, "{\"LED0\":\"ON\"}", 0, 0, 0);
    else esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, "{\"LED0\":\"OFF\"}", 0, 0, 0);
    printf("LED0 state:%d\n", led[0]);

    if(led[1] == 1) esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, "{\"LED1\":\"ON\"}", 0, 0, 0);
    else esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, "{\"LED1\":\"OFF\"}", 0, 0, 0);
    printf("LED1 state:%d\n", led[1]);

    if(led[2] == 1) esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, "{\"LED2\":\"ON\"}", 0, 0, 0);
    else esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, "{\"LED2\":\"OFF\"}", 0, 0, 0);
    printf("LED2 state:%d\n", led[2]);

}

/* Postavi stanja LED-ica */
void set_LED_state(int setLevel, int ledPin, int ledNumberInArray) {

    gpio_num_t gpioPin = (gpio_num_t) ledPin;
    
    gpio_set_level(gpioPin, setLevel);
    led[ledNumberInArray] = setLevel;
    
}

/* Specifične funkcije za ovaj uređaj */

/* Dretva za periodičko objavljivanje podataka */
void device_send_data_task(void *param) {

    esp_mqtt_client_handle_t *clientPointer = (esp_mqtt_client_handle_t *)param;
    esp_mqtt_client_handle_t client = *clientPointer; 

    while (1) {

        send_LED_states(client);
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    
    }

}

/* Početne postavke */
void device_setup() {

    ESP_LOGI(TAG, "Device function setup initiated");

    /* Odaberi pino-ove za korištenje */
    gpio_reset_pin(LED0_PIN);
    gpio_reset_pin(LED1_PIN);
    gpio_reset_pin(LED2_PIN);

    /* Odaberi način IZLAZ */
    gpio_set_direction(LED0_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED1_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED2_PIN, GPIO_MODE_OUTPUT);

    /* Spusti sve na 0 */
    gpio_set_level(LED0_PIN, 0);
    gpio_set_level(LED1_PIN, 0);
    gpio_set_level(LED2_PIN, 0);

    ESP_LOGI(TAG, "Device function setup finished");

}

/* Funkcija koju uređaj izvršava pri svakom dolasku MQTT poruke */
void device_actuate(esp_mqtt_client_handle_t client, esp_mqtt_event_handle_t event) {

    /* Ako je došla poruka da se LED-ica upali/ugasi, upali/ugasi je */
    if(strncmp(event->data, "LED0:ON", event->data_len) == 0) set_LED_state(1, LED0_PIN, 0);
    else if(strncmp(event->data, "LED0:OFF", event->data_len) == 0) set_LED_state(0, LED0_PIN, 0);
         
    /* Ako je došla poruka da se LED-ica upali/ugasi, upali/ugasi je */
    if(strncmp(event->data, "LED1:ON", event->data_len) == 0) set_LED_state(1, LED1_PIN, 1);
    else if(strncmp(event->data, "LED1:OFF", event->data_len) == 0) set_LED_state(0, LED1_PIN, 1);
          
    /* Ako je došla poruka da se LED-ica upali/ugasi, upali/ugasi je */
    if(strncmp(event->data, "LED2:ON", event->data_len) == 0) set_LED_state(1, LED2_PIN, 2);
    else if(strncmp(event->data, "LED2:OFF", event->data_len) == 0) set_LED_state(0, LED2_PIN, 2);

}