/*******************************************************************************
* SIMPLE FOC Inspired RP2040 Based Motor Controller
*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"
#include "hardware/irq.h"

#include "stepper.h"
#include "MT6701.h"
#include "can/can2040.h"

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

// CAN constants
#define CAN_TX_PIN 0
#define CAN_RX_PIN 1
#define CAN_PIO    pio0
#define CAN_SM     0

struct can2040 cb;

/*******************************************************************************
* Main
*/

void can_rx_callback(struct can2040 *cd, struct can2040_msg *msg) {
    printf("Received CAN msg ID=0x%03X LEN=%d: ", msg->id, msg->dlc);
    for (int i = 0; i < msg->dlc; i++) {
        printf("%02X ", msg->data[i]);
    }
    printf("\n");
}

void can2040_irq_handler() {
    can2040_pio_irq_handler(&cb);
}


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

    // Initialize CAN
    can2040_setup(&cb, 5); // 500 kbps
    cb.callback = can_rx_callback;

    irq_set_exclusive_handler(PIO0_IRQ_0 + CAN_SM, can2040_irq_handler);
    irq_set_enabled(PIO0_IRQ_0 + CAN_SM, true);

    can2040_start(&cb);

    struct can2040_msg msg = {
        .id = 0x123,
        .dlc = 8,
        .data = {0xDE, 0xAD, 0xBE, 0xEF, 0xBA, 0xAD, 0xF0, 0x0D}
    };

    // Send a CAN message
    struct can2040_msg tx_msg = {
        .id = 0x123,
        .dlc = 2,
        .data = {0xAB, 0xCD}
    };

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


        //Can transmit
        can2040_transmit(&cb, &tx_msg);
        printf("Message sent\n");

        // i2c_scan();
        // sleep_ms(5000);

        // sleep_ms(10);
    }

   return 0;
}