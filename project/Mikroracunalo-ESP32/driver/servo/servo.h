#ifndef __SERVO_H__
#define __SERVO_H__

#include "esp_err.h"
#include "driver/ledc.h"

#define SERVO_PWM_CHANNEL               LEDC_CHANNEL_0          // PWM kanal
#define SERVO_PWM_FREQUENCY             50                      // PWM frekvencija 50Hz
#define SERVO_PWM_TIMER                 LEDC_TIMER_0            // Koji timer koristiti
#define SERVO_PWM_RESOLUTION            LEDC_TIMER_12_BIT       // Rezolucija 

void servo_setup(int pin);
esp_err_t servo_position(float angle);

#endif