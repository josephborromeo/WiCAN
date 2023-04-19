import serial
import time

ser = serial.Serial(port='COM12', baudrate=115200)

f = open("slcan_log_file.txt", "w")

while True:
    text = ser.read_until(b'\r').decode('utf-8')
    f.write(f"{text[:-1]} {time.perf_counter():.3f}\n")

f.close()

