#!/usr/bin/env python3
"""CAN Bus Scanner - Scan and sync SimpleFOC motors"""
import time
import can
from simplefoc import packets as p

bus = can.interface.Bus(channel='can0', interface="socketcan", bitrate=1000000)
responses, start = {}, time.time()

board = p.CANComms(bus, target_address=0)
board.connect()

ind = 0
board.observable().subscribe(lambda frame: print(f"Slave {ind} responded."))

for i in range(256):
    if i % 20 == 0:
        print(f"Scanning address {i}-{i+20}...")
    ind = i
    board.target_address = i
    board.send_frame(p.Frame(frame_type=p.FrameType.SYNC))
    time.sleep(0.05)
    