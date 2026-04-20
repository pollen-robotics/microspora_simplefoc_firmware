Python example for testing CAN communication with the Microspora using the SimpleFOC CAN protocol.

Install `pysimplefoc` for this:

```bash
pip install pyserial rx python-can
pip install git+https://github.com/simplefoc/pysimplefoc.git
```

Setup can0 interface on Linux on the USB-CAN adapter (e.g. CANable) connected to
e.g. `/dev/ttyACM2`:
```
sudo slcand -o -c -s6 /dev/ttyACM2 can0
# Linux SocketCAN setup
sudo ip link set can0 type can bitrate 1000000
sudo ip link set can0 up
```

To scan the CAN bus for devices:
```sh
python can_scripts/scan_bus.py
```

To send a command to the motor controller:
```sh
python can_scripts/move.py
```

See [pysimplefoc documentation](https://github.com/simplefoc/pysimplefoc) for more details on how to use the library for CAN communication with SimpleFOC devices.