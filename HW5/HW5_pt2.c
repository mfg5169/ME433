#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <stdint.h>
#include <math.h>
// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

#define PIN_RAM_CS 5






void print_binary(unsigned char byte);

void print_binary(unsigned char byte) {
    for (int i = 7; i >= 0; i--) {
        printf("%d", (byte >> i) & 1);
    }
}


union FloatInt {
    float f;
    uint32_t i;
};
uint32_t float_to_int(float f) {
    union FloatInt fi;
    fi.f = f;
    return fi.i;
}
float int_to_float(uint32_t i) {
    union FloatInt fi;
    fi.i = i;
    return fi.f;
}

void arithmetic(){
    volatile float f1, f2;
    printf("Enter two floats to use:\n");
    scanf("%f %f", &f1, &f2);

    volatile float f_add, f_sub, f_mult, f_div;
    const int iterations = 1000;

    uint64_t t_start, t_end, elapsed;
    double cycles_per_us = 150.0; // 150 MHz → 150 cycles per microsecond

    // Addition
    absolute_time_t t1 = get_absolute_time();
    for (int i = 0; i < iterations; i++) {
        f_add = f1 + f2;
    }
    t_end = to_us_since_boot(get_absolute_time());
    t_start = to_us_since_boot(t1);
    elapsed = t_end - t_start;
    printf("Addition: %.2f cycles per op\n", (elapsed * cycles_per_us) / iterations);

    // Subtraction
    t1 = get_absolute_time();
    for (int i = 0; i < iterations; i++) {
        f_sub = f1 - f2;
    }
    t_end = to_us_since_boot(get_absolute_time());
    t_start = to_us_since_boot(t1);
    elapsed = t_end - t_start;
    printf("Subtraction: %.2f cycles per op\n", (elapsed * cycles_per_us) / iterations);

    // Multiplication
    t1 = get_absolute_time();
    for (int i = 0; i < iterations; i++) {
        f_mult = f1 * f2;
    }
    t_end = to_us_since_boot(get_absolute_time());
    t_start = to_us_since_boot(t1);
    elapsed = t_end - t_start;
    printf("Multiplication: %.2f cycles per op\n", (elapsed * cycles_per_us) / iterations);

    // Division
    t1 = get_absolute_time();
    for (int i = 0; i < iterations; i++) {
        f_div = f1 / f2;
    }
    t_end = to_us_since_boot(get_absolute_time());
    t_start = to_us_since_boot(t1);
    elapsed = t_end - t_start;
    printf("Division: %.2f cycles per op\n", (elapsed * cycles_per_us) / iterations);

    // Print results
    printf("\nResults: \n%f + %f = %f \n%f - %f = %f \n%f * %f = %f \n%f / %f = %f\n",
           f1, f2, f_add, f1, f2, f_sub, f1, f2, f_mult, f1, f2, f_div);


}


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

void setup_sequencial(){
    // Setup sequencial mode
    uint8_t setup_command[2] = {0x01, 0x40}; // Example setup command, adjust as needed
    cs_select(PIN_RAM_CS);
    spi_write_blocking(SPI_PORT, setup_command, 2);
    cs_deselect(PIN_RAM_CS);
}

uint16_t writetoRAM(uint16_t address, uint32_t *data, size_t len){

    uint8_t write_command = 0x02;

    // Check data range
    cs_select(PIN_RAM_CS);
    // for (size_t i = 0; i < len; i++) {
    //     uint32_t int_value = float_to_int(data[i]);
    //     spi_write_blocking(SPI_PORT, &write_command, 1);
    //     spi_write_blocking(SPI_PORT, (uint8_t*)&address, sizeof(address));
    //     spi_write_blocking(SPI_PORT, (uint8_t*)&int_value, sizeof(int_value));
    //     address += sizeof(float);


    // }



    spi_write_blocking(SPI_PORT, &write_command, 1);
    spi_write_blocking(SPI_PORT, (uint8_t*)&address, sizeof(address));
    spi_write_blocking(SPI_PORT, (uint8_t*)data, len * sizeof(uint32_t));
    cs_deselect(PIN_RAM_CS);

    return address;


}

uint32_t* readfromRAM(uint16_t address) {
    uint8_t read_command = 0x03;

    static uint32_t data[400]; // Use static to persist after function returns

    cs_select(PIN_RAM_CS);

    spi_write_blocking(SPI_PORT, &read_command, 1);
    spi_write_blocking(SPI_PORT, (uint8_t*)&address, sizeof(address));
    spi_read_blocking(SPI_PORT, 0, (uint8_t*)data, 400 * sizeof(uint32_t));

    cs_deselect(PIN_RAM_CS);

    return data;
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

int main()
{


    while (!stdio_usb_connected()) {
            sleep_ms(100);
     }
    printf("Start!\n");
    stdio_init_all();
    arithmetic();
    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_RAM_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_RAM_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_RAM_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_RAM_MOSI, GPIO_FUNC_SPI); 
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    gpio_set_dir(PIN_RAM_CS, GPIO_OUT);
    gpio_put(PIN_RAM_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi
    float t = 0;
    uint direction = 1;
    float v1 = 0;
    float v0 = 0;
    float f = 2;
    
    //float step = (3.3 / 10);
    float step = (3.3 / 5);

    uint32_t v[1000];
    float v2[1000];



    printf("Hello, world!\n");
    sleep_ms(1000);

        // adc_init(); // init the adc module
        // adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
        // adc_select_input(0); // select to read from ADC0

    // Generate 400 samples of a sine wave and store as uint32_t (float bits)
    for (uint t = 0; t < 1000; t++) {
        v0 = 1.65 * sinf(2 * M_PI * f * t / 1000.0f) + 1.65;
        v[t] = float_to_int(v0); // Store bitwise float as uint32_t
    }

    arithmetic();

    setup_sequencial();
    uint16_t address = 0x0000; // Starting address in RAM
    address = writetoRAM(address, v, 1000); // Write the data to RAM
    printf("Data written to RAM at address: 0x%04X\n", address);
    // Read the data back from RAM

    printf("Data read from RAM: \n");
    
    for (int i = 0; i < 1000; i++) {
        printf("Value %d: %f\n", i, read_data[i]);
    }
    uint32_t *read_data = readfromRAM(address); // Read the data from RAM
    // Write to DAC
    for (int i = 0; i < 1000; i++) {
        writeDAC(0, int_to_float(read_data[i])); // Write each value to DAC channel 0
        sleep_ms(1); // Delay to allow DAC to process the value
    }
    printf("DAC write complete.\n");
    // Write to DAC
}


