
import simplefoc.packets as packets
from simplefoc import TorqueControlType
from simplefoc import MotionControlType
from simplefoc import motors as m
import time, sys, signal

import numpy as np

CAN_ID = 15
TARGET_VOLTAGE= 2

# Connect to CAN
motors = m.can('can0', target_address=CAN_ID, bitrate=1000000)
motors.connect()

# Get motor
motor = motors.motor(0)

time.sleep(1)   
motor.set_limits(max_voltage=12.0, max_current=1.0, max_velocity=120.0)
motor.set_mode(MotionControlType.torque, TorqueControlType.voltage)

motor.enable()

try:
    print("Angle PID set.", motor.get_angle_pid())
except Exception as e:
    print("Error getting angle PID:", e)

print("Velocity PID set.", motor.get_velocity_pid())
def signalhandler(sig, frame):
    print("Disabling motor...")
    motor.set_target(0.0)
    time.sleep(1.0)
    motor.disable()
    motors.disconnect()
    print("Serial port closed.")
    sys.exit(0)

signal.signal(signal.SIGINT, signalhandler)

while True:
    err = 0
    s = time.time()
    for i in range(1000):
        target = TARGET_VOLTAGE*np.sin(time.time()*2.0)
        motor.set_target(target)
        time.sleep(0.001)
    e = time.time()
    print(f"----------------------------------\nTransfer Rate:{1000/(e-s)}msg/s" )

