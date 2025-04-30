#include "MT6701_I2C.h"


/** Typical configuration for the 12bit MT6701 magnetic sensor over I2C interface */
MT6701_I2CConfig_s sensor_default = {
    .chip_address = 0x06, 
    .bit_resolution = 14,
    .angle_register = 0x03,
    .data_start_bit = 15
};


// MagneticSensorI2C(uint8_t _chip_address, float _cpr, uint8_t _angle_register_msb)
//  @param _chip_address  I2C chip address
//  @param _bit_resolution  bit resolution of the sensor
//  @param _angle_register_msb  angle read register
//  @param _bits_used_msb number of used bits in msb
MT6701_I2C::MT6701_I2C(uint8_t _chip_address, int _bit_resolution, uint8_t _angle_register_msb, int _bits_used_msb){
    // chip I2C address
    chip_address = _chip_address;
    // angle read register of the magnetic sensor
    angle_register_msb = _angle_register_msb;
    // register maximum value (counts per revolution)
    cpr = _powtwo(_bit_resolution);

    // depending on the sensor architecture there are different combinations of
    // LSB and MSB register used bits
    // AS5600 uses 0..7 LSB and 8..11 MSB
    // AS5048 uses 0..5 LSB and 6..13 MSB
    // MT6701 uses 0..5 LSB and 9..15 MSB
    // used bits in LSB
    lsb_used = _bit_resolution - _bits_used_msb;
    // extraction masks
    lsb_mask = (uint8_t)( (2 << lsb_used) - 1 );
    msb_mask = (uint8_t)( (2 << _bits_used_msb) - 1 );
}

MT6701_I2C::MT6701_I2C(MT6701_I2CConfig_s config){
    chip_address = config.chip_address; 

    // angle read register of the magnetic sensor
    angle_register_msb = config.angle_register;
    // register maximum value (counts per revolution)
    cpr = _powtwo(config.bit_resolution);

    int bits_used_msb = config.data_start_bit - 7;
    lsb_used = config.bit_resolution - bits_used_msb;
    // extraction masks
    lsb_mask = (uint8_t)( (2 << lsb_used) - 1 );
    msb_mask = (uint8_t)( (2 << bits_used_msb) - 1 );
}


void MT6701_I2C::init(i2c_inst_t* i2c_port, uint8_t sda_pin, uint8_t scl_pin) {
    I2C_PORT = i2c_port;
    I2C_SCL_PIN = scl_pin;
    I2C_SDA_PIN = sda_pin;

    i2c_init(I2C_PORT, 400 * 1000);  // Initialize I2C with 100kHz clock

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    this->Sensor::init(); // call base class init
}

//  Shaft angle calculation
//  angle is in radians [rad]
float MT6701_I2C::getSensorAngle(){
    // (number of full rotations)*2PI + current sensor angle 
    return  ( getRawCount() / (float)cpr) * _2PI ;
}



// function reading the raw counter of the magnetic sensor
int MT6701_I2C::getRawCount(){
	return (int)MT6701_I2C::read(angle_register_msb);
}

// I2C functions
/*
* Read a register from the sensor
* Takes the address of the register as a uint8_t
* Returns the value of the register
*/
int MT6701_I2C::read(uint8_t angle_reg_msb) {
    // read the angle register first MSB then LSB
    uint8_t readArray[2];
    uint16_t readValue = 0;

    // MT6701 uses 0..5 LSB and 6..13 MSB
    // Read data from register(s) over I2C
    i2c_write_blocking(I2C_PORT, chip_address, &angle_register_msb, 1, true);
    i2c_read_blocking(I2C_PORT, chip_address, readArray, 2, false);

    // Data is split between 8 bits in Reg 0x03 and 6 bits in Reg 0x04
    readValue = ((readArray[0] << 8) | (readArray[1])) >> 2;
    
    return readValue;
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

void MT6701_I2C::i2c_scan() {

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