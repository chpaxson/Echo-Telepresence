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

int can_downsample = 1; // downsample the can bus to 1s

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

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg) {
    if (notify & CAN2040_NOTIFY_RX) {
        // A message was received
        // printf("Received CAN msg ID=0x%03X LEN=%d: ", msg->id, msg->dlc);
        // for (int i = 0; i < msg->dlc; i++) {
        //     printf("%u ", msg->data[i]);
        // }
        // printf("\n");

        // // Example: Process received data
        if (msg->id == receive_msg_id && msg->dlc == 4) { // Check message ID and data length
            memcpy(&received_angle, msg->data, sizeof(float)); // Copy the bytes into a float
            // printf("Received angle: %f\n", received_angle); // Print the received angle
        }
    }



    if (notify & CAN2040_NOTIFY_TX) {
        // A message was successfully transmitted
        //printf("Message transmitted successfully.\n");
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
    uint32_t sys_clock = 125000000, bitrate = 750000;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIOx_IRQHandler);
    NVIC_SetPriority(PIO0_IRQ_0_IRQn, 1);
    NVIC_EnableIRQ(PIO0_IRQ_0_IRQn);

    // Start canbus
    can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
    printf("CAN bus initialized.\n");
}

void adc_setup(void) {
    adc_init();

    adc_gpio_init(ADC_CURRENT_A_PIN); // Current A input 0
    adc_gpio_init(ADC_CURRENT_B_PIN); // Current B input 1
    adc_gpio_init(ADC_MT6701_PIN); // MT6701 input 2
    adc_gpio_init(ADC_VBUS_PIN); // VBUS input 3
}

void core1_main() {
    canbus_setup();
    printf("Entered core0 (core=%d)\n", get_core_num());
    
    // Send a CAN message
    struct can2040_msg tx_msg = {
        .id = transmit_msg_id,
        .dlc = sizeof(float),
    };

    uint32_t raw_data; // Initialize raw_data
    float angle; // Initialize angle
    while (1) {
        raw_data = multicore_fifo_pop_blocking(); // Receive raw 32-bit data

        memcpy(&angle, &raw_data, sizeof(float)); // Copy the raw data back into a float
        memcpy(tx_msg.data, &angle, sizeof(float)); // Copy the angle to the CAN message data

        int result = can2040_transmit(&cbus, &tx_msg);
        if (result == 0) {
            // printf("Message queued for transmission.\n");
        } else {
            printf("Failed to queue message for transmission. Error: %d\n", result);
        }
    }

}

int main() {
    stdio_init_all();
    sleep_ms(10000);

    printf("Start... \n");

    printf("Entered core0 (core=%d)\n", get_core_num());
    multicore_launch_core1(core1_main);

    sensor.init(I2C_PORT, I2C_SCL_PIN, I2C_SDA_PIN); // Initialize the MT6701_I2C instance   
    // link the motor to the sensor
    motor.linkSensor(&sensor);

    driver.voltage_power_supply = 24; // set the power supply voltage for the driver
    // driver config
    driver.init();
    motor.linkDriver(&driver);

    // set motion control loop to be used
    // motor.torque_controller = TorqueControlType::voltage;
    motor.controller = MotionControlType::torque;

    // default voltage_power_supply
<<<<<<< Updated upstream
    motor.voltage_limit = 24; // Volts
    motor.phase_resistance = 1.85; // Ohm
    motor.current_limit = 2; // Amps
=======
    motor.voltage_limit = 9; // Volts
    motor.phase_resistance = 1.3; // Ohm
    motor.phase_inductance = 0.0029; // Henry
    mmmotor.kV_rating = 0.1;
    motor.current_limit = 4; // Amps
>>>>>>> Stashed changes
    motor.voltage_sensor_align = 7; 

    motor.zero_electric_angle = 5.52; // Set the zero electric angle to 0

    // initialize motor
    motor.init();
    // align sensor and start FOC
    motor.initFOC();

    //motor.motion_downsample = 100; // downsample the motion control loop to 15ms
    
    int can_downsample_cnt = 0;
    float angle;
    uint32_t raw_data;

    float offset = 3.0f; // Offset for the target angle
    float gain = 3.0f; // Gain for the error term
    float error, targetTorque;

    while (1) {
        // main FOC algorithm function
        motor.loopFOC();

        // Determine offset based on the direction of the difference
<<<<<<< Updated upstream
        // error = received_angle - motor.shaft_angle;

        // motor.move( ((error > 0 ) ? offset : (-1 * offset)) + (gain * error)); // target torque
        motor.move(8);
=======
        error = received_angle - motor.shaft_angle;
        targetTorque = ((error > 0 ) ? offset : (-1 * offset)) + (gain * error);
        if (targetTorque < 7.0f && targetTorque > -7.0f) {
            targetTorque = 0.0f; // Set to zero if within the deadband
        }
        motor.move(targetTorque); // target torque
>>>>>>> Stashed changes

        // if(can_downsample_cnt == can_downsample) {
        //     // Send the sensor angle to core 1
        //     // Convert the angle to a 32-bit integer representation

        //     angle = motor.shaft_angle;
        //     memcpy(&raw_data, &angle, sizeof(float)); // Convert float to uint32_t
        //     multicore_fifo_push_blocking(raw_data); // Send the data to core 1

        //     can_downsample_cnt = 0; // Reset downsample counter

<<<<<<< Updated upstream
        //     // printf("MyAngle:%f,RecievedAngle:%f,error:%f,applied_torque:%f\n", angle, received_angle, error, ((error > 0 ) ? offset : (-1 * offset)) + (gain * error));
        // }
        // can_downsample_cnt++;
        // sleep_ms(1); // Sleep for 1ms to avoid busy waiting
=======
            printf("MyAngle:%f,RecievedAngle:%f,error:%f,applied_torque:%f\n", angle, received_angle, error, ((error > 0 ) ? offset : (-1 * offset)) + (gain * error));
        }
        can_downsample_cnt++;
        sleep_ms(1); // Sleep for 1ms to avoid busy waiting
>>>>>>> Stashed changes
    }

   return 0;
}