#ifndef MT6701_I2C_LIB_H
#define MT6701_I2C_LIB_H

#include "common/base_classes/Sensor.h"
#include "common/foc_utils.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"


struct MT6701_I2CConfig_s  {
    int chip_address;
    int bit_resolution;
    int angle_register;
    int data_start_bit; 
};

// some predefined structures
extern MT6701_I2CConfig_s sensor_default;

class MT6701_I2C: public Sensor{
 public:
    /**
     * MT6701_I2C class constructor
     * @param chip_address  I2C chip address
     * @param bits number of bits of the sensor resolution 
     * @param angle_register_msb  angle read register msb
     * @param _bits_used_msb number of used bits in msb
     */
    MT6701_I2C(uint8_t _chip_address, int _bit_resolution, uint8_t _angle_register_msb, int _msb_bits_used);

    /**
     * MagneticSensorI2C class constructor
     * @param config  I2C config
     */
    MT6701_I2C(MT6701_I2CConfig_s config);

        
    /** sensor initialise pins */
    void init(i2c_inst_t* i2c_port, uint8_t scl_pin, uint8_t sda_pin);

    // implementation of abstract functions of the Sensor class
    /** get current angle (rad) */
    float getSensorAngle() override;

    /** experimental function to check and fix SDA locked LOW issues */
    void i2c_scan();

    /** current error code from Wire endTransmission() call **/
    uint8_t currWireError = 0;

  private:
    float cpr; //!< Maximum range of the magnetic sensor
    uint16_t lsb_used; //!< Number of bits used in LSB register
    uint8_t lsb_mask;
    uint8_t msb_mask;
    
    // I2C variables
    uint8_t angle_register_msb; //!< I2C angle register to read
    uint8_t chip_address; //!< I2C chip select pins

    // I2C functions
    /** Read one I2C register value */
    int read(uint8_t angle_register_msb);

    /**
     * Function getting current angle register value
     * it uses angle_register variable
     */
    int getRawCount();
    
    /* the i2c port for this sensor */
    i2c_inst_t* I2C_PORT;
    uint8_t I2C_SDA_PIN;
    uint8_t I2C_SCL_PIN;


};


#endif