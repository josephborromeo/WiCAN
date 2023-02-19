"""
File to test out parsing CAN frames from the serial interface
"""

import serial
import time
import cantools
import can
import can.interfaces.slcan as slcan

run_duration = 10  # Seconds to run

port = "COM12"
baudrate = 115200
serial_timeout = 0.05

# ser = serial.Serial(port, baudrate, timeout=serial_timeout)
db = cantools.database.load_file("../resources/2018CAR.dbc")

print("Going to init bus")
bus = slcan.slcanBus(channel=port, ttyBaudrate=baudrate, bitrate=500000)
print("bus initialized")
# bus.open()


recv_data = []
data_buff = ""

print("Going into loop")
while time.clock() < run_duration:
    message = bus.recv()

    # try:
    # print(message.arbitration_id)
    # print(message.dlc)
    # print(message.data)

    decoded_data = db.decode_message(
        message.arbitration_id,
        message.data,
        decode_choices = True,      # Might have to be false
        decode_containers = False,    # might have to be true and handle the different containers (possibly with cell temps
        allow_truncated=True,
    )

    # except Exception as e:
    #     decoded_data = message
    #     print("not in DBC? Error:", e)

    for signal in decoded_data:
        print(f"Timestamp: {message.timestamp} \tSignal: {signal} \t Data: {decoded_data[signal]}")

    # data =ser.readline().decode().strip()
    # if data != "":
    #     print(data)
    # data_buff += ser.read(ser.inWaiting()).decode()
    # new_data = data_buff.split("\r\n")
    # if len(new_data) >= 2:
    #     data_buff = new_data[-1]
    #     print(new_data[:-1])
