#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#define LEDPin 25 // the built in LED on the Pico
#define TIME 4000
#define WRAP 300000 // when to rollover, must be less than 65535

#define IS_RGBW false
#define NUM_PIXELS 4//150

#ifdef PICO_DEFAULT_WS2812_PIN
#define WS2812_PIN PICO_DEFAULT_WS2812_PIN
#else
// default to pin 2 if the board doesn't have a default WS2812 pin defined
#define WS2812_PIN 2
#endif

// Check the pin is compatible with the platform
#if WS2812_PIN >= NUM_BANK0_GPIOS
#error Attempting to use a pin>=32 on a platform that does not support it
#endif

static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return
            ((uint32_t) (r) << 16) |
            ((uint32_t) (g) << 8) |
            (uint32_t) (b);
}

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} wsColor; 

wsColor HSBtoRGB(float hue, float sat, float brightness) {
    float red = 0.0;
    float green = 0.0;
    float blue = 0.0;

    if (sat == 0.0) {
        red = brightness;
        green = brightness;
        blue = brightness;
    } else {
        if (hue == 360.0) {
            hue = 0;
        }

        int slice = hue / 60.0;
        float hue_frac = (hue / 60.0) - slice;

        float aa = brightness * (1.0 - sat);
        float bb = brightness * (1.0 - sat * hue_frac);
        float cc = brightness * (1.0 - sat * (1.0 - hue_frac));

        switch (slice) {
            case 0:
                red = brightness;
                green = cc;
                blue = aa;
                break;
            case 1:
                red = bb;
                green = brightness;
                blue = aa;
                break;
            case 2:
                red = aa;
                green = brightness;
                blue = cc;
                break;
            case 3:
                red = aa;
                green = bb;
                blue = brightness;
                break;
            case 4:
                red = cc;
                green = aa;
                blue = brightness;
                break;
            case 5:
                red = brightness;
                green = aa;
                blue = bb;
                break;
            default:
                red = 0.0;
                green = 0.0;
                blue = 0.0;
                break;
        }
    }

    unsigned char ired = red * 255.0;
    unsigned char igreen = green * 255.0;
    unsigned char iblue = blue * 255.0;

    wsColor c;
    c.r = ired;
    c.g = igreen;
    c.b = iblue;
    return c;
}

void gpio_init(){
    gpio_set_function(LEDPin, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(LEDPin); // Get PWM slice number
    float div = 10; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // divider
    //Suint16_t wrap = 300000; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, WRAP);
    pwm_set_enabled(slice_num, true); // turn on the PWM

    pwm_set_gpio_level(LEDPin, WRAP * 0.025); // set the duty cycle to 50%
}

void update_Pixels(uint8_t hue[], uint8_t sat[], uint8_t bri[], PIO pio, int sm){
    for(int i=0;i<NUM_PIXELS;i++){
        wsColor c = HSBtoRGB(hue[i], sat[i], bri[i]); // convert HSB to RGB
        put_pixel(pio, sm, urgb_u32(c.r, c.g, c.b)); // use fields of c directly
        hue[i] = (hue[i] + 1) % 360; // increment the hue safely
    }
}
int main()
{
    stdio_init_all();
    gpio_init(); // Initialize the GPIO for the LED

    float incr = 0.1f / (TIME / 2); // increment for the duty cycle, use float division

    uint8_t hue[NUM_PIXELS] = {0, 90, 180, 270}; // hue values for the pixels
    uint8_t sat[NUM_PIXELS] = {1, 1, 1, 1}; // saturation values for the pixels
    uint8_t bri[NUM_PIXELS] = {1, 1, 1, 1}; // brightness values for the pixels
    int time = 5; // time variable for the loop

    // Initialize PIO and state machine for WS2812
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    while (true) {
        printf("Hello, world!\n");
        for (int i = 0; i < TIME / 2; i++) {
            float duty = 0.025f + (i+1)*incr;
            if (duty > 1.0f) duty = 1.0f;
            pwm_set_gpio_level(LEDPin, (uint16_t)(WRAP * duty)); // set the duty cycle
            update_Pixels(hue, sat, bri, pio, sm); // update the pixels
            sleep_ms(80);
            //incr += 0.0001;
        }

        for (int i = 0; i < TIME / 2; i++) {
            float duty = 0.125f - (i+1)*incr;
            if (duty < 0.0f) duty = 0.0f;
            pwm_set_gpio_level(LEDPin, (uint16_t)(WRAP * duty)); // set the duty cycle
            update_Pixels(hue, sat, bri, pio, sm); // update the pixels
            sleep_ms(80);
            //incr += 0.0001;
        }


        sleep_ms(80); // wait for 1 second
    }
}
