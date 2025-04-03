#include <stdio.h>
#include "pico/stdlib.h"
#include "stepper.h"

stepper_t stepper;
const uint8_t stepper_pin_1A = 6;  // NOTE [1]
const uint8_t stepper_pin_1B = 7;
const uint8_t stepper_pin_2A = 8;
const uint8_t stepper_pin_2B = 9;
const uint16_t stepper_steps_per_revolution = 200;  // NOTE [2]
const stepper_mode_t stepping_mode = single;  // NOTE [3]

int main() {
    stdio_init_all();
    stepper_init(&stepper, stepper_pin_1A, stepper_pin_1B,
                 stepper_pin_2A, stepper_pin_2B,
                 stepper_steps_per_revolution, stepping_mode);
    stepper_set_speed_rpm(&stepper, 20);  // NOTE [4]

    while (true) {
        stepper_rotate_steps(&stepper, 200);  // NOTE [5]
        sleep_ms(1000);
        stepper_rotate_degrees(&stepper, -90);
        sleep_ms(1000);
    }
    return 0;
}