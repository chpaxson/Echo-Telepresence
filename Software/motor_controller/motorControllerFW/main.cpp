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

// Sensor constants
#define I2C_PORT i2c1
const uint8_t I2C_SDA_PIN = 2;
const uint8_t I2C_SCL_PIN = 3;
MT6701_I2C sensor = MT6701_I2C(sensor_default); // Create an instance of the MT6701_I2C class

//Current sense constants
InlineCurrentSense current_sense = InlineCurrentSense(.523, ADC_CURRENT_A_PIN, ADC_CURRENT_B_PIN);
// Comamander instance
Commander command = Commander();

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

void core1_main() {

    printf("Entered core0 (core=%d)\n", get_core_num());
    
    // Send a CAN message
    struct can2040_msg tx_msg = {
        .id = 0x123,
        .dlc = 2,
        .data = {0xAB, 0xCD}
    };
    
    while (1) {
        //Can transmit  
        can2040_transmit(&cbus, &tx_msg);
        printf("Message sent\n");

    }

}

int main() {
    stdio_init_all();
    sleep_ms(10000);

    printf("Start... \n");

    // printf("Entered core0 (core=%d)\n", get_core_num());
    // multicore_launch_core1(core1_main);

    sensor.init(I2C_PORT, I2C_SCL_PIN, I2C_SDA_PIN); // Initialize the MT6701_I2C instance   
    // link the motor to the sensor
    motor.linkSensor(&sensor);

    driver.voltage_power_supply = 12;
    // driver config
    driver.init();
    current_sense.linkDriver(&driver);
    motor.linkDriver(&driver);

    current_sense.init();
    motor.linkCurrentSense(&current_sense);

    //current_sense.skip_align = true;
    // set motion control loop to be used
    motor.torque_controller = TorqueControlType::voltage;
    motor.controller = MotionControlType::angle;

    // controller configuration 
    // default parameters in defaults.h

    // controller configuration based on the control type 
    // velocity PID controller parameters
    //default P=0.5 I = 10 D =0
    motor.PID_velocity.P = 0.2;
    motor.PID_velocity.I = 20;
    motor.PID_velocity.D = 0.001;
    // // jerk control using voltage voltage ramp
    // // default value is 300 volts per sec  ~ 0.3V per millisecond
    motor.PID_velocity.output_ramp = 1000;

    // // velocity low pass filtering
    // // default 5ms - try different values to see what is the best. 
    // // the lower the less filtered
    motor.LPF_velocity.Tf = 0.01;

    // angle P controller -  default P=20
    motor.P_angle.P = 20;

    //  maximal velocity of the position control
    // default 20
    motor.velocity_limit = 4;
    // default voltage_power_supply
    motor.voltage_limit = 10;

    // since the phase resistance is provided we set the current limit not voltage
    // default 0.2
    //motor.phase_resistance = 1.5; // Ohm
    motor.current_limit = 1.5; // Amps
   // motor.velocity_limit = 1; // rad/s
    //motor.voltage_limit = 10; // Volts
    motor.voltage_sensor_align = 7; 

    //motor.target = 6.4; // target voltage


    // initialize motor
    motor.init();
    // align sensor and start FOC
    motor.initFOC();

    float target = 3;
    
    while (1) {
        // main FOC algorithm function
    //    sensor.update(); // update the sensor values

        // Motion control function
        uint64_t t0 = time_us_64();
        while (time_us_64() - t0 < 9 * 1e6) {
            motor.loopFOC();
            motor.move(target); // target angle
        }

        printf("Angle: %f | target: %f | error: %f \n", motor.shaftAngle(), target, motor.shaftAngle() - target);
        target += 1.5;
    }

   return 0;
}