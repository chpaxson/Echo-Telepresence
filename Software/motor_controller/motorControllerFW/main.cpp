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

// Enum for motor IDs
enum MotorID {
    MOTOR_0 = 0,
    MOTOR_1 = 1,
    MOTOR_2 = 2,
    MOTOR_3 = 3
};

MotorID thisMotor = MOTOR_3; 

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

int can_downsample = 10; // downsample the can bus to 1s

// Sensor constants
#define I2C_PORT i2c1
const uint8_t I2C_SDA_PIN = 2;
const uint8_t I2C_SCL_PIN = 3;
MT6701_I2C sensor = MT6701_I2C(sensor_default); // Create an instance of the MT6701_I2C class

//
float linked_angle;

bool received_can = 0;
bool recieved_target = 0;

// Global variables for storing received CAN data
float R, L, kV, vel_Lim, V_lim, I_lim;
int sensor_direction;
float _zero_electric_angle;
float vel_kp, vel_ki, vel_kd;
float pos_kp, pos_ki, pos_kd;
uint8_t controller;
float target;
float get_position, get_velocity;

/*******************************************************************************
* Main
*/

// Helper function to process CAN data
void process_can_data(uint32_t id, void *dest, size_t expected_size, const struct can2040_msg *msg) {
    // Extract the motor-specific ID range
    uint32_t motor_base_id = thisMotor << 8; // Shift motor ID to the upper byte (e.g., 0x100, 0x200)
    uint32_t relative_id = id - motor_base_id;

    if (msg->dlc == expected_size) {
        memcpy(dest, msg->data, expected_size);
    } else {
        printf("Invalid data length for CAN ID: 0x%03X (Motor %d). Expected: %zu, Received: %d\n",
               id, motor, expected_size, msg->dlc);
    }
}

// Callback function for CAN messages
static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg) {
    if (notify & CAN2040_NOTIFY_RX) {
        // A message was received
        uint32_t motor_base_id = thisMotor << 8; // Shift motor ID to the upper byte (e.g., 0x100, 0x200)
        uint32_t relative_id = msg->id - motor_base_id;

        switch (relative_id) {
            case 0x00: // R
                process_can_data(msg->id, &R, sizeof(float), msg);
                break;
            case 0x01: // L
                process_can_data(msg->id, &L, sizeof(float), msg);
                break;
            case 0x02: // kV
                process_can_data(msg->id, &kV, sizeof(float), msg);
                break;
            case 0x03: // vel_Lim
                process_can_data(msg->id, &vel_Lim, sizeof(float), msg);
                break;
            case 0x04: // V_lim
                process_can_data(msg->id, &V_lim, sizeof(float), msg);
                break;
            case 0x05: // I_lim
                process_can_data(msg->id, &I_lim, sizeof(float), msg);
                break;
            case 0x06: // sensor_direction + zero_electric_angle
                if (msg->dlc == sizeof(float) + 1) { // Boolean + float
                    sensor_direction = msg->data[0]; // First byte is the boolean
                    memcpy(&_zero_electric_angle, &msg->data[1], sizeof(float)); // Remaining bytes are the float
                } else {
                    printf("Invalid data length for CAN ID: 0x%03X (Motor %d). Expected: %zu, Received: %d\n",
                           msg->id, thisMotor, sizeof(float) + 1, msg->dlc);
                }
                break;
            case 0x07: // vel_kp
                process_can_data(msg->id, &vel_kp, sizeof(float), msg);
                break;
            case 0x08: // vel_ki
                process_can_data(msg->id, &vel_ki, sizeof(float), msg);
                break;
            case 0x09: // vel_kd
                process_can_data(msg->id, &vel_kd, sizeof(float), msg);
                break;
            case 0x0A: // pos_kp
                process_can_data(msg->id, &pos_kp, sizeof(float), msg);
                break;
            case 0x0B: // pos_ki
                process_can_data(msg->id, &pos_ki, sizeof(float), msg);
                break;
            case 0x0C: // pos_kd
                process_can_data(msg->id, &pos_kd, sizeof(float), msg);
                break;
            case 0x0D: // controller
                process_can_data(msg->id, &controller, sizeof(uint8_t), msg);
                printf("Received controller: %d\n", controller);
                received_can = true;
                break;
            case 0x14: // target
                process_can_data(msg->id, &target, sizeof(float), msg);
                recieved_target = true;
                break;
            default:
                // printf("Unknown CAN ID: 0x%03X (Motor %d)\n", msg->id, thisMotor);
                break;
        }

        if(msg->id == ((((thisMotor + 2) % 4) << 8) + 0x018)) { 
            // Process the received angle
            memcpy(&linked_angle, msg->data, sizeof(float));
            // printf("Received angle: %f\n", linked_angle);
        }
    }

    if (notify & CAN2040_NOTIFY_TX) {
        // A message was successfully transmitted
        // printf("Message transmitted successfully.\n");
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
    uint32_t sys_clock = 125000000, bitrate = 500000; // 500 kbps

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
        .id = (thisMotor << 8) + 0x018, // Set the ID for the message
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

    printf("Entered core0 (core=%d)\n", get_core_num());
    multicore_launch_core1(core1_main);

    // Wait for the CAN RX notify flag
    while (!( received_can)) {
        printf("Waiting for CAN RX notify...\n");
        tight_loop_contents(); // Wait in a tight loop
    }
    printf("Start... \n");

    sensor.init(I2C_PORT, I2C_SCL_PIN, I2C_SDA_PIN); // Initialize the MT6701_I2C instance   
    // link the motor to the sensor
    motor.linkSensor(&sensor);

    driver.voltage_power_supply = 24; // set the power supply voltage for the driver
    driver.voltage_limit = 24; // set the voltage limit for the driver
    // driver config
    driver.init();
    motor.linkDriver(&driver);

    motor.PID_velocity.P = 0.2;
    motor.PID_velocity.I = 20;
    motor.PID_velocity.D = 0.001;

    motor.PID_velocity.output_ramp = 1000; // Set the output ramp for the velocity PID controller

    motor.LPF_velocity.Tf = 0.005;

    motor.P_angle.P = 20; 
    motor.P_angle.I = 0;  // usually only P controller is enough 
    motor.P_angle.D = 0;  // usually only P controller is enough 

    motor.P_angle.output_ramp = 10000; // default 1e6 rad/s^2
    motor.LPF_angle.Tf = 0; // default 0

    motor.velocity_limit = vel_Lim; // Set the position limit

    // set motion control loop to be used
    // default voltage_power_supply
    motor.voltage_limit = V_lim; // Volts
    motor.phase_resistance = R; // Ohm
    motor.phase_inductance = L; // Henry
    motor.KV_rating = kV; // V/rad/s
    motor.current_limit = I_lim; // Amps

    motor.voltage_sensor_align = 14;


    // Determine the controller type
    switch (controller) {
        case 0: //Disabled
            motor.controller = MotionControlType::torque;
            motor.torque_controller = TorqueControlType::voltage;
            motor.voltage_limit = 0; // Volts
            motor.current_limit = 0; // Amps
            break;
        case 1: // Torque control
            motor.torque_controller = TorqueControlType::voltage;
            motor.controller = MotionControlType::torque;
            break;
        case 2: // Position control
            motor.controller = MotionControlType::velocity_openloop;
            break;
        case 3: // Velocity control Open Loop
            motor.controller = MotionControlType::angle_openloop;
            break;
        default:
            printf("Unknown controller mode: %d\n", controller);
            break;
    }


    // Print all values to serial
    printf("Motor Configuration: %u | Motor Controller: %u\n", thisMotor, controller);
    printf("Voltage Limit: %f V\n", V_lim);
    printf("Phase Resistance: %f Ohm\n", R);
    printf("Phase Inductance: %f Henry\n", L);
    printf("KV Rating: %f V/rad/s\n", kV);
    printf("Current Limit: %f Amps\n", I_lim);
    printf("Voltage Sensor Align: %f\n", motor.voltage_sensor_align);

    // initialize motor
    motor.init();
    // align sensor and start FOC
    motor.initFOC();

    // motor.motion_downsample = 1; // downsample the motion control loop to 15ms
    
    int can_downsample_cnt = 0;
    float angle;
    uint32_t raw_data;

    float offset = 0.0f; // Offset for the target angle
    float deadband = offset; // Deadband for the target torque
    float gain = 5.0f; // Gain for the error term
    float velocity_noise = 0.1f; // Noise in the velocity measurement
    float error, targetTorque;

    sleep_ms(3000);

    while (1) {
        // main FOC algorithm function
        // sensor.update(); // update the sensor
        motor.loopFOC();

        motor.voltage_limit = V_lim; // Volts
        motor.current_limit = I_lim; // Amps

        // Print the updated voltage and current limits to serial
        // printf("Updated Voltage Limit: %f V\n", motor.voltage_limit);
        // printf("Updated Current Limit: %f Amps\n", motor.current_limit);
        motor.PID_velocity.P = vel_kp; // Set the velocity PID parameters
        motor.PID_velocity.I = vel_ki;
        motor.PID_velocity.D = vel_kd;

        motor.P_angle.P = pos_kp; // Set the position PID parameters
        motor.P_angle.I = pos_ki;
        motor.P_angle.D = pos_kd;
        motor.velocity_limit = vel_Lim; // Set the position limit

        gain = vel_kp;
        offset = vel_ki;
        velocity_noise = vel_kd;


        // Print all values to serial
        // printf("Velocity PID: P=%f, I=%f, D=%f\n", vel_kp, vel_ki, vel_kd);
        // printf("Position PID: P=%f, I=%f, D=%f\n", pos_kp, pos_ki, pos_kd);
        // printf("Velocity Limit: %f\n", vel_Lim);


        // // Determine offset based on the direction of the difference
        error = linked_angle - motor.shaft_angle;
        targetTorque = ((error > 0 ) ? offset : (-1 * offset)) + (gain * error);

        // // // Apply deadband to the target torque
        if (abs(targetTorque) < deadband || abs(motor.shaft_velocity) < velocity_noise) {
            targetTorque = 0.0f; // Set to zero if within the deadband
        }

        printf("target: %f| otherAngle: %f| Myangle: %f \n", target, linked_angle, sensor.getAngle());

        if(controller == 1) {
            target = targetTorque; // Set the target torque
        }

        motor.move(target); // target torque
        
        if(can_downsample_cnt == can_downsample) {
            // Send the sensor angle to core 1
            // Convert the angle to a 32-bit integer representation
            angle = sensor.getAngle(); // Get the angle from the motor
            memcpy(&raw_data, &angle, sizeof(float)); // Convert float to uint32_t
            multicore_fifo_push_blocking(raw_data); // Send the data to core 1
        
            can_downsample_cnt = 0; // Reset downsample counter
        }
        can_downsample_cnt++;
    }

   return 0;
}