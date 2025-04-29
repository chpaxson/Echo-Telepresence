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
#include "RP2040.h"

extern "C" {
#include "stepper.h"
#include "MT6701.h"
#include "can/can2040.h"
}
/*******************************************************************************
* Pin Definitions
*/

// Analog Pins
const uint8_t ADC_CURRENT_A_PIN = 26, ADC_CURRENT_A_IN = 0; // Integrated Current Sensor A 
const uint8_t ADC_CURRENT_B_PIN = 27, ADC_CURRENT_B_IN = 1; // Integrated Current Sensor B 
const uint8_t ADC_MT6701_PIN = 28, ADC_MT6701_IN = 2; // MT6701 Analog input
const uint8_t ADC_VBUS_PIN = 29, ADC_VBUS_IN = 3; // Voltage on Power Rail

// Stepper constants.
stepper_t stepper;
const uint8_t stepper_pin_1A = 6;
const uint8_t stepper_pin_1B = 7;
const uint8_t stepper_pin_2A = 8;
const uint8_t stepper_pin_2B = 9;
const uint16_t stepper_steps_per_revolution = 200;
const stepper_mode_t stepping_mode = power;

// CAN constants
uint8_t pio_num = 0;
uint8_t gpio_rx = 1, gpio_tx = 0;
struct can2040 cbus;

/*******************************************************************************
* Main
*/

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg) {
    if (notify & CAN2040_NOTIFY_RX) {
        // A message was received
        printf("Received CAN msg ID=0x%03X LEN=%d: ", msg->id, msg->dlc);
        for (int i = 0; i < msg->dlc; i++) {
            printf("%02X ", msg->data[i]);
        }
        printf("\n");
    }

    if (notify & CAN2040_NOTIFY_TX) {
        // A message was successfully transmitted
        printf("Message transmitted successfully.\n");
    }

    if (notify & CAN2040_NOTIFY_ERROR) {
        // An error occurred
        printf("CAN error occurred!\n");
    }
}

static void
PIOx_IRQHandler(void)
{
    can2040_pio_irq_handler(&cbus);
}

void canbus_setup(void)
{
    uint32_t sys_clock = 125000000, bitrate = 125000;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIOx_IRQHandler);
    NVIC_SetPriority(PIO0_IRQ_0_IRQn, 1);
    NVIC_EnableIRQ(PIO0_IRQ_0_IRQn);

    // Start canbus
    can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
}

void adc_setup(void) {
    adc_init();

    adc_gpio_init(ADC_CURRENT_A_PIN); // Current A input 0
    adc_gpio_init(ADC_CURRENT_B_PIN); // Current B input 1
    adc_gpio_init(ADC_MT6701_PIN); // MT6701 input 2
    adc_gpio_init(ADC_VBUS_PIN); // VBUS input 3
}

void i2c_setup(void) {
    i2c_init(I2C_PORT, 400 * 1000);  // Initialize I2C with 100kHz clock

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
}

void core1_main() {

    printf("Entered core0 (core=%d)\n", get_core_num());

    // Initialise the stepper
    stepper_init(&stepper, stepper_pin_1A, stepper_pin_1B,
        stepper_pin_2A, stepper_pin_2B,
        stepper_steps_per_revolution, stepping_mode);

    while (1) {
        // stepper_set_speed_rpm(&stepper, 50);
        // stepper_rotate_degrees(&stepper, 270);
        // sleep_ms(500);

        stepper_set_speed_rpm(&stepper, 150);
        stepper_rotate_steps(&stepper, 20);

        // stepper_release(&stepper);
        printf("Finished a stepper loop");
        sleep_ms(1000);
    }

}

int main() {
    stdio_init_all();
    sleep_ms(3000);

    printf("Entered core0 (core=%d)\n", get_core_num());
    multicore_launch_core1(core1_main);

    i2c_setup();    // Setup I2C
    adc_setup();    // Setup ADC
    canbus_setup(); // Setup CAN bus

    sleep_ms(5000);

    printf("Start... \n");

    // Send a CAN message
    struct can2040_msg tx_msg = {
        .id = 0x123,
        .dlc = 2,
        .data = {0xAB, 0xCD}
    };

    uint8_t data[2] = {0};
    uint16_t V = 0;
    float angle = 0;
    uint16_t Acurrent = 0;
    uint16_t Bcurrent = 0;
    float AI = 0;
    float BI = 0;
    float VBUS = 0;

    //scans for i2c address
    i2c_scan();

    while (1) {
        
        adc_select_input(ADC_VBUS_IN);
        V = adc_read();  // 12-bit value (0–4095)
        angle = angle_read(data);
        adc_select_input(ADC_CURRENT_A_IN);
        Acurrent = adc_read();
        adc_select_input(ADC_CURRENT_B_IN);
        Bcurrent = adc_read();

        // Convert to bus voltage (V)
        VBUS = (float)V / 4095.0 * 12.0; // Convert to voltage

        // Convert to phase current (A and B)
        AI = (float)Acurrent * 0.001575 * 523 * 3.3 / 4095.0;
        BI = (float)Bcurrent * 0.001575 * 523 * 3.3 / 4095.0;
  
        printf("Angle Bin Val: %u | Angle Val: %.2f | V_BUS Analog Val: %.2f | Acurr: %.2f | Bcurr: %.2f \r\n", angle_read_raw(data), angle, VBUS, AI, BI);
        // printf("Angle:%.2f,AngleAnalogVal:%.2f,V_BUSAnalogVal:%.2f,Acurr:%.2f,Bcurr:%.2f\r\n",angle,analog_angle_read(),VBUS,AI,BI);


        //Can transmit
        // can2040_transmit(&cbus, &tx_msg);
        // printf("Message sent\n");

        sleep_ms(100); // Sleep for 100ms
    }

   return 0;
}