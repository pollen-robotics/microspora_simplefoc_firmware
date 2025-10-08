Microspora board firmware based on SimpleFOC using platformIO
========================================

A template platformIO project for the microspora firmware using SimpleFOC.

- microspora by [@Rambos](https://github.com/rambros3d) : https://oshwlab.com/rambros/nano-8316-motor-driver
- SimpleFOC by [@simplefoc](https://github.com/simplefoc/Arduino-FOC): https://simplefoc.com/

This is the firmware for the microspora board, a compact motor driver board based on the DRV8316 IC and an STM32G4 series microcontroller, and MT6701 encoder. The board is designed for BLDC motors under 10Amps, and is compatible with the SimpleFOC library. It is super compact and it seems to be a very very nice fit to start with SimpleFOC!

It can be used for torque, velocity and position control.

## Requirements

You'll need to have [PlatformIO](https://platformio.org/) installed and to download the SimpleFOC, as well as the SimpleFOC-drivers library into your `lib` folder. 

I'd suggest you to do it like this:

```
cd your_project_folder/lib
git clone https://github.com/simplefoc/Arduino-FOC.git
git clone https://github.com/simplefoc/SimpleFOC-drivers.git
```

