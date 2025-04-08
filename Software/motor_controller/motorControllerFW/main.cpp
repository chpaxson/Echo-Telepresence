#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"

#include "MT6701_i2c.h"

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);  // Initialize I2C with 100kHz clock
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

    uint8_t data[2] = {0};

    while (1) {
        i2c_read_data(data);
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