#ifndef __LIGHTS_CONTROLLER_H__
#define __LIGHTS_CONTROLLER_H__

/* ZA MIKROKONTROLER KOJI UPRAVLJA OSVJETLJENJEM */
#include "mqtt_client.h"

/* Za konfiguraciju MQTT-a */
#define MQTT_MY_CLIENT_ID           "LIGHTS-CONTROLLER"
/* Za konfiguraciju MQTT-a */

/* Pinovi */
#define LED0_PIN                    GPIO_NUM_2
#define LED1_PIN                    GPIO_NUM_3
#define LED2_PIN                    GPIO_NUM_4
/* Pinovi */

// Funkcije koje će se pozivati pri dolasku MQTT naredbi
/* Sučelje */
void device_setup();
void device_send_data_task(void *param);
void device_actuate(esp_mqtt_client_handle_t client, esp_mqtt_event_handle_t event);
/* Sučelje */

#endif