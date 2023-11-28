#include "controller.h"
#include "configuration.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "dht.h"
#include "driver/adc.h"
#include "servo.h"
#include "esp_adc_cal.h"
#include <stdlib.h>
#include "servo.h"

/* Varijable koje spremaju stanje servo-a */
double servo_angle = 0;
/* Varijable koje spremaju stanje servo-a */

static const char* TAG = "ESP32 Blinds Control";

/* Varijable za ADC pretvorbu odnosno za svjetlosne otpornike */
static const adc_channel_t channel0 = ADC_CHANNEL_6; // Pin 34
static const adc_channel_t channel1 = ADC_CHANNEL_7; // Pin 35

static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

static const adc_bits_width_t width = ADC_WIDTH_BIT_12;

// Za ADC
static esp_adc_cal_characteristics_t *adc_chars;
/* Varijable za ADC pretvorbu odnosno za svjetlosne otpornike */

/* ADC za svjetlosne otpornike */
void config_ADC() {

    ESP_LOGI(TAG, "Configuring ADC.");

    // Konfiguracija ADC-a, rezolucija kanala
    adc1_config_width(ADC_WIDTH_BIT_12);

    // Konfiguracija atenuacije
    adc1_config_channel_atten((adc1_channel_t)channel0, atten);
    adc1_config_channel_atten((adc1_channel_t)channel1, atten);

    // Dobij karakteristike ADC-a
    adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, 1100, adc_chars);

    ESP_LOGI(TAG, "ADC configured.");

}

/* Specifične funkcije za ovaj uređaj */

/* Salji vrijednosti fotootpornika */
void send_photoresistors_values(esp_mqtt_client_handle_t client) {

    uint32_t value;
    value = adc1_get_raw((adc1_channel_t)channel0);
    
    char value_string0[32];
    sprintf(value_string0, "{\"PHOTORESISTOR0\":\"%ld\"}", value);
    esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, value_string0, 0, 0, 0);
    
    value = adc1_get_raw((adc1_channel_t)channel1);
    char value_string1[32];
    sprintf(value_string1, "{\"PHOTORESISTOR1\":\"%ld\"}", value);
    esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, value_string1, 0, 0, 0);

}

/* Šalji vrijednosti vlažnosti i temperature */
void send_temperature_and_humidity(esp_mqtt_client_handle_t client) {

    float humidity = 0, temperature = 0;
    dht_read_float_data(DHT_TYPE_DHT11, (gpio_num_t)DHT_DATA_PIN, &humidity, &temperature);
    printf("Temperature %f°C, humidity %f%%\n", temperature, humidity);
    
    char humidity_string[32];
    sprintf(humidity_string, "{\"HUMIDITY\":\"%.2f%%\"}", humidity);
    esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, humidity_string, 0, 0, 0);
    
    char temperature_string[32];
    sprintf(temperature_string, "{\"TEMPERATURE\":\"%.2f°C\"}", temperature);
    esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, temperature_string, 0, 0, 0);

}

/* Šalji vrijednost serva */
void send_servo_angle(esp_mqtt_client_handle_t client) {

    char servo_string[32];
    sprintf(servo_string, "{\"SERVO\":\"%.2f°\"}", servo_angle);
    esp_mqtt_client_publish(client, PUBLISH_TO_TOPIC, servo_string, 0, 0, 0);

}

/* Postavi stanje servo-a */
void set_servo_angle(esp_mqtt_client_handle_t client, char* data, uint32_t size) {

    char* delimiter_colon = ":";
    char* delimiter_degree = "°";
    
    // Vrati mi pokazivač gdje se dvotočka prvi puta pojavljuje
    char* colon = strchr(data, delimiter_colon[0]);

    // Vrati mi pokazivač gdje se oznaka celzijus prvi puta pojavljuje
    char* degree = strchr(data, delimiter_degree[0]);

    if(colon != NULL && degree != NULL) {

        // Pokazivač na početak broja u poruci
        char* substring = colon + 1;

        // Velčina podniza string-a koji sadrži broj
        size_t substring_length = degree - substring;

        // Alociramo placeholder
        char* number_string = (char*)malloc((substring_length + 1) * sizeof(char));

        if(number_string != NULL) {

            // Kopiraj pod-string u novoalocirani prostor
            strncpy(number_string, substring, substring_length);

            // Dodaj mu na kraju terminator
            number_string[substring_length] = '\0';

            printf("%s\n", number_string);

            // Pretvori realni broj u string-u u float
            servo_angle = atof(number_string);
            if(servo_angle < 0.00) servo_angle = 0.00;
            if(servo_angle > 180.00) servo_angle = 180.00;
        
        }

        // Oslobađamo placeholder
        free(number_string);

        printf("Servo angle: %.2f°\n", servo_angle);
        ESP_ERROR_CHECK(servo_position(servo_angle));

    } else {

        ESP_LOGE(TAG, "Error in parsing received command regarding servo angle");

    }
    
}

/* Specifične funkcije za ovaj uređaj */

/* Dretva za periodičko objavljivanje podataka */
void device_send_data_task(void *param) {

    esp_mqtt_client_handle_t *clientPointer = (esp_mqtt_client_handle_t *)param;
    esp_mqtt_client_handle_t client = *clientPointer; 

    while (1) {

        send_photoresistors_values(client);
        send_temperature_and_humidity(client);
        send_servo_angle(client);

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    
    }

}

/* Početne postavke */
void device_setup() {

    ESP_LOGI(TAG, "Device function setup initiated.");
    
    /* Sinkroniziraj DHT11 */
    gpio_set_direction((gpio_num_t)DHT_DATA_PIN, GPIO_MODE_OUTPUT_OD);
    gpio_set_level((gpio_num_t)DHT_DATA_PIN, 1);

    /* Konfiguracija ADC pinova */
    config_ADC();

    /* Konfiguracija PWM-a */
    servo_setup(SERVO_PIN);
    
    ESP_LOGI(TAG, "Device function setup finished.");
    
}

/* Funkcija koju uređaj izvršava pri svakom dolasku MQTT poruke */
void device_actuate(esp_mqtt_client_handle_t client, esp_mqtt_event_handle_t event) {

    /* Ako je došla poruka za vrijednost temperature */
    if(strncmp(event->data, "SERVO:", sizeof("SERVO:") - 1) == 0) {
        set_servo_angle(client, event->data, event->data_len); /* Uspoređuje do terminatora */
    }

}