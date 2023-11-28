#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

/* Ovdje se namještaju početne postavke */

/* Mreža: WiFi i IP */
#define SSID                        "Iwan's Galaxy A52"
#define PASSWORD                    "00112233"

/* Mreža: MQTT*/
#define MQTT_BROKER_ADDRESS         "mqtt://192.168.82.192:1883"
#define MQTT_BROKER_HOSTNAME        "aorus"
#define MQTT_USERNAME               "username"
#define MQTT_PASSWORD               "password"
#define PUBLISH_TO_TOPIC            "sensors/data"
#define SUBSCRIBE_TO_TOPIC          "homeassistant/commands"

#endif