"""
Tool to generate and array of CAN frames to be send on an arduino to spoof the ESP
"""
import time
import random
import cantools

start = time.clock()
db = cantools.database.load_file("../resources/2018CAR.dbc")
print(f"Load Time: {time.clock()-start}s")

num_messages = 2

message_name = 'SpeedMotorKPH'
message = db.get_message_by_name(message_name)

b_array = []
print("{")
for i in range(num_messages):
    print("\t{")
    speed_l = int(i*5)
    speed_r = int(speed_l*(1+random.uniform(0, 0.1)))
    b_arr = message.encode({'SpeedMotorLeftKPH': speed_l, 'SpeedMotorRightKPH': speed_r})
    # print(speed_l, speed_r)
    # print("Decoded:", message.decode(b_arr))
    print("\t\t", end="")
    print(*list(b_arr), sep=", ")

    print("\t},")
print("};")