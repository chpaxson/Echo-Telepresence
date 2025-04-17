/*******************************************************************************
* SIMPLE FOC Inspired RP2040 Based Motor Controller
*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
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


/*******************************************************************************
* Main
*/

void core1_main() {

    printf("Entered core0 (core=%d)\n", get_core_num());

    // Initialise the stepper
    stepper_init(&stepper, stepper_pin_1A, stepper_pin_1B,
        stepper_pin_2A, stepper_pin_2B,
        stepper_steps_per_revolution, stepping_mode);

    while (1) {
        stepper_set_speed_rpm(&stepper, 50);
        stepper_rotate_degrees(&stepper, 270);
        sleep_ms(500);

        stepper_set_speed_rpm(&stepper, 150);
        stepper_rotate_degrees(&stepper, 360);

        stepper_release(&stepper);
        printf("Finished a stepper loop");
        sleep_ms(4000);
    }

}

int main() {
    stdio_init_all();
    
    sleep_ms(3000);

    printf("Entered core0 (core=%d)\n", get_core_num());

    // multicore_launch_core1(core1_main);

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

    sleep_ms(5000);
    printf("Start... \n");

    uint8_t data[2] = {0};
    uint16_t VBUS = 0;
    float angle = 0;
    uint16_t Acurrent = 0;
    uint16_t Bcurrent = 0;
    float AI = 0;
    float BI = 0;

    //scans for i2c address
    i2c_scan();

    while (1) {
        adc_select_input(3);
        VBUS = adc_read();  // 12-bit value (0–4095)
        angle = angle_read(data);
        adc_select_input(0);
        Acurrent = adc_read();
        adc_select_input(1);
        Bcurrent = adc_read();

        AI = (float)Acurrent * 0.001575 * 523 * 3.3 / 4095.0; // Convert to phase current
        BI = (float)Bcurrent * 0.001575 * 523 * 3.3 / 4095.0; // Convert to phase current
  
        // printf("Angle: %.2f | Angle Analog Val: %.2f | V_BUS Analog Val: %u | Acurr: %.2f | Bcurr: %.2f \r\n", angle, analog_angle_read(), VBUS, AI, BI);
        printf("Angle:%.2f,AngleAnalogVal:%.2f,V_BUSAnalogVal:%u,Acurr:%.2f,Bcurr:%.2f\r\n",angle,analog_angle_read(),VBUS,AI,BI);

        // i2c_scan();
        // sleep_ms(5000);

        // sleep_ms(10);
    }

   return 0;
}