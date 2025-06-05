/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#define FLAG_VALUE 123

void gpio_setup() {
    // Set up GPIO pins
    gpio_init(15);
    gpio_set_dir(15, GPIO_OUT);
    gpio_put(15, 0); // Turn off the LED initially
}

void core1_entry() {

    multicore_fifo_push_blocking(FLAG_VALUE);

    uint32_t g = multicore_fifo_pop_blocking();
    gpio_setup();
    adc_init();
    adc_gpio_init(26); // Initialize GPIO 26 for ADC input (A0)
            // adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // Select ADC input 0 (A0)


    if (g != FLAG_VALUE)
        printf("Hmm, that's not right on core 1!\n");
    else
        printf("Its all gone well on core 1!");

    while (1)
    {
        g = multicore_fifo_pop_blocking();

        if (g == 0) {
            //printf("Core 1: %s\n", "voltage on pin A0");
            
            uint16_t result = adc_read();
            multicore_fifo_push_blocking(result); // Push the ADC result to core 0
            //printf("Core 1: ADC result: %d\n", result);
        } else if (g == 1) {
            //printf("Core 1: %s\n", "LED on GP15");
            gpio_put(15, 1); // Turn on the LED
        } else if (g == 2) {
            //printf("Core 1: %s\n", "LED off GP15");

            gpio_put(15, 0); // Turn off the LED
        } else {
            printf("Core 1: Invalid selection.\n");
        }

        //multicore_fifo_push_blocking(g);
    }
}

void core0_entry() {
    // This is the entry point for core 0
    // You can put any code you want here, but it will be run on core 0
    // This function is not required, but it can be useful for organization
    while (1)
        tight_loop_contents();
}

typedef struct {
    char item1[32];
    char item2[32];
    char item3[32];
} results_t;

results_t obj = {
    "voltage on pin A0",
    "LED on GP15",
    "LED off GP15"
};


int main() {
    stdio_init_all();


    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Hello, multicore!\n");
    /// \tag::setup_multicore[]

    multicore_launch_core1(core1_entry);

    // Wait for it to start up

    uint32_t g = multicore_fifo_pop_blocking();

    if (g != FLAG_VALUE)
        printf("Hmm, that's not right on core 0!\n");
    else {
        multicore_fifo_push_blocking(FLAG_VALUE);
        printf("It's all gone well on core 0!");
    }

    const char *exp;
    while (1) {
        sleep_ms(1000);

        printf("Press 0, 1, or 2 to continue...\n");
        scanf("%u", &g);
        printf("\n");
        multicore_fifo_push_blocking(g);

        if (g == 0){
            exp = obj.item1;
            uint32_t r = multicore_fifo_pop_blocking();
            float voltage = (r * 3.3f) / 4095.0f; // Convert ADC result to voltage
            printf("Core 0: %s, ADC result: %0.2f\n", exp, voltage);
        } else if (g == 1){
            exp = obj.item2;
            printf("Core 0: %s\n", exp);
        } else if (g == 2){
            exp = obj.item3;
            printf("Core 0: %s\n", exp);
        }
        else
            printf("Invalid selection.\n");
            continue;   

        

        // add a timer
        

        printf("It's all gone well on core 0!\n");
    }
       


}


    /// \end::setup_multicore[]

