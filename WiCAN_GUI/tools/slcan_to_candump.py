import time
import cantools
from can import Message
import can.interfaces.slcan as slcan

db = cantools.database.load_file("../resources/2018CAR.dbc")

f = open("slcan_log_file.txt", "r")

f_parsed = open("slcan_log_file_candump.log", "w")

for x in f:
    data = x.strip().split(" ")
    readStr = data[0]
    time_stamp = float(data[1])
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
                      timestamp=time_stamp,   # Better than nothing...
                      is_remote_frame=False,
                      dlc=dlc,
                      data=frame)


        data = ""
        for j in range(dlc):
            data += hex(msg.data[j])[2:] + " "

        f_parsed.write(f"({msg.timestamp})  can1  {hex(msg.arbitration_id)[2:]}   [{msg.dlc}]  {data}\n")

f.close()
f_parsed.close()