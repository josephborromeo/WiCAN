"""
File to test out parsing CAN frames from the serial interface
"""

import serial
import time
import cantools
import can
import can.interfaces.slcan as slcan

run_duration = 120  # Seconds to run

port = "COM12"
baudrate = 5000000
serial_timeout = 0.05

# ser = serial.Serial(port, baudrate, timeout=serial_timeout)
db = cantools.database.load_file("../resources/2018CAR.dbc")

print("Going to init bus")
bus = slcan.slcanBus(channel=port, ttyBaudrate=baudrate, bitrate=500000, rtscts=True)


# print(bus.poll_interval)
print("bus initialized")


recv_data = []
data_buff = ""

# tx_msg = db.get_message_by_name("DCU_buttonEvents")
# msg_data = tx_msg.encode({"ButtonEnduranceToggleEnabled":0, "ButtonEnduranceLapEnabled":0, "ButtonTCEnabled":0, "ButtonHVEnabled": 1, "ButtonEMEnabled": 0})
#
# msg = can.Message(arbitration_id=tx_msg.frame_id, data=msg_data, is_extended_id=True)
bus.open()
bus.close()

bus.open()



# tx_msg = db.get_message_by_name("UartOverCanConfig")
# msg_data = tx_msg.encode({"UartOverCanConfigSignal": 100})
# msg = can.Message(arbitration_id=tx_msg.frame_id, data=msg_data)

#
# time.sleep(0.02)
# bus.send(msg)
# print("send msg")
# time.sleep(0.02)
# bus.send(msg)
# time.sleep(0.02)
# bus.send(msg)
# time.sleep(0.02)
# bus.send(msg)
# time.sleep(0.02)



print("Going into loop")
start_time = time.perf_counter()
num_msg = 0
while time.perf_counter() < run_duration:
    try:
        message = bus.recv(None)
    except Exception:
        print("exception")
    # bus.recv(None)
    num_msg += 1


    if time.perf_counter() - start_time > 1:
        print(f"{float(num_msg) / (time.perf_counter() - start_time):.2f} msgs / s")
        num_msg = 0
        start_time = time.perf_counter()

    # try:
    # print(message.arbitration_id)
    # print(message.dlc)
    # print(message.data)

    # decoded_data = db.decode_message(
    #     message.arbitration_id,
    #     message.data,
    #     decode_choices = True,      # Might have to be false
    #     decode_containers = False,    # might have to be true and handle the different containers (possibly with cell temps
    #     allow_truncated=True,
    # )

    # except Exception as e:
    #     decoded_data = message
    #     print("not in DBC? Error:", e)

    # for signal in decoded_data:
    #     print(f"Timestamp: {message.timestamp} \tSignal: {signal} \t Data: {decoded_data[signal]}")

    # data =ser.readline().decode().strip()
    # if data != "":
    #     print(data)
    # data_buff += ser.read(ser.inWaiting()).decode()
    # new_data = data_buff.split("\r\n")
    # if len(new_data) >= 2:
    #     data_buff = new_data[-1]
    #     print(new_data[:-1])
