// Izvor: https://github.com/UncleRus/esp-idf-lib/tree/master/components/dht

/*
 * Copyright (c) 2016 Jonathan Hartsuiker <https://github.com/jsuiker>
 * Copyright (c) 2018 Ruslan V. Uss <unclerus@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of itscontributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file dht.c
 *
 * ESP-IDF driver for DHT11, AM2301 (DHT21, DHT22, AM2302, AM2321), Itead Si7021
 *
 * Ported from esp-open-rtos
 *
 * Copyright (c) 2016 Jonathan Hartsuiker <https://github.com/jsuiker>\n
 * Copyright (c) 2018 Ruslan V. Uss <unclerus@gmail.com>\n
 *
 * BSD Licensed as described in the file LICENSE
 */

#include "dht.h"

#include <freertos/FreeRTOS.h>
#include <string.h>
#include <esp_log.h>
#include <rom/ets_sys.h>
#include "esp_idf_lib_helpers.h"

// Preciznost DHT timera u mikrosekundama
#define DHT_TIMER_INTERVAL 2
#define DHT_DATA_BITS 40
#define DHT_DATA_BYTES (DHT_DATA_BITS / 8)

/*
 *
 *  __           ______          _______                              ___________________________
 *    \    A    /      \   C    /       \   DHT duration_data_low    /                           \
 *     \_______/   B    \______/    D    \__________________________/   DHT duration_data_high    \__
 *
 *
 *  Inicijalizacija komunikacije sa DHT-om sastoji se od 4 DHT faza:
 *
 *  Faza A - ESP postavlja signal u 0 barem 18000us
 *  Faza B - ESP dopušta signalu da se vrati u 1 i čeka 20-40us na DHT-u da ga postavi u 0
 *  Faza C - DHT postavlja signal u 0 za ~80us
 *  Faza D - DHT postavlja signal u 1 za ~80us
 *
 *  Nakon ovoga DHT prenosi prvi bit postavljajući signal u stanje 0 na 50us
 *  onda postavlja signal u stanje 1 i drži ga na neku vrijeme ovisno o bitu koji se šalje.
 *  ako je duljina vremena manja od 50us onda se interpretira stanje 0 inače stanje '1'.
 *
 *  40 bitova podataka se prenose sekvencijalno. Ovi bitovi pune nize od 5 bajtova.
 *  Prvi i treći bajt su vlažnost (%) i temperatura (C). Bajtovi 2 i 4
 *  su 0 a bajt 5 je checksum čija se ispravnost računa kao:
 *
 *  bajt_5 == (bajt_1 + bajt_2 + bajt_3 + bajt_4) & 0xFF
 *
 */

static const char *TAG = "dht";

#if HELPER_TARGET_IS_ESP32
static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
#define PORT_ENTER_CRITICAL() portENTER_CRITICAL(&mux)
#define PORT_EXIT_CRITICAL() portEXIT_CRITICAL(&mux)

#elif HELPER_TARGET_IS_ESP8266
#define PORT_ENTER_CRITICAL() portENTER_CRITICAL()
#define PORT_EXIT_CRITICAL() portEXIT_CRITICAL()
#endif

#define CHECK_ARG(VAL) do { if (!(VAL)) return ESP_ERR_INVALID_ARG; } while (0)

#define CHECK_LOGE(x, msg, ...) do { \
        esp_err_t __; \
        if ((__ = x) != ESP_OK) { \
            PORT_EXIT_CRITICAL(); \
            ESP_LOGE(TAG, msg, ## __VA_ARGS__); \
            return __; \
        } \
    } while (0)

// Argumenti: pin SDA, timeout, očekivano stanje pina nakon intervala, duljina vremena koja je bila potrebna da DHT postavi željeno stanje
static esp_err_t dht_await_pin_state(gpio_num_t pin, uint32_t timeout, int expected_pin_state, uint32_t *duration) {

    // Postavi pin u ulaznom načinu rada, čekaj DHT-ove signale u fazi B, C i D i pazi na dodjeljene timeout-e
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    for(uint32_t i = 0; i < timeout; i += DHT_TIMER_INTERVAL) { /* Pazi na timeout */

        // Cekaj bar jedan cijeli interval kako bi se spriječilo podrhtavanje signala (signal možda mrvicu zakasni prijeći u željeno stanje)
        ets_delay_us(DHT_TIMER_INTERVAL);
        if(gpio_get_level(pin) == expected_pin_state) /* Usporedi stanje */
        {
            if(duration)
                *duration = i;
            return ESP_OK;
        }
    
    }

    return ESP_ERR_TIMEOUT;
}

// Zatrazi podatke od DHT-a
static inline esp_err_t dht_fetch_data(dht_sensor_type_t sensor_type, gpio_num_t pin, uint8_t data[DHT_DATA_BYTES]) {

    uint32_t low_duration;
    uint32_t high_duration;

    // Faza A - započni zahtjev za čitanjem
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    
    // Vremenski razmak ovisno o tipu komponente (500us ili 20000us), za DHT22 je 20000us (faza a barem 180000us u stanje 0)
    gpio_set_level(pin, 0);
    ets_delay_us(sensor_type == DHT_TYPE_SI7021 ? 500 : 20000);
    
    // Vrati stanje natrag u 1
    gpio_set_level(pin, 1);

    // Faza B - čekaj 40 us dok se ne pojavi stanje 0
    CHECK_LOGE(dht_await_pin_state(pin, 40, 0, NULL),
            "Initialization error, problem in phase 'B'");
    
    // Faza C - čekaj 88 us dok se ne pojavi stanje 1
    CHECK_LOGE(dht_await_pin_state(pin, 88, 1, NULL),
            "Initialization error, problem in phase 'C'");
    
    // Faza D - čekaj 88 us dok se ne pojavi stanje 0
    CHECK_LOGE(dht_await_pin_state(pin, 88, 0, NULL),
            "Initialization error, problem in phase 'D'");

    // Čitaj 40 bitova
    for(int i = 0; i < DHT_DATA_BITS; i++) {

        // Čekaj max 65us dok DHT ne postavi stanje HIGH
        CHECK_LOGE(dht_await_pin_state(pin, 65, 1, &low_duration),
                "LOW bit timeout");
        // Čekaj max 75us dok DHT ne postavi stanje LOW
        CHECK_LOGE(dht_await_pin_state(pin, 75, 0, &high_duration),
                "HIGH bit timeout");

        // Koji bajt
        uint8_t b = i / 8;

        // Koji bit
        uint8_t m = i % 8;

        // Ako je bit početni od nekog bajta (MSB), postavi cijeli bajt inicijalno u 0
        if(!m)
            data[b] = 0;

        // Ako je vremenska duljina stanja 1 bila dulja nego 0, postavi bit 1 na na mjesto 7 - m, inače postavi 0 na mjesto 7 - m
        // radi se operacija OR sa inicijalno bajtom 0 (ako trenutno obrađujemo MSB) 
        data[b] |= (high_duration > low_duration) << (7 - m);
    }

    return ESP_OK;
}

/**
 * Pack two data bytes into single value and take into account sign bit.
 */

// Pretvori dva bajta u korisni 16 bitni broj
static inline int16_t dht_convert_data(dht_sensor_type_t sensor_type, uint8_t msb, uint8_t lsb) {
    int16_t data;

    // Ako je senzor tipa DHT11, vrati (data[0] * 10) ili (data[2] * 10)
    if(sensor_type == DHT_TYPE_DHT11) {
    
        data = msb * 10;
    
    } else { // u suprotnom
    
        // Postavi data[0] odnosno data[2] kao [0x000000000XXXXXXX]
        data = msb & 0x7F;

        // Pomakni ih kao [0x0XXXXXX00000000]
        data <<= 8;

        // OR sa data[1] ili data[3] (ako je sve prošlo pravilno): [0x0XXXXXXX00000000 OR 0x00000000] = [0x0XXXXXXX00000000]
        data |= lsb;

        // Ako je data[0] ili data[2] oblika: [0x1XXXXXXXX] onda je konačni podatak: - podatak (po 2 komplementu)
        if (msb & BIT(7))
            data = -data;       // convert it to negative
    
    }

    return data;
}


// Čitaj svježe (neobrađene podatke)
esp_err_t dht_read_data(dht_sensor_type_t sensor_type, gpio_num_t pin, int16_t *humidity, int16_t *temperature) {

    // Provjeri da nisu NULL
    CHECK_ARG(humidity || temperature);

    uint8_t data[DHT_DATA_BYTES] = { 0 };

    // Postavi izlazni način rada
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 1);

    // Zapocni kriticnu sekciju (u slučaju da imamo više task-ova koji čitaju sa istog DHT-a) i dohvati podatke
    PORT_ENTER_CRITICAL();
    esp_err_t result = dht_fetch_data(sensor_type, pin, data);
    if(result == ESP_OK)
        PORT_EXIT_CRITICAL();

    // Vrati natrag izlazni način rada, nakon što si isčitao podatke
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 1);

    // Ispitaj jeli prošlo sve u redu s čitanjem
    if(result != ESP_OK)
        return result;

    // Ispitaj jeli dobar checksum
    if(data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)){

        ESP_LOGE(TAG, "Checksum failed, invalid data received from sensor");
        return ESP_ERR_INVALID_CRC;
    
    }

    // Pretvori isčitane podatke u korisne podatke
    if(humidity)
        *humidity = dht_convert_data(sensor_type, data[0], data[1]);
    if(temperature)
        *temperature = dht_convert_data(sensor_type, data[2], data[3]);

    ESP_LOGD(TAG, "Sensor data: humidity=%d, temp=%d", *humidity, *temperature);

    return ESP_OK;
}

// Čitaj podatke kao realne brojeve
esp_err_t dht_read_float_data(dht_sensor_type_t sensor_type, gpio_num_t pin, float *humidity, float *temperature) {

    // Provjeri da nisu NULL
    CHECK_ARG(humidity || temperature);

    int16_t i_humidity, i_temp;

    // Čitaj svježe podatke
    esp_err_t res = dht_read_data(sensor_type, pin, humidity ? &i_humidity : NULL, temperature ? &i_temp : NULL);

    // Ispitaj jeli prošlo sve u redu s čitanjem
    if(res != ESP_OK)
        return res;

    // Pretvori podatke u float tipa
    if(humidity)
        *humidity = i_humidity / 10.0;
    if(temperature)
        *temperature = i_temp / 10.0;

    return ESP_OK;

}