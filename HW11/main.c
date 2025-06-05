/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "hardware/gpio.h" 
#include "pico/stdio_usb.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */

#define PIN_NUM 15
#define UP_PIN 16
#define DOWN_PIN 17
#define LEFT_PIN 18
#define RIGHT_PIN 19
#define MODE_PIN 20

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

typedef struct {

    uint8_t pin;
    bool is_pressed;
    uint32_t press_start_time;
} ButtonState;

int get_speed(uint32_t held_time_ms) {
    if (held_time_ms < 500) return 1;
    else if (held_time_ms < 1000) return 3;
    else if (held_time_ms < 2000) return 5;
    else return 7;
}


void loop(ButtonState* buttons, int num_buttons) {
    for (int i = 0; i < num_buttons; i++) {
        ButtonState* button = &buttons[i];
        // if (!gpio_is_valid(button->pin)) {
        //     continue; // skip invalid GPIO pins
        // }
        if (gpio_get(button->pin) == 0) { // pressed (assuming active low)
            if (!button->is_pressed) {
                button->press_start_time = to_ms_since_boot(get_absolute_time());
                
            }


            button->is_pressed = true;

            // Apply speed to x/y based on direction
        } else {
            button->is_pressed = false;
        }
    }

    // Build mouse report from x/y
    // Send with tud_hid_mouse_report()
    
    sleep_ms(10); // control frame rate
}

#include <math.h>
void get_next_location(uint8_t angle, int* x, int* y) {
    // Calculate the next location based on the angle
    *x = (int)(cos((angle + 1) * M_PI / 180.0) * 10); // scale as needed
    *y = (int)(sin((angle + 1) * M_PI / 180.0) * 10);
}
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

void initialize_buttons(){
  gpio_init(UP_PIN);  
  gpio_set_dir(UP_PIN, GPIO_IN);    
  gpio_pull_up(UP_PIN); 
  
  gpio_init(LEFT_PIN);  
  gpio_set_dir(LEFT_PIN, GPIO_IN);    
  gpio_pull_up(LEFT_PIN);
  
  gpio_init(RIGHT_PIN);  
  gpio_set_dir(RIGHT_PIN, GPIO_IN);    
  gpio_pull_up(RIGHT_PIN);

  gpio_init(DOWN_PIN);  
  gpio_set_dir(DOWN_PIN, GPIO_IN);    
  gpio_pull_up(DOWN_PIN);

  gpio_init(MODE_PIN);  
  gpio_set_dir(MODE_PIN, GPIO_IN);    
  gpio_pull_up(MODE_PIN);
}

void circle_cursor() {
    static uint8_t angle = 0;
    static int x = 0, y = 0;

    for(int i = 0; i < 360; i++) {
        // Initialize buttons
      get_next_location(angle, &x, &y);

      // Send mouse report with the new coordinates
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0, x, y, 0, 0);

      // Increment angle for next call
      angle = (angle + 1) % 360; // Keep angle in [0, 359]
      }
    // Get the next location based on the angle

}
/*------------- MAIN -------------*/
int main(void)
{
  board_init();

  // while (!stdio_usb_connected()) {
  //           sleep_ms(100);
  // }
  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);
  gpio_init(PIN_NUM);
  gpio_set_dir(PIN_NUM, GPIO_IN);
  gpio_pull_up(PIN_NUM);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  int16_t dx = 0;
  int16_t dy = 0;
  int8_t mode = 0;
  struct {
      int8_t x;
      int8_t y;
    } report = { .x = 0, .y = 0 };

  ButtonState buttons[] = {
      {UP_PIN, false, 0},
      {DOWN_PIN, false, 0},
      {LEFT_PIN, false, 0},
      {RIGHT_PIN, false, 0},
      {MODE_PIN, false, 0}
  };

  int up_speed = 0, down_speed = 0, left_speed = 0, right_speed = 0;

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    if(gpio_get(MODE_PIN) == 1){
      gpio_put(PIN_NUM, 1); // Set the pin high
      circle_cursor();
      continue; // Skip the rest of the loop if in circle cursor mode
    }
    
    gpio_put(PIN_NUM, 0); // Set the pin low



    

    // Define speed (set to a default value, or calculate as needed)
    int speed = 1;
    loop(buttons, sizeof(buttons) / sizeof(buttons[0]));
    uint32_t now = to_ms_since_boot(get_absolute_time());
    if (buttons[0].is_pressed) {

      up_speed = get_speed(now - buttons[0].press_start_time);
    } else {
      up_speed = 0;
    }
    if (buttons[1].is_pressed) {

      down_speed = get_speed(now - buttons[1].press_start_time);
    } else {
      down_speed = 0;
    }
    if (buttons[2].is_pressed) {

      left_speed = get_speed(now - buttons[2].press_start_time);
    } else {
      left_speed = 0;
    }
    if (buttons[3].is_pressed) {

      right_speed = get_speed(now - buttons[3].press_start_time);
    } else {
      right_speed = 0;

    }

    // Use the calculated speeds for movement
    report.x = right_speed - left_speed;
    report.y = down_speed - up_speed;


    // Send report
    printf(" Left: %d, Right: %d, Up: %d, Down: %d\n", 
           left_speed, right_speed, up_speed, down_speed);
    tud_hid_mouse_report(REPORT_ID_MOUSE, 0, report.x, report.y, 0, 0);
    mode = gpio_get(PIN_NUM);
    

    hid_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn, int8_t dx, int8_t dy)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      //int8_t const delta = 5;

      // no button, right + down, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, dx, dy, 0, 0);
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      }else
      {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_MOUSE, btn, 5, 5); // was REPORT_ID_KEYBOARD

  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read(), 5, 5);
  }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
