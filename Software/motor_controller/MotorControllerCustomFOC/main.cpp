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
#include "can/can2040.h"
}

#include "sensors/MT6701_I2C.h"
#include "drivers/StepperDriver4PWM.h"
#include "StepperMotor.h"
#include "communication/Commander.h"
#include "current_sense/InlineCurrentSense.h"
/*******************************************************************************
* Pin Definitions
*/

// Analog Pins
const uint8_t ADC_CURRENT_A_PIN = 26, ADC_CURRENT_A_IN = 0; // Integrated Current Sensor A 
const uint8_t ADC_CURRENT_B_PIN = 27, ADC_CURRENT_B_IN = 1; // Integrated Current Sensor B 
const uint8_t ADC_MT6701_PIN = 28, ADC_MT6701_IN = 2; // MT6701 Analog input
const uint8_t ADC_VBUS_PIN = 29, ADC_VBUS_IN = 3; // Voltage on Power Rail

// Stepper constants.
const uint8_t stepper_pin_1A = 6;
const uint8_t stepper_pin_1B = 7;
const uint8_t stepper_pin_2A = 8;
const uint8_t stepper_pin_2B = 9;

// StepperMotor(pole pair number, phase resistance (optional) );
StepperMotor motor = StepperMotor(50);

// StepperDriver4PWM(ph1A, ph1B, ph2A, ph2B, (en1, en2 optional))
StepperDriver4PWM driver = StepperDriver4PWM(6, 7, 8, 9, NOT_SET, NOT_SET);

// CAN constants
uint8_t pio_num = 0;
uint8_t gpio_rx = 1, gpio_tx = 0;
struct can2040 cbus;

// LEFT MOTOR
uint32_t transmit_msg_id = 0x100; // ID for the message to be transmitted
uint32_t receive_msg_id = 0x200; // ID for the message to be received

// RIGHT MOTOR
// uint32_t transmit_msg_id = 0x200; // ID for the message to be transmitted
// uint32_t receive_msg_id = 0x100; // ID for the message to be received

int can_downsample = 10; // downsample the can bus to 1s

// Sensor constants
#define I2C_PORT i2c1
const uint8_t I2C_SDA_PIN = 2;
const uint8_t I2C_SCL_PIN = 3;
MT6701_I2C sensor = MT6701_I2C(sensor_default); // Create an instance of the MT6701_I2C class

//Current sense constants
InlineCurrentSense current_sense = InlineCurrentSense(.523, ADC_CURRENT_A_PIN, ADC_CURRENT_B_PIN);
// Comamander instance
Commander command = Commander();

//
float received_angle;

/*******************************************************************************
* Main
*/



int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
