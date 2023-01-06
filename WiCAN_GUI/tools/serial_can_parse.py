"""
File to test out parsing CAN frames from the serial interface
"""

import serial
import time
import cantools

run_duration = 20  # Seconds to run

port = "COM6"
baudrate = 115200
serial_timeout = 0.05

ser = serial.Serial(port, baudrate, timeout=serial_timeout)
db = cantools.database.load_file("../resources/2018CAR.dbc")

recv_data = []
data_buff = ""

while time.clock() < run_duration:
    data =ser.readline().decode().strip()
    if data != "":
        print(data)
    # data_buff += ser.read(ser.inWaiting()).decode()
    # new_data = data_buff.split("\r\n")
    # if len(new_data) >= 2:
    #     data_buff = new_data[-1]
    #     print(new_data[:-1])
