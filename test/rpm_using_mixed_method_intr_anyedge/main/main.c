#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "init_sra.h"
#include "pin_defs.h"
#include "motor.h"

motor_t motor_R = (motor_t) {.name = "MOTOR_R", .id = 2, .desr_rpm = 200, .Kp = 0.003,  .Kd = 0.75, .duty_cycle = 0, \
                        .encoder = {.name = "MOTOR_R", .id = 2, .curr_rpm = 0, .ticks_count = 0, .enc_intr1 = ENCODER_R_A, .enc_intr0 = ENCODER_R_B},  \
                        .pwm_A = {.pwm_unit = MCPWM_UNIT_1, .pwm_timer = MCPWM_TIMER_0, .pwm_operator = MCPWM_OPR_A, .pwm_io_signals = MCPWM0A, .pwm_pin = MOTOR_R_PWM_A}, \
                        .pwm_B = {.pwm_unit = MCPWM_UNIT_1, .pwm_timer = MCPWM_TIMER_0, .pwm_operator = MCPWM_OPR_B, .pwm_io_signals = MCPWM0B, .pwm_pin = MOTOR_R_PWM_B}  \
                    };

void initialize()
{
    init_motor(&motor_R);
    motor_R.duty_cycle = -50;
    write_duty_cycle(&motor_R);
}

void app_main()
{
    initialize();
    while(true){
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}