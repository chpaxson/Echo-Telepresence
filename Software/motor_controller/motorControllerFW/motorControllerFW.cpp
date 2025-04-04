#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 2
#define I2C_SCL 3

// Initialize I2C register
const uint8_t addr = 0b1000110; // MT6701 address

uint8_t buffer[4] = {0};
uint16_t angle = 0;

int main() {
    stdio_init_all();

    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    while (true) {

        printf("Reading angle...\n");

        i2c_read_blocking (i2c0, addr, buffer, 4, false);
        // buffer[2] = angle[13:6]
        // buffer[3] = angle[5:0]

        angle = (buffer[2] << 8) | buffer[3];  // Combine the two bytes into a single 16-bit value

        printf("Angle: 0x%04X\n", angle);  // Print the angle value to the console

        sleep_ms(250);
    }
    return 0;
}