/*******************************************************************************
* SIMPLE FOC Inspired RP2040 Based Motor Controller
*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/adc.h"

#include "MT6701_i2c.h"

/*******************************************************************************
* Pin Definitions
*/


/*******************************************************************************
* Main
*/

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);  // Initialize I2C with 100kHz clock
    adc_init();

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // Select ADC input 0 (GPIO26)
    adc_gpio_init(28);
    adc_select_input(2);

    sleep_ms(5000);
    printf("Start... \n");

    uint8_t data[2] = {0};
    uint16_t analog = 0;
    float angle = 0;

    //scans for i2c address
    i2c_scan();

    while (1) {
        analog = adc_read();  // 12-bit value (0–4095)


        angle = i2c_read(data);
        printf("Angle: %.2f | Angle Analog Val: %.2f\r\n", angle, (analog / 4095.0f) * 360.0f);

        sleep_ms(1000);  // Read every second
    }

   return 0;
}