#include "driver/gpio.h"
#include "servo.h"
#include "esp_err.h"

#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"



/* Inicijalizacija PWM-a */
void servo_setup(int pin) {

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, (gpio_num_t)pin);    

    mcpwm_config_t pwm_config;
    pwm_config.frequency = 50;                                  // PWM frekvencija
    pwm_config.cmpr_a = 0;                                      // Početna duljina vremena trajanja stanje visoko 
    pwm_config.counter_mode = MCPWM_UP_COUNTER;                 // Brojilo broji od 0
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;                   // Vrijeme trajanja (duty cycle) se odnosi na stanje visoko
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    /* Postavi početni položaj servo-a */
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 5);

}

/* Postavi poziciju servomotora */
esp_err_t servo_position(float angle) {

    /* 0-180 stupnjeva u 5% do 10% ciklusa tranjanja visokog stanja */
    float duty_cycle = (((angle / 180.00) / 20.00  + 0.05) * 100.00);
    printf("Duty Cycle: %.2f %%\n", duty_cycle);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);

    return ESP_OK;

}