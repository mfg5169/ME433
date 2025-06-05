/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"

#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#define GPIO_WATCH_PIN 14
#define SECOND_LED_PIN 15


static char event_str[128];
static int HL_COUNT = 0;

void gpio_event_string(char *buf, uint32_t events);





static const char *gpio_irq_str[] = {
    "LEVEL_LOW",  // 0x1
    "LEVEL_HIGH", // 0x2
    "EDGE_FALL",  // 0x4
    "EDGE_RISE"   // 0x8
};

void gpio_event_string(char *buf, uint32_t events) {
for (uint i = 0; i < 4; i++) {
    uint mask = (1 << i);
    if (events & mask) {
        // Copy this event string into the user string
        const char *event_str = gpio_irq_str[i];
        while (*event_str != '\0') {
            *buf++ = *event_str++;
        }
        events &= ~mask;

        // If more events add ", "
        if (events) {
            *buf++ = ',';
            *buf++ = ' ';
        }
    }
}
*buf++ = '\0';
}

// Set an LED_TYPE variable - 0 is default, 1 is connected to WIFI chip
// Note that LED_TYPE == 1 is only supported when initially compiled for
// a board with PICO_CYW43_SUPPORTED (eg pico_w), else the required
// libraries won't be present
bi_decl(bi_program_feature_group(0x1111, 0, "LED Configuration"));
#if defined(PICO_DEFAULT_LED_PIN)
    // the tag and id are not important as picotool filters based on the
    // variable name, so just set them to 0
    bi_decl(bi_ptr_int32(0x1111, 0, LED_TYPE, 0));
    bi_decl(bi_ptr_int32(0x1111, 0, LED_PIN, PICO_DEFAULT_LED_PIN));
    bi_decl(bi_ptr_int32(0x1111, 0, LED2_PIN, SECOND_LED_PIN));
    

#elif defined(CYW43_WL_GPIO_LED_PIN)
    bi_decl(bi_ptr_int32(0x1111, 0, LED_TYPE, 1));
    bi_decl(bi_ptr_int32(0x1111, 0, LED_PIN, CYW43_WL_GPIO_LED_PIN));
#else
    bi_decl(bi_ptr_int32(0x1111, 0, LED_TYPE, 0));
    bi_decl(bi_ptr_int32(0x1111, 0, LED_PIN, 25));
#endif

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif

void gpio_callback(uint gpio, uint32_t events) {
    // Put the GPIO event(s) that just happened into event_str
    // so we can print it

    HL_COUNT++;
    if (HL_COUNT % 2 == 0) {
        gpio_put(LED_PIN, 1);
        gpio_put(LED2_PIN, 0);
    } else {
        gpio_put(LED_PIN, 0);
        gpio_put(LED2_PIN, 1);
    }
    gpio_event_string(event_str, events);
    printf("GPIO %d %s\n", gpio, event_str);
}
// Perform initialisation
int pico_led_init(void) {
    if (LED_TYPE == 0) {
        // A device like Pico that uses a GPIO for the LED so we can
        // use normal GPIO functionality to turn the led on and off
        gpio_init(LED_PIN);
        gpio_set_dir(LED_PIN, GPIO_OUT);
        gpio_init(LED2_PIN);
        gpio_set_dir(LED2_PIN, GPIO_OUT);
        return PICO_OK;
#ifdef CYW43_WL_GPIO_LED_PIN
    } else if (LED_TYPE == 1) {
        // For Pico W devices we need to initialise the driver etc
        return cyw43_arch_init();
#endif
    } else {
        return PICO_ERROR_INVALID_DATA;
    }
}

// // Turn the led on or off
// void pico_set_led(bool led_on,) {
//     if (LED_TYPE == 0) {
//         // Just set the GPIO on or off
//         gpio_put(LED_PIN, led_on);
// #ifdef CYW43_WL_GPIO_LED_PIN
//     } else if (LED_TYPE == 1) {
//         // Ask the wifi "driver" to set the GPIO on or off
//         cyw43_arch_gpio_put(LED_PIN, led_on);
// #endif
//     }
// }

int main() {

    //Initialize USB
    stdio_init_all();

    //Initialize PICO LED
    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);

    //Initialize GPIO
    gpio_init(GPIO_WATCH_PIN);

    //Set GPIO Detection
    gpio_set_irq_enabled_with_callback(GPIO_WATCH_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);


    
    while (true) {
        printf("button count: %d\n", HL_COUNT);
        sleep_ms(1000);

    }
}
