#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include <math.h>
#include "hardware/adc.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19




static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

void writeDAC(int, float);

void print_binary(unsigned char byte);

void print_binary(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}


int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 10000);
    adc_init();

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi
    adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0
    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);

        while (!stdio_usb_connected()) {
            sleep_ms(100);
        }
        printf("Start!\n");

        // adc_init(); // init the adc module
        // adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
        // adc_select_input(0); // select to read from ADC0


    
        float t = 0;
        int direction = 1;
        float v1 = 0;
        float v0 = 0;
        float f = 2;
        float tm = 20;
        //float step = (3.3 / 10);
        float step = 3.3f / (50);

        
        for (int i = 0; i < 100; i++) {
                printf("\n=====================================\n");
    
                v0 = 1.65 * sin(2 * M_PI * f * t) + 1.65;
                
                v1 += (step * direction);

                printf("At time %f\n", t);
                printf("step: %f\n", step);

                printf("v1 triangle wave with direction %d: %f \nv0 sin wave: %f\n", direction, v1,  v0);
                
                if ((v1 > 3.3) || (v1 < 0)) {
                    direction = -direction;
                }

                printf("Writing to DAC...\n");
                writeDAC(0, v0);

                
                t += 0.01;
                writeDAC(1, v1);

                printf("Reading ADC0...\n");

                uint16_t result = adc_read();
                float adc_voltage = result * 3.3f / 4095.0f;
                printf("ADC0 voltage: %f V\n", adc_voltage);
                //printf("ADC0 read: %d\n", result);
                printf("\n=====================================\n");
                sleep_ms(10);

            }
            sleep_ms(1000);
        }

}

void writeDAC(int channel, float voltage)
{


    printf("Writing to DAC..%d\n", channel);

    uint8_t data[2] = {0, 0};
    data[0] |= (channel << 7); // Command byte
    data[0] |= (0b111 << 4); // Data bits
    uint16_t v_scale = voltage * 1023 / 3.3;

    uint8_t last_six = (v_scale & 0x3F) << 2;
    uint8_t first_four = (v_scale >> 6) & 0x0F;

    data[0] |= first_four;
    data[1] |= last_six;

    printf("Data to be sent: ");
    print_binary(data[0]);
    printf("\n");
    printf("second byte index: ");
    print_binary(data[1]);
    printf("\n");



    cs_select(PICO_DEFAULT_SPI_CSN_PIN);
    spi_write_blocking(spi_default, data, sizeof(data));
    // spi_write_blocking(spi_default, (uint8_t *)&data, sizeof(data));
    cs_deselect(PICO_DEFAULT_SPI_CSN_PIN);
}
