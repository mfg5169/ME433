#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 12
#define I2C_SCL 13
#define ADDR 0x20 // MCP23017 I2C address (A0, A1, A2 all low)

// Register addresses
#define IODIR 0x00
#define IPOL 0x01
#define GPINTEN 0x02
#define DEFVAL 0x03
#define INTCON 0x04
#define IOCON 0x05
#define GPPU 0x06
#define INTF 0x07
#define INTCAP 0x08
#define GPIO 0x09
#define OLAT 0x0A

void i2c_initialize()
{
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    //uint8_t buf[2] = {IODIR, 0x80};
    uint8_t buf[2] = {IODIR, 0x01}; // Set all pins as inputs (0x7F = 01111111)
    printf("I2C Initialized IODIR\n");
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    printf("WRITE TO MCP23017\n");
    i2c_write_blocking(i2c_default, ADDR, buf, 2, false);


}
int pico_led_init(void) {
    #if defined(PICO_DEFAULT_LED_PIN)
        // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
        // so we can use normal GPIO functionality to turn the led on and off
        gpio_init(PICO_DEFAULT_LED_PIN);
        gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
        return PICO_OK;
    #elif defined(CYW43_WL_GPIO_LED_PIN)
        // For Pico W devices we need to initialise the driver etc
        return cyw43_arch_init();
    #endif
    }
int main()
{
    stdio_init_all();
    int rc = pico_led_init();
    if (rc != PICO_OK) {
        printf("LED init failed\n");
        return rc;
    }

    while (!stdio_usb_connected()) {
            sleep_ms(100);
    }
    printf("I2C MCP23017 Example\n");
    printf("Press the button connected to pin 7 to turn on the LED connected to pin 7.\n");
    printf("Press Ctrl+C to exit.\n");
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    uint8_t reg = GPIO;
    uint8_t buf = 0x00;
    uint8_t turn_on[2] = {OLAT, 0x80};
    uint8_t turn_off[2] = {OLAT, 0x00}; 
    i2c_initialize();
    printf("FINISHED INITIALIZATION\n");
    while (true) {

        while(true){
            i2c_write_blocking(i2c_default, ADDR, &reg, 1, true);  // read from GPIO true to keep master control of bus
            i2c_read_blocking(i2c_default, ADDR, &buf, 1, false);  // false - finished with bus
            printf("Read GPIO: 0x%02X\n", buf);
            printf("Button state: %s\n", (buf & 0x01) ? "Not Pressed" : "Pressed");
            
            if (!(buf & 0x01)) { // Check if button is pressed (bit 0 is low)
                break;
            }
            turn_on[2] = {OLAT, 0x80}; // Set pin 7 high (0x80 = 10000000)
            i2c_write_blocking(i2c_default, ADDR, turn_on, 2, false);
            printf("Button Not Pressed\n");
            sleep_ms(100);
        }

        i2c_write_blocking(i2c_default, ADDR, turn_off, 2, false);
        printf("Button Not Not Pressed\n");
        pico_set_led(true); // Turn off the LED
            
        while(true){
            i2c_write_blocking(i2c_default, ADDR, &reg, 1, true);  // read from GPIO true to keep master control of bus
            i2c_read_blocking(i2c_default, ADDR, &buf, 1, false);  // false - finished with bus
            printf("Read GPIO: 0x%02X\n", buf);
            printf("Button state: %s\n", (buf & 0x01) ? "Not Pressed" : "Pressed");
            if (!(buf & 0x01)) { // Check if button is pressed (bit 0 is low)
                break;
            }
            turn_on[2] = {OLAT, 0x80}; // Set pin 7 high (0x80 = 10000000)
            i2c_write_blocking(i2c_default, ADDR, turn_on, 2, false);
            printf("Button Not Pressed\n");
            sleep_ms(100);
        }

        i2c_write_blocking(i2c_default, ADDR, turn_off, 2, false);
        printf("Next Loop!\n");
        pico_set_led(false); // Turn off the LED

        sleep_ms(100);
    }
}
