#ifndef __BLINDS_CONTROLLER_H__
#define __BLINDS_CONTROLLER_H__

/* ZA MIKROKONTROLER KOJI UPRAVLJA ROLETAMA */
#include "mqtt_client.h"

/* Za konfiguraciju MQTT-a */
#define MQTT_MY_CLIENT_ID           "BLINDS-CONTROLLER"
/* Za konfiguraciju MQTT-a */

/* Pinovi */
#define DHT_DATA_PIN                GPIO_NUM_4
#define PHOTO_R0                    GPIO_NUM_34   // ADC1_CH6
#define PHOTO_R1                    GPIO_NUM_35   // ADC1_CH7
#define SERVO_PIN                   GPIO_NUM_12
/* Pinovi */

// Funkcije koje će se pozivati pri dolasku MQTT naredbi
/* Sučelje */
void device_setup();
void device_send_data_task(void *param);
void device_actuate(esp_mqtt_client_handle_t client, esp_mqtt_event_handle_t event);
/* Sučelje */

#endif