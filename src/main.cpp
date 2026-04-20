/*
  MicroSpora - Closed loop Torque Control Example
*/

#include "Arduino.h"
#include <SimpleFOC.h>
#include "current_sense/hardware_specific/stm32/stm32g4/stm32g4_hal.h"
#include <SimpleFOCDrivers.h>
#include "drivers/drv8316/drv8316.h"
#include "encoders/mt6701/MagneticSensorMT6701SSI.h"
#include "encoders/calibrated/CalibratedSensor.h"
#include "comms/can/CANCommander.h"
#include "SimpleCANio.h"

#define MOT_POLE_PAIRS 14

// motor instance
BLDCMotor motor = BLDCMotor(MOT_POLE_PAIRS); // 7 pole pairs, 2.04 Ohm, 0.35 mH
DRV8316Driver6PWM driver = DRV8316Driver6PWM(PHA_H, PHA_L, PHB_H, PHB_L, PHC_H, PHC_L, DRV_CS, false);

// sensor instance
SPISettings MT6701SSISettings_new(10000000, MT6701_BITORDER, SPI_MODE2);
MagneticSensorMT6701SSI sensor = MagneticSensorMT6701SSI(ENC_CS, MT6701SSISettings_new);

// current sense instance
LowsideCurrentSense current_sense = LowsideCurrentSense(375.0f, (int)ISENS_A, (int)ISENS_B, (int)ISENS_C);

// instantiate the calibrated sensor object, 
// CalibratedSensor sensor_calibrated = CalibratedSensor(sensor,, 200); // 200 points in the LUT

// SPI configuration for the encoder and driver
SPIClass SPI_1(ENC_NC, ENC_SDO, ENC_CLK); // use the SPI1 for the encoder
SPIClass SPI_3(PB5_ALT1, PB4_ALT1, PC10); // use SPI3 for the driver ( need to use alternate pins for SCK, MOSI and MISO)


CANio can(CAN_RX, CAN_TX, NC, CAN_ENABLE); // Create CAN object
CANCommander command_can(can, 15); // listen at the address 15 on the CAN bus

// instantiate the commander
Commander command = Commander(Serial);
// simple function to be called upon receiving commands
void doMotor(char* cmd){
  command.motor(&motor, cmd);
}


void setup() {
  _delay(6000);
  // use monitoring with serial
  Serial.begin(250000);
  // enable more verbose output for debugging
  // comment out if not needed
  SimpleFOCDebug::enable(&Serial);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println("Initializing...");

  // motor.sensor_direction = Direction::CCW;
  // motor.zero_electric_angle = 3.00;
  digitalWrite(ENC_CS, HIGH);

  sensor.init(&SPI_1);        //  Initialize sensor on SPI_1 bus
  motor.linkSensor(&sensor);  // link the motor to the sensor

  Serial.print("Sensor initialized: ");

  float voltage =_readRegularADCVoltage(VSENS)* VSCALE; 
  Serial.print("Supply voltage: ");
  Serial.print(voltage);
  Serial.println(" V");

  while(voltage < 6.0f){
    Serial.println("Voltage too low for motor operation!");
    _delay(1000);
  }


  // driver config
  driver.voltage_power_supply = voltage;
  driver.pwm_frequency = 25000;
  driver.dead_zone = 0.001f; 
  driver.init(&SPI_3);
  driver.setSlew(Slew_200Vus); // add the highest slew rate to make the voltage changes sharper
  driver.setCurrentSenseGain(Gain_0V375); // configure the most sensitive gain for the current sense measurements
  driver.setOCPMode(NoAction); 
  driver.setPWMMode(PWM6_Mode); 
  driver.setBuckVoltage(VB_5V);

  Serial.println("Driver initialized");

  current_sense.linkDriver(&driver);
  current_sense.init();
  current_sense.skip_align = true; // no need to align 
  motor.linkCurrentSense(&current_sense);
  Serial.println("Current sense initialized");

  motor.linkDriver(&driver);                          // link the motor and the dr
  // TorqueControlType::foc_current
  motor.torque_controller = TorqueControlType::foc_current;
  // set motion control loop to be used
  motor.controller = MotionControlType::torque;
  motor.useMonitoring(Serial);  // comment out if not needed
  motor.monitor_end_char = 'M'; // set monitoring end character to M for better parsing in the odrive tool
  motor.monitor_start_char = 'M'; // set monitoring start character to S for better parsing in the odrive tool
  // init motor hardware
  if (!motor.init()) {
    Serial.println("Motor init failed!");
    return;
  }

  // sensor eccentricity calibration if necessary
  // sensor_calibrated.voltage_calibration = 2.0f; // voltage to be applied during the calibration
  // sensor_calibrated.calibrate(motor); // run the calibration
  // motor.linkSensor(&sensor_calibrated);  // link the motor to the sensor
  // sensor_calibrated.printLUT(motor, Serial); // print the LUT to serial monitor
  // sensor_calibrated.min_elapsed_time = 0.0005f; // set minimum elapsed time between updates to 500us
  motor.initFOC();
  Serial.println("Init complete...");
  
  command_can.init();
  command_can.addMotor(&motor);

  // subscribe motor to the commander
  command.add('M', doMotor, "Set motor target");

  // determine the motor parameters (optional)
  motor.characteriseMotor(3.0f); // characterise motor with 2V voltage
  motor.tuneCurrentController(100.0f); // tune current controller with 100rad/s bandwidth
  
  digitalWrite(LED_BUILTIN, LOW);
  _delay(200);

  // create a hardware timer
  // For example, we will create a timer that runs at 10kHz on the TIM8
  HardwareTimer* timer = new HardwareTimer(TIM8);
  // Set timer frequency to 10kHz
  timer->setOverflow(10000, HERTZ_FORMAT); 
  // add the loopFOC and move to the timer
  timer->attachInterrupt([](){
    // call the loopFOC and move functions
    motor.loopFOC();
    motor.move();
  });
  // start the timer
  timer->resume();
}
// do non-time critical tasks in the main loop
void loop() {
  // monitor the motor variables and send them to serial
  motor.monitor();

  // user communication
  command.run();
  // CAN communication (does nothing if no can sent/received)
  command_can.run();
}