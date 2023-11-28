#ifndef __NETWORK_SETUP_H__
#define __NETWORK_SETUP_H__

// Izvori za spajanje na WiFi:
// https://github.com/espressif/esp-idf/blob/v4.4.1/examples/wifi/getting_started/station/main/station_example_main.c
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html
// https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/mqtt.html

// Klasa u kojoj definiramo parametre za WiFi, IP/TCP i MQTT protokole
class ESP32_network {

    public:
        ESP32_network();
        void setupWiFi();
        void setupMQTT();

};

#endif