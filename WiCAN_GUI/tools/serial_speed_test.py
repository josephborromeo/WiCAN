import serial
import time

ser = serial.Serial(port='COM12', baudrate=115200)

start_time = time.perf_counter()
run_duration = 60

num_msg = 0

while time.perf_counter() < run_duration:
    ser.read_until(b'\r')

    num_msg += 1

    if time.perf_counter() - start_time > 1:
        print(f"{float(num_msg) / (time.perf_counter() - start_time):.2f} msgs / s")
        num_msg = 0
        start_time = time.perf_counter()

