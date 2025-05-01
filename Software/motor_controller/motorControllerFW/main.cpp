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

// I2C constants
#define I2C_PORT i2c1
const uint8_t I2C_SDA_PIN = 2;
const uint8_t I2C_SCL_PIN = 3;
MT6701_I2C sensor = MT6701_I2C(sensor_default); // Create an instance of the MT6701_I2C class

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
    // adc_setup();    // Setup ADC
    // canbus_setup(); // Setup CAN bus

    // ================================================
    // power supply voltage [V]
    driver.voltage_power_supply = 12;
    // Max DC voltage allowed - default voltage_power_supply
    driver.voltage_limit = 12;

    // driver init
    if (!driver.init()) {
        printf("Driver init failed!");
    }

    // enable driver
    driver.enable();
    printf("Driver ready!");

    // link the motor and the driver
    motor.linkDriver(&driver);

    // open loop control config
    motor.controller = MotionControlType::velocity_openloop;

    // init motor hardware
    if(!motor.init()){
        printf("Motor init failed!");
    }

    printf("Motor ready!");

    sleep_ms(5000);

    uint16_t V = 0;
    uint16_t Acurrent = 0;
    uint16_t Bcurrent = 0;
    float AI = 0;
    float BI = 0;
    float VBUS = 0;

    while (1) {
        sensor.update();

        // adc_select_input(ADC_VBUS_IN);
        // V = adc_read();  // 12-bit value (0–4095)
        // adc_select_input(ADC_CURRENT_A_IN);
        // Acurrent = adc_read();
        // adc_select_input(ADC_CURRENT_B_IN);
        // Bcurrent = adc_read();

        // // Convert to bus voltage (V)
        // VBUS = (float)V / 4095.0 * 12.0; // Convert to voltage

        // // Convert to phase current (A and B)
        // AI = (float)Acurrent * 0.001575 * 523 * 3.3 / 4095.0;
        // BI = (float)Bcurrent * 0.001575 * 523 * 3.3 / 4095.0;
  
        // // printf("Velocity Val: %.4f | Angle Val: %.4f | V_BUS Analog Val: %.2f | Acurr: %.2f | Bcurr: %.2f \r\n", sensor.getVelocity(), sensor.getSensorAngle(), VBUS, AI, BI);
        printf("Velocity: %.4f rad/s | Angle: %.4f rad | SensorAngle: %.4f rad | MechanicalAngle: %.4f rad | PreciseAngle: %.4f rad | Full Rotations: %i rev \r\n", motor.shaftVelocity(), sensor.getAngle(), sensor.getSensorAngle(), sensor.getMechanicalAngle(), sensor.getPreciseAngle(), sensor.getFullRotations());
        // // printf("Velocity:%.2f,Angle:%.2f,V_BUSAnalog:%.2f,Acurr:%.2f,Bcurr:%.2f\r\n",sensor.getVelocity(),sensor.getSensorAngle(),VBUS,AI,BI);

        motor.move(-6.28);
        sleep_ms(1);
    }

   return 0;
}