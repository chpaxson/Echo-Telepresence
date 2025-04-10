/*******************************************************************************
* SIMPLE FOC Inspired RP2040 Based Motor Controller
*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"
#include "stepper.h"

#include "MT6701.h"

/*******************************************************************************
* Pin Definitions
*/

// Stepper constants.
stepper_t stepper;
const uint8_t stepper_pin_1A = 6;
const uint8_t stepper_pin_1B = 7;
const uint8_t stepper_pin_2A = 8;
const uint8_t stepper_pin_2B = 9;
const uint16_t stepper_steps_per_revolution = 200;
const stepper_mode_t stepping_mode = power;
uint8_t speed = 20;


/*******************************************************************************
* Main
*/

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);  // Initialize I2C with 100kHz clock
    adc_init();

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    adc_gpio_init(26); // Current A input 0
    adc_gpio_init(27); // Current B input 1
    adc_gpio_init(28); // MT6701 input 2
    adc_gpio_init(29); // VBUS input 3

    // Initialise the stepper
    stepper_init(&stepper, stepper_pin_1A, stepper_pin_1B,
            stepper_pin_2A, stepper_pin_2B,
            stepper_steps_per_revolution, stepping_mode);
    stepper_set_speed_rpm(&stepper, speed);

    sleep_ms(5000);
    printf("Start... \n");

    uint8_t data[2] = {0};
    uint16_t VBUS = 0;
    float angle = 0;

    //scans for i2c address
    i2c_scan();

    while (1) {
        adc_select_input(3);
        VBUS = adc_read();  // 12-bit value (0–4095)
        angle = angle_read(data);
  
        printf("Angle: %.2f | Angle Analog Val: %.2f| V_BUS Analog Val: %u \r\n", angle, analog_angle_read(), VBUS);


        // Rotate 3/4 of a turn.
        stepper_rotate_degrees(&stepper, 270);
        sleep_ms(500);

        // Now rotate these many steps in the oposite direction
        stepper_rotate_steps(&stepper, -45);
        sleep_ms(500);

        // Increase the speed and rotate 360 degrees
        speed = 50;
        stepper_set_speed_rpm(&stepper, speed);
        stepper_rotate_degrees(&stepper, 360);

        // Release the coils and sleep for a while. You can check that
        // the coils are not energised by moving the rotor manually:
        // there should be little resistance.
        stepper_release(&stepper);
        sleep_ms(4000);

        // Decrease the speed
        speed = 15;
        stepper_set_speed_rpm(&stepper, speed);

        sleep_ms(10);  // Read every second
    }

   return 0;
}