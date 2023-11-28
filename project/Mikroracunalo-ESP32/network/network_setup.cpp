#include <stdio.h>
#include <esp_wifi.h>
#include <cstring>
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "esp_event.h"
#include "esp_event_loop.h"

#include "configuration.h"
#include "controller.h"
#include "network_setup.h"

#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/dns.h>

static const char* TAG = "ESP32 Network";

/* Handle grupe događaja */
static EventGroupHandle_t eventGroup;

/* Handle za task */
TaskHandle_t taskHandle;

/* Jedna grupa zadataka vezana u grupi događaja može postaviti maksimalno 32 bita,
 * znači možemo imati 32 zadataka u grupi koji se međusobno sinkroniziraju
 * BIT0 - spojili smo se na IP adresu
 * BIT1 - nismo se uspjeli spojiti čak i s pomoću ponavljanjem pokušaja */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

/* Maksimalni broj ponovnih pokušaja */
#define MAX_RETRY   3

// Trenutni broj pokušaja
static int retry_num = 0;

/* Handle za MQTT */
static esp_mqtt_client_handle_t mqtt_client;

/* Handler ID - pokušaj povezat se na mrežu */
static void eventHandlerID(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

    /* Pokušaj se spojiti na mrežu danim SSID-om i password-om */
    if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        
        esp_wifi_connect();

    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
    
        ESP_LOGI(TAG, "WiFi Connected");

    } else if(event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        
        if(retry_num < MAX_RETRY) { /* Pokušaj se ponovno spojit */
        
            ESP_LOGI(TAG, "Retry to connect to the AP");
            esp_wifi_connect();
            retry_num++;
        
        } else {
        
            xEventGroupSetBits(eventGroup, WIFI_FAIL_BIT); /* Postavi bit da je postavljanje neuspješno */
        
        }
        
        ESP_LOGI(TAG,"Connect to the AP fail");
    
    }

}

/* Handler IP - ako nam se dodijeljuje IP adresa */
static void eventHandlerIP(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {

    if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {

        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data; /* Ispisi dodijeljenu IP adresu */
        ESP_LOGI(TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        retry_num = 0;
        xEventGroupSetBits(eventGroup, WIFI_CONNECTED_BIT); /* Postavi da je povezivanje uspješno */
    
    }

}

/* Inicijaliziraj WiFi modul te se spoji na mrežu*/
static void initStationWifi(char* ssid, char* password) {

    /* 
     * Stvori grupu događaja, svaki novostvoreni zadatak koji će se stvoriti s ovim prosljeđenim parametrom moći
     * će komunucirati sa drugim zadatcima koji imaju prosljeđen ovaj parametar
    */
    eventGroup = xEventGroupCreate();

    /* Inicijaliziraj ESP mrežno sučelje */
    // ESP_ERROR_CHECK(esp_netif_init());
    esp_netif_init();

    /* Inicijaliziraj ESP petlja događaja */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    /* Stvori WiFi STATION u WiFi driver-u*/
    esp_netif_create_default_wifi_sta();

    /* Inicijaliziraj uobičajenu strukturu wifi_init_config_t (ovo inicijalizira sam modul) */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    /* Inicijaliziraj WiFi modul sa prethodno definiranom strukturom (postavkama)*/
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* Varijable za ID instanca handler-a događaja */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    /* Registriraj handler za WiFi vezane događaje */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,                         // Handler događaja će se zvati za WiFi vezane događaje
                                                        ESP_EVENT_ANY_ID,                   // Bilo kakav WiFi vezan događaj
                                                        &eventHandlerID,                    // Handler
                                                        NULL,                               // Parametri za handler
                                                        &instance_any_id));                 // Varijabla gdje će će se spremiti ID registriranog događaja


    /* Registriraj handler za IP vezane događaje */
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,                           // Handler događaja će se zvati za IP vezane događaje
                                                        IP_EVENT_STA_GOT_IP,                // Handler će se zvati kada sučelje dobije svoju IP adresu
                                                        &eventHandlerIP,                    // Handler
                                                        NULL,                               // Parametri za handler
                                                        &instance_got_ip));                 // Varijabla gdje će će se spremiti ID registriranog događaja

    
    /* Spremaj podatke u RAM */
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    esp_wifi_set_storage(WIFI_STORAGE_FLASH);

    /* Definiraj karakteristike WiFi mreže kojoj se pridružujemo */
    wifi_config_t wifi_config;
    
    memset(&wifi_config, 0, sizeof(wifi_config));    
    memcpy(wifi_config.sta.ssid, (uint8_t*)SSID, strlen(SSID));
    memcpy(wifi_config.sta.password, (uint8_t*)PASSWORD, strlen(PASSWORD));

    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK; /* Koristi WPA2 */
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    /* Postavi način rada STATION i karakteristike mreže te započni rad modula */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "WiFi initialisation finished");

    /* 
     * Čeka dok se ne uspostavi konekcija (WIFI_CONNECTED_BIT) ili ne uspostavi
     * (WIFI_FAIL_BIT). Prethodna instancirani događaj će postaviti ove bitove
    */
    EventBits_t bits = xEventGroupWaitBits(eventGroup,          // Grupa događaja na koju se odnosi ovo čekanje (prethodna dva instancirana događaja)
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,                 // Bitove na koje čekamo (BIT 0 ili BIT 1)
            pdFALSE,                                            // Nemoj odmah očistiti bitove nakon operacije čekanja
            pdFALSE,                                            // Čekaj hoće li se postaviti bilo koji bit od gore navedenih - OR operacija (pdTRUE za AND) 
            portMAX_DELAY);                                     // Čekaj zbeskonačno mnogo dok se ovo ne desi

    /* Ispitaj što se dogodilo na kraju */
    if(bits & WIFI_CONNECTED_BIT) ESP_LOGI(TAG, "Connected to ap SSID:%s password:%s", ssid, password);
    else if(bits & WIFI_FAIL_BIT) ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ssid, password);
    else ESP_LOGE(TAG, "UNEXPECTED EVENT");

    /* Makni prethodne registracije, handleri se sad više neće zvati */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    
    /* Obriši grupu događaja */
    vEventGroupDelete(eventGroup);

}

/* Konstruktor */
ESP32_network::ESP32_network() {}

void ESP32_network::setupWiFi() {

    /* Inicijaliziraj trajnu (non-volatile) particiju za pisanje (kako bi zadržali postavke WiFi-ja) */
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      nvs_flash_erase();
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* Inicijaliziraj STATION način rada -> uređaj će se ponašati kao običan klijent spojen na mrežu */
    initStationWifi(SSID, PASSWORD);

}

/* Pronađi IP adresu lokalne mreže na kojoj je broker */
int find_IP_by_hostname(char broker_IP_address[16]) {

    int found = -1;

    /* TODO - implementiraj pretragu IP adresu po imenu */

    return -1;

}

/* Ispis ako ima greške */
static void log_error_if_nonzero(const char *message, int error_code)
{
    if(error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/* Handler za MQTT - pokušaj povezat se na mrežu */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {

    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
    esp_mqtt_client_handle_t client = event->client;
    
    int msg_id;

    /* Ovisno o ID-u poruke, radi sljedeće */
    switch((esp_mqtt_event_id_t)event_id) {
        
        case MQTT_EVENT_CONNECTED:
        
            /* Pretplati se na temu */
            msg_id = esp_mqtt_client_subscribe(client, SUBSCRIBE_TO_TOPIC, 0);
            ESP_LOGI(TAG, "Sent subscribe successful, msg_id=%d", msg_id);

            break;
        
        case MQTT_EVENT_DISCONNECTED:

            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");

            break;

        case MQTT_EVENT_SUBSCRIBED:

            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            
            /* Ako si pretplaćen stvori zadatak, parametri: funkcija koja se izvršava, ime zadatka, 
            veličina stoga, parametar koji se prosljeđuje, prioritet, handle -> mqtt_client je globalna varijabla
            ako bi stavili event->client događa se kernel panic */
            xTaskCreate(&device_send_data_task, "device_task", 4096, &mqtt_client, 1, &taskHandle);

            ESP_LOGI(TAG, "Device task created");

            break;
        
        case MQTT_EVENT_UNSUBSCRIBED:

            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            
            /* Ako raskineš pretplatu uništi handle */
            vTaskDelete(taskHandle);

            ESP_LOGI(TAG, "Device task destroyed");

            break;
        
        case MQTT_EVENT_PUBLISHED:
            
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            
            break;
        
        case MQTT_EVENT_DATA:

            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            
            /* Kada smo dobili poruku */
            device_actuate(client, event);
            
            break;
        
        case MQTT_EVENT_ERROR:
            
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

            }
            
            break;

        default:

            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            
            break;
    
    }

}

/* Postavi MQTT */
void ESP32_network::setupMQTT() {

    /* Pronađi IP adresu brokera na lokalnoj mreži */
    char broker_IP_address[16];
    int found = find_IP_by_hostname(broker_IP_address);
    
    /* Ako pretraživanje po imenu nije uspjelo, a nije ni IP adresa zadana */
    if(found == -1)
        if(sizeof(MQTT_BROKER_ADDRESS) == 0)
            return;
    


    /* Konfiguracija */
    esp_mqtt_client_config_t mqtt_cfg = {0};
    mqtt_cfg.broker.address.uri = MQTT_BROKER_ADDRESS;               // URI brokera
    mqtt_cfg.credentials.username = MQTT_USERNAME;
    mqtt_cfg.credentials.authentication.password = MQTT_PASSWORD;

    /* Handle */
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    ESP_LOGI(TAG, "Client initialized");

    /* Registriraj događaj */
    esp_mqtt_client_register_event(mqtt_client, static_cast<esp_mqtt_event_id_t>(ESP_EVENT_ANY_ID), mqtt_event_handler, NULL);

    /* Postavi početne postavke uređaja */
    device_setup();

    /* Započni konekciju */
    esp_mqtt_client_start(mqtt_client);
    ESP_LOGI(TAG, "Started MQTT");

}