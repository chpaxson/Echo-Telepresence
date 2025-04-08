#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"

#include "MT6701_i2c.h"

#define I2C_PORT i2c1
#define I2C_SDA_PIN 2
#define I2C_SCL_PIN 3

// Initialize I2C register
const uint8_t DEVICE_ADDR = 0b0000110; // MT6701 address 
const uint8_t REG_ADDR_A = 0x03;
const uint8_t REG_ADDR_B = 0x04;   

uint8_t high = 0, low = 0;

void i2c_read_data() {

    reg_read(I2C_PORT, DEVICE_ADDR, 0x03, &high, 1);
    reg_read(I2C_PORT, DEVICE_ADDR, 0x04, &low, 1);
    
    uint16_t angle_raw = ((high << 6) | (low >> 2)) & 0x3FFF;
    printf("Angle: %u (raw), %.2f degrees\n", angle_raw, angle_raw * 360.0f / 16384.0f);
}

void i2c_scan() {
        printf("Scanning I2C bus...\n");
        for (uint8_t addr = 1; addr < 127; addr++) {
            int result = i2c_write_blocking(I2C_PORT, addr, NULL, 0, false);
            if (result >= 0) {
                printf("Found device at 0x%02X\n", addr);
            }
        }
    }

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 100 * 1000);  // Initialize I2C with 100kHz clock
    adc_init();

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Make sure GPIO is high-impedance, no pullups etc
    //adc_gpio_init(26);
    // Select ADC input 0 (GPIO26)
    //adc_select_input(0);

    sleep_ms(5000);
    printf("Reading angle...\n");

    while (1) {
        // const float conversion_factor = 3.3f / (1 << 12);
        // uint16_t result = adc_read();
        // printf("Raw value: 0x%03x, cuurent: %f A\n", result, result * conversion_factor);
       // i2c_read_data();
        i2c_scan();
        sleep_ms(100);  // Read every second
    }

    return 0;
}

// bool reserved_addr(uint8_t addr) {
//     return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
// }

// int main() {
//     // Enable UART so we can print status output
//     stdio_init_all();
// #if !defined(I2C_PORT) || !defined(I2C_SDA_PIN) || !defined(I2C_SCL_PIN)
// #warning i2c/bus_scan example requires a board with I2C pins
//     puts("Default I2C pins were not defined");
// #else
//     // This example will use I2C0 on the default SDA and SCL pins (GP4, GP5 on a Pico)
//     i2c_init(I2C_PORT, 100 * 1000);
//     gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
//     gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
//     gpio_pull_up(I2C_SDA_PIN);
//     gpio_pull_up(I2C_SDA_PIN);
//     // Make the I2C pins available to picotool
//     bi_decl(bi_2pins_with_func(I2C_SDA_PIN, I2C_SDA_PIN, GPIO_FUNC_I2C));

//     printf("\nI2C Bus Scan\n");
//     printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

//     for (int addr = 0; addr < (1 << 7); ++addr) {
//         if (addr % 16 == 0) {
//             printf("%02x ", addr);
//         }

//         // Perform a 1-byte dummy read from the probe address. If a slave
//         // acknowledges this address, the function returns the number of bytes
//         // transferred. If the address byte is ignored, the function returns
//         // -1.

//         // Skip over any reserved addresses.
//         int ret;
//         uint8_t rxdata;
//         if (reserved_addr(addr))
//             ret = PICO_ERROR_GENERIC;
//         else
//             ret = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false);

//         printf(ret < 0 ? "." : "@");
//         printf(addr % 16 == 15 ? "\n" : "  ");
//     }
//     printf("Done.\n");
//     return 0;
// #endif
// }