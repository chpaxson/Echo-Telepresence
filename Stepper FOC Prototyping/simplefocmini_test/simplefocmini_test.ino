#include <SimpleFOC.h>


BLDCMotor motor = BLDCMotor(14, 10.4, 260);
BLDCDriver3PWM driver = BLDCDriver3PWM(14, 32, 15, 33);
MagneticSensorI2C sensor = MagneticSensorI2C(AS5600_I2C);

Commander command = Commander(Serial);
void onTarget(char* cmd){ command.motion(&motor, cmd); }


void setup() {
  // configure i2C and sensor
  Wire.setPins(4, 5);
  Wire.setClock(400000);
  sensor.init();

  Serial.println("Sensor ready");
  _delay(1000);

  // Configure driver
  driver.voltage_power_supply = 24;
  driver.voltage_limit = 6;
  driver.init();

  // Link driver and sensor
  motor.linkSensor(&sensor);
  motor.linkDriver(&driver);

  motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
  motor.controller = MotionControlType::angle;
  motor.velocity_limit = 4;

  motor.PID_velocity.P = 0.2;
  motor.PID_velocity.I = 20;
  motor.LPF_velocity.Tf = 0.02;

  motor.init();

  motor.initFOC();

  // add target command T
  command.add('T', onTarget, "motion control");

  // monitoring port
  Serial.begin(115200);
  Serial.println("Motor ready.");
  Serial.println("Set the target angle using serial terminal:");
  _delay(1000);

}

void loop() {
  
  motor.loopFOC();
  
  motor.move();

  command.run();
}