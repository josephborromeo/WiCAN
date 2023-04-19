import serial
import time
from can import Message
import cantools

ser = serial.Serial(port='COM12', baudrate=115200)
db = cantools.database.load_file("../resources/2018CAR.dbc")

start_time = time.perf_counter()
run_duration = 60

num_msg = 0

while time.perf_counter() < run_duration:
    data = ser.read_until(b'\r')
    readStr = data.decode().strip()
    # print(readStr)
    frame = []
    if readStr[0] == 'T':
        # extended frame
        canId = int(readStr[1:9], 16)
        dlc = int(readStr[9])
        extended = True
        for i in range(0, dlc):
            frame.append(int(readStr[10 + i * 2:12 + i * 2], 16))

        msg = Message(arbitration_id=canId,
                      is_extended_id=extended,
                      timestamp=time.perf_counter(),   # Better than nothing...
                      is_remote_frame=False,
                      dlc=dlc,
                      data=frame)

        try:
            decoded_data = db.decode_message(
                msg.arbitration_id,
                msg.data,
                decode_choices = True,      # Might have to be false
                decode_containers = False,    # might have to be true and handle the different containers (possibly with cell temps
                allow_truncated=True,
            )
            print(decoded_data)
        except:
            pass

        num_msg += 1

    if time.perf_counter() - start_time > 1:
        print(f"{float(num_msg) / (time.perf_counter() - start_time):.2f} msgs / s")
        num_msg = 0
        start_time = time.perf_counter()

