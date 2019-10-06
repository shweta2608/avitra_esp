#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "init_sra.h"
#include "pin_defs.h"
#include "motor_commander.h"
#include "rosserial.h"

#define MOTOR_KP_STEP 0.005
#define MOTOR_KD_STEP 1.0
#define MAXPWM 65.0
#define MINPWM 35.0
#define ENCODING_FACTOR (MAXPWM)/40.0

motor_commander_t motor_F = (motor_commander_t) {.name = "MOTOR_F", .id = 0, .desr_rpm = 200, .Kp = 0.003,  .Kd = 0.75, .duty_cycle = 0,  \
                        .encoder = {.name = "MOTOR_F", .id = 0, .curr_rpm = 0, .ticks_count = 0, .enc_intr = ENCODER_F_A, .enc_dir = ENCODER_F_B},  \
                        .pwm_A = {.pwm_unit = MCPWM_UNIT_0, .pwm_timer = MCPWM_TIMER_0, .pwm_operator = MCPWM_OPR_A, .pwm_io_signals = MCPWM0A, .pwm_pin = MOTOR_F_PWM_A}, \
                        .pwm_B = {.pwm_unit = MCPWM_UNIT_0, .pwm_timer = MCPWM_TIMER_0, .pwm_operator = MCPWM_OPR_B, .pwm_io_signals = MCPWM0B, .pwm_pin = MOTOR_F_PWM_B}  \
                    };

motor_commander_t motor_L = (motor_commander_t) {.name = "MOTOR_L", .id = 1, .desr_rpm = 200, .Kp = 0.003,  .Kd = 0.75, .duty_cycle = 0,  \
                        .encoder = {.name = "MOTOR_L", .id = 1, .curr_rpm = 0, .ticks_count = 0, .enc_intr = ENCODER_L_A, .enc_dir = ENCODER_L_B},  \
                        .pwm_A = {.pwm_unit = MCPWM_UNIT_0, .pwm_timer = MCPWM_TIMER_1, .pwm_operator = MCPWM_OPR_A, .pwm_io_signals = MCPWM1A, .pwm_pin = MOTOR_L_PWM_A}, \
                        .pwm_B = {.pwm_unit = MCPWM_UNIT_0, .pwm_timer = MCPWM_TIMER_1, .pwm_operator = MCPWM_OPR_B, .pwm_io_signals = MCPWM1B, .pwm_pin = MOTOR_L_PWM_B}  \
                    };

motor_commander_t motor_R = (motor_commander_t) {.name = "MOTOR_R", .id = 2, .desr_rpm = 200, .Kp = 0.003,  .Kd = 0.75, .duty_cycle = 0, \
                        .encoder = {.name = "MOTOR_R", .id = 2, .curr_rpm = 0, .ticks_count = 0, .enc_intr = ENCODER_R_A, .enc_dir = ENCODER_R_B},  \
                        .pwm_A = {.pwm_unit = MCPWM_UNIT_1, .pwm_timer = MCPWM_TIMER_0, .pwm_operator = MCPWM_OPR_A, .pwm_io_signals = MCPWM0A, .pwm_pin = MOTOR_R_PWM_A}, \
                        .pwm_B = {.pwm_unit = MCPWM_UNIT_1, .pwm_timer = MCPWM_TIMER_0, .pwm_operator = MCPWM_OPR_B, .pwm_io_signals = MCPWM0B, .pwm_pin = MOTOR_R_PWM_B}  \
                    };

motor_commander_t motor_B = (motor_commander_t) {.name = "MOTOR_B", .id = 3, .desr_rpm = 200, .Kp = 0.003,  .Kd = 0.75, .duty_cycle = 0,  \
                        .encoder = {.name = "MOTOR_B", .id = 3, .curr_rpm = 0, .ticks_count = 0, .enc_intr = ENCODER_B_A, .enc_dir = ENCODER_B_B},  \
                        .pwm_A = {.pwm_unit = MCPWM_UNIT_1, .pwm_timer = MCPWM_TIMER_1, .pwm_operator = MCPWM_OPR_A, .pwm_io_signals = MCPWM1A, .pwm_pin = MOTOR_B_PWM_A}, \
                        .pwm_B = {.pwm_unit = MCPWM_UNIT_1, .pwm_timer = MCPWM_TIMER_1, .pwm_operator = MCPWM_OPR_B, .pwm_io_signals = MCPWM1B, .pwm_pin = MOTOR_B_PWM_B}  \
                    };

void ticks_publisher(){
    while(true){
        rosserial_publish(&motor_L.encoder.total_ticks, &motor_R.encoder.total_ticks);
        motor_L.encoder.total_ticks = 0;
        motor_R.encoder.total_ticks = 0;
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void bot_motion(){
    float pwm_l = 0;
    float pwm_r = 0;
    while(true){
        rosserial_subscribe(&pwm_l, &pwm_r);
        motor_L.duty_cycle = ENCODING_FACTOR * pwm_l;
        motor_R.duty_cycle = ENCODING_FACTOR * pwm_r;
        write_duty_cycle(&motor_L);
        write_duty_cycle(&motor_R);
        write_duty_cycle(&motor_F);
        write_duty_cycle(&motor_B);
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

void app_main(){
    rosserial_setup();
    init_motor(&motor_F);
    init_motor(&motor_L);
    init_motor(&motor_B);
    init_motor(&motor_R);
    xTaskCreatePinnedToCore(ticks_publisher, "ticks_publisher", 8192, NULL, 22, NULL, 0);
    xTaskCreatePinnedToCore(bot_motion, "teleop_subscriber", 8192, NULL, 23, NULL, 1);
    while(true);
}