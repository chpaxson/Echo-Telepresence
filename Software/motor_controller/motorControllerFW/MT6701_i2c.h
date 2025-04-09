/*******************************************************************************
* I2C Library for the MT6701 Rotary Incoder
*/

#include "hardware/i2c.h"

#define I2C_PORT i2c1
#define I2C_SDA_PIN 2
#define I2C_SCL_PIN 3

// Initialize I2C register
const uint8_t DEVICE_ADDR = 0x06; // MT6701 address 
const uint8_t REG_ADDR_A = 0x03;
const uint8_t REG_ADDR_B = 0x04;   
   

/*******************************************************************************
* Function Declarations
*/
int reg_write( i2c_inst_t *i2c, 
               const uint addr, 
               const uint8_t reg, 
               uint8_t *buf,
               const uint8_t nbytes);

int reg_read(  i2c_inst_t *i2c,
               const uint addr,
               const uint8_t reg,
               uint8_t *buf,
               const uint8_t nbytes);

uint16_t i2c_read_raw(uint8_t buffer[2]);

float i2c_read(uint8_t buffer[2]);

void i2c_scan();

/*******************************************************************************
* Function Definitions
*/

// Write 1 byte to the specified register
int reg_write( i2c_inst_t *i2c, 
               const uint addr, 
               const uint8_t reg, 
               uint8_t *buf,
               const uint8_t nbytes) {

   int num_bytes_read = 0;
   uint8_t msg[nbytes + 1];

   // Check to make sure caller is sending 1 or more bytes
   if (nbytes < 1) {
       return 0;
   }

   // Append register address to front of data packet
   msg[0] = reg;
   for (int i = 0; i < nbytes; i++) {
       msg[i + 1] = buf[i];
   }

   // Write data to register(s) over I2C
   i2c_write_blocking(i2c, addr, msg, (nbytes + 1), false);

   return num_bytes_read;
}

// Read byte(s) from specified register. If nbytes > 1, read from consecutive
// registers.
int reg_read(  i2c_inst_t *i2c,
               const uint addr,
               const uint8_t reg,
               uint8_t *buf,
               const uint8_t nbytes) {

   int num_bytes_read = 0;

   // Check to make sure caller is asking for 1 or more bytes
   if (nbytes < 1) {
       return 0;
   }

   // Read data from register(s) over I2C
   i2c_write_blocking(i2c, addr, &reg, 1, true);
   num_bytes_read = i2c_read_blocking(i2c, addr, buf, nbytes, false);

   return num_bytes_read;
}

uint16_t i2c_read_raw(uint8_t buffer[2]) {

    reg_read(I2C_PORT, DEVICE_ADDR, 0x03, buffer, 1);

    // Data is split between 8 bits in Reg 0x03 and 6 bits in Reg 0x04
    uint16_t angle_raw = ((buffer[0] << 8) | (buffer[1])) >> 2;

    return angle_raw;
}

float i2c_read(uint8_t buffer[2]) {
    uint16_t angle_raw = i2c_read_raw(buffer);

    //scale value to degrees
    return angle_raw * 360.0f / 16384.0f;
}

// Sweep through all 7-bit I2C addresses, to see if any slaves are present on
// the I2C bus. Print out a table that looks like this:
//
// I2C Bus Scan
//    0 1 2 3 4 5 6 7 8 9 A B C D E F
// 00 . . . . . . . . . . . . . . . .
// 10 . . @ . . . . . . . . . . . . .
// 20 . . . . . . . . . . . . . . . .
// 30 . . . . @ . . . . . . . . . . .
// 40 . . . . . . . . . . . . . . . .
// 50 . . . . . . . . . . . . . . . .
// 60 . . . . . . . . . . . . . . . .
// 70 . . . . . . . . . . . . . . . .
// E.g. if addresses 0x12 and 0x34 were acknowledged.

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void i2c_scan() {

    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;

        //MT6701 DOES NOT follow i2c reserved address specs, reserved address condition ignored
        // if (reserved_addr(addr))
        //     ret = PICO_ERROR_GENERIC;
        // else
            ret = i2c_read_blocking(I2C_PORT, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}