#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "font.h"   
#include "ssd1306.c"
#include "hardware/adc.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

#define ADOPIN 26


int WriteMessage(unsigned char x, unsigned char y, char *message);

void i2c_initialize()
{
    printf("I2C Initialisation\n");
    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    printf("WRITE TO MCP23017\n");



}

int main()
{
    stdio_init_all();
    while (!stdio_usb_connected()) {
            sleep_ms(100);
    }
    printf("Start!\n");
    printf("INITIALIZE ADC\n");
    adc_init(); 

    printf("ADC Initialised\n");
    adc_gpio_init(26);
    adc_select_input(0);
    ptintf("ADC GPIO Initialised\n");

    printf("ADC GPIO Initialised\n");
     
    // I2C Initialisation. Using it at 400Khz.
    i2c_initialize();

    printf("SSD1306 Initialisation\n");
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c
    ssd1306_setup();
    printf("SSD1306 Initialised\n");
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    while (true) {
        ssd1306_clear();
        int i;
        char message[50]; 


        gpio_put(PICO_DEFAULT_LED_PIN, 1); // Turn LED on
        // ssd1306_drawPixel(3, 4, 1);
        // ssd1306_update();
        // printf("Adding Pixel");

        
        sleep_ms(250);
        gpio_put(PICO_DEFAULT_LED_PIN, 0); // Turn LED off


        // ssd1306_drawPixel(3, 4, 0);
        // ssd1306_update();
        // printf("Removing Pixel");
        
        
        

        sleep_ms(250);
        printf("spribtf!\n");
        sleep_ms(1000);
        adc_select_input(0);
        unsigned int t = to_us_since_boot(get_absolute_time());  

        for(int j = 0; j < 1000; j++)
        {
   
            uint16_t = adc_read();
            float V = adcV*(3.3/4095.0);
            sprintf(message, " ADC Read: %0.2f V", V);
            printf("ADC %d: %f V\n", j, V);
            // printf("Enter an integer: ");
            // scanf("%d", &i); // read an integer from the user
            //sprintf(message, "my var = %d", i); 
            WriteMessage(50,20,message); // draw starting at x=20,y=10 
            ssd1306_update();



        }

        ssd1306_clear();







        unsigned int t2 = to_us_since_boot(get_absolute_time());  
        unsigned int differential = t2 - t;
        uint64_t fps = (uint64_t)(1/((float)(differential/1000000.0)));// turn time to frames per second

        sprintf(message, "%u fps", fps);  
        WriteMessage(10,15, message);
 
        sleep_ms(1000);
        ssd1306_update();

    }
}

int WriteMessage(unsigned char x, unsigned char y, char *message)
{
    ssd1306_clear();
    int frames = 0;
    for(int i = 0; message[i] != '\0'; i++)
    {

        const char *rep = ASCII[message[i] - 0x20];

        for(int j = 0; j < 5; j++)
        {
            for(int k = 0; k < 8; k++)
            {
                int bit = (rep[j] >> k) & 1;
                if(bit)
                {
                    ssd1306_drawPixel(x + j, y + k, 1);
                    ssd1306_update();
                    frames++;
                
                }
                else
                {
                    ssd1306_drawPixel(x + j, y + k, 0);
                }


            };
        };

        x += 6;
        if(x > 127)
        {
            x = 0;
            y += 8;
        };




    };
    ssd1306_update();
    return frames;
}
