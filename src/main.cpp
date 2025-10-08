/*
  MicroSpora - Closed loop Torque Control Example
*/

#include "Arduino.h"
#include <SimpleFOC.h>
#include <SimpleFOCDrivers.h>
#include "drivers/drv8316/drv8316.h"
#include "encoders/mt6701/MagneticSensorMT6701SSI.h"

// motor instance
BLDCMotor motor = BLDCMotor(11);
DRV8316Driver6PWM driver = DRV8316Driver6PWM(PHA_H, PHA_L, PHB_H, PHB_L, PHC_H, PHC_L, DRV_CS, false);

// sensor instance
MagneticSensorMT6701SSI sensor = MagneticSensorMT6701SSI(ENC_CS);

// current sense instance
LowsideCurrentSense current_sense = LowsideCurrentSense(375.0f, (int)ISENS_A, (int)ISENS_B, (int)ISENS_C);


// SPI configuration for the encoder and driver
SPIClass SPI_1(ENC_NC, ENC_SDO, ENC_CLK); // use the SPI1 for the encoder
SPIClass SPI_3(PB5_ALT1, PB4_ALT1, PB3_ALT1); // use SPI3 for the driver ( need to use alternate pins for SCK, MOSI and MISO)

// instantiate the commander
Commander command = Commander(Serial);

// simple function to be called upon recieving commands
void doMotor(char* cmd){
  command.motor(&motor, cmd);
}


void setup() {
  _delay(3000);
  // use monitoring with serial
  Serial.begin(250000);
  // enable more verbose output for debugging
  // comment out if not needed
  SimpleFOCDebug::enable(&Serial);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("Initializing...");

  //motor.sensor_direction = Direction::CCW;
  //motor.zero_electric_angle = 3.6938;
  digitalWrite(ENC_CS, HIGH);

  sensor.init(&SPI_1);        //  Initialize sensor on SPI_1 bus
  motor.linkSensor(&sensor);  // link the motor to the sensor

  Serial.print("Sensor initialized: ");

  // driver config
  driver.voltage_power_supply = 20.0;
  driver.pwm_frequency = 25000;
  driver.dead_zone = 0.001f;
  driver.init(&SPI_3);
  driver.setSlew(Slew_200Vus);
  driver.setCurrentSenseGain(Gain_0V375);
  driver.setOCPMode(NoAction);
  driver.setPWMMode(PWM6_Mode);
  driver.setBuckVoltage(VB_5V);


  Serial.println("Driver initialized");

  current_sense.linkDriver(&driver);
  current_sense.init();
  motor.linkCurrentSense(&current_sense);
  Serial.println("Current sense initialized");

  motor.linkDriver(&driver);                          // link the motor and the driver
  motor.foc_modulation = FOCModulationType::SinePWM;  // set FOC modulation type
  // TorqueControlType::foc_current
  motor.torque_controller = TorqueControlType::voltage;
  // set motion control loop to be used
  motor.controller = MotionControlType::torque;

  motor.useMonitoring(Serial);  // comment out if not needed

  // init motor hardware
  if (!motor.init()) {
    Serial.println("Motor init failed!");
    return;
  }

  motor.initFOC();
  Serial.println("Init complete...");

  // subscribe motor to the commander
  command.add('M', doMotor, "Set motor target");

  digitalWrite(LED_BUILTIN, LOW);
  _delay(200);
}


void loop() {

  // main FOC algorithm function
  motor.loopFOC();

  // Motion control function
  // velocity, position or torque (defined in motor.controller)
  // this function can be run at much lower frequency than loopFOC() function
  // You can also use motor.move() and set the motor.target in the code
  motor.move();

  // function intended to be used with serial plotter to monitor motor variables
  // significantly slowing the execution down!!!!
  // motor.monitor();

  // user communication
  command.run();
}