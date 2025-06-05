#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdlib.h>


#define PWM_WRAP 10000 // max duty resolution
#define PWM_FREQ_HZ 1000
#define AIN1 2
#define AIN2 3

int duty_cycle = 0; // Range: -100 to +100

void setup_pwm(uint pin) {
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(pin);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_enabled(slice, true);
}

void update_motor(int duty) {
    int level = abs(duty) * PWM_WRAP / 100;
    uint slice1 = pwm_gpio_to_slice_num(AIN1);
    uint slice2 = pwm_gpio_to_slice_num(AIN2);

    if (duty > 0) {
        pwm_set_gpio_level(AIN1, level);
        pwm_set_gpio_level(AIN2, 0);
    } else if (duty < 0) {
        pwm_set_gpio_level(AIN1, 0);
        pwm_set_gpio_level(AIN2, level);
    } else {
        pwm_set_gpio_level(AIN1, 0);
        pwm_set_gpio_level(AIN2, 0);
    }
}

int main() {
    stdio_usb_init();
    gpio_set_function(AIN1, GPIO_FUNC_PWM);
    gpio_set_function(AIN2, GPIO_FUNC_PWM);
    setup_pwm(AIN1);
    setup_pwm(AIN2);

    printf("Motor Control Ready: use '+' or '-' to adjust speed.\n");

    while (true) {
        int c = getchar_timeout_us(0);
        if (c == '+') {
            if (duty_cycle < 100) duty_cycle++;
        } else if (c == '-') {
            if (duty_cycle > -100) duty_cycle--;
        }

        update_motor(duty_cycle);
        sleep_ms(50); // debounce input

        printf("Duty Cycle: %d%%\r", duty_cycle);
    }
}
