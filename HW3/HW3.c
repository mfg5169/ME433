#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define LED_NUM 15
#define BUT_NUM 14


int main()
{
    stdio_init_all();
    gpio_init(LED_NUM);
    gpio_init(BUT_NUM);
    gpio_set_dir(LED_NUM, GPIO_OUT);
    gpio_set_dir(BUT_NUM, GPIO_IN);
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    printf("Start!\n");

    while(!gpio_get(BUT_NUM)) {
        printf("Waiting for button press...\n");
        sleep_ms(100);
    }
    printf("Button pressed!\n");
    gpio_put(LED_NUM,1);
    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0
 
    while (1) {
        char message[100];
        printf("How many sample would you like to read: ");
        scanf("%s", message);
        printf("message: %s\r\n",message);
        sleep_ms(50);

        

        int count = atoi(message);

        printf("count: %d\r\n", count);


        if (count > 100 || count <= 0) {
            printf("Too many samples requested. Please enter a number less than or equal to 100.\n");
            continue;
        }
        
        int num_samples = 0;

        while (num_samples < count){
            uint16_t result = adc_read();
            printf("ADC0 read: %d\n", result);
            sleep_ms(10);
            num_samples++;
        }
    }
}
