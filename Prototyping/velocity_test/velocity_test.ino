#include <SimpleFOC.h>

StepperMotor motor = StepperMotor( 50 );
StepperDriver4PWM driver = StepperDriver4PWM(32, 15, 33, 27);

MagneticSensorI2C sensor = MagneticSensorI2C(AS5600_I2C);

// velocity set point variable
float target_velocity = 100;
// commander interface
Commander command = Commander(Serial);
void onTarget(char* cmd){ command.scalar(&target_velocity, cmd); }

void setup() {
  Serial.begin(115200);
  motor.useMonitoring(Serial);

  // init magnetic sensor hardware
  sensor.init();
  motor.linkSensor(&sensor);

  // power supply voltage
  // default 12V
  driver.voltage_power_supply = 24;
  driver.init();
  // link the motor to the driver
  motor.linkDriver(&driver);

  // set control loop type to be used
  motor.controller = MotionControlType::velocity;

  // velocity PI controller parameters
  // default P=0.5 I = 10
  // motor.PID_velocity.P = 0.2;
  // motor.PID_velocity.I = 20;
  //default voltage_power_supply
  motor.voltage_limit = 4;
  
  // velocity low pass filtering
  // default 5ms - try different values to see what is the best. 
  // the lower the less filtered
  // motor.LPF_velocity.Tf = 0.01;

  // initialize motor
  motor.init();
  // align encoder and start FOC
  motor.initFOC();

  // add target command T
  command.add('T', onTarget, "target velocity");

  // monitoring port
  // Serial.begin(115200);
  

  Serial.println("Motor ready.");
  Serial.println("Set the target velocity using serial terminal:");
  _delay(1000);
}


void loop() {
  // iterative foc function 
  motor.loopFOC();

  // iterative function setting and calculating the velocity loop
  // this function can be run at much lower frequency than loopFOC function
  motor.move(target_velocity);

  // user communication
  command.run();
}