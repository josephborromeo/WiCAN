import os
import time
from pathlib import Path

import cantools


# Directory where CAN .log files are stored, relative to base of repo
LOG_DIRECTORY = Path("Software/Tests/CAN_logs")

FILES = os.listdir(LOG_DIRECTORY)

db = cantools.database.load_file("WiCAN_GUI/resources/2018CAR.dbc")

line_counter = 0

start = time.perf_counter()
for log_file in FILES:
    with open(LOG_DIRECTORY / log_file, 'r') as file:
        lines = file.readlines()
        for line in lines:
            line_counter += 1
            # try:
            #     if len(line) > 1:
            #         split_str = line.split()
            #         timestamp = float(split_str[0].strip("()"))
            #         arbitration_id = int(split_str[2], 16)
            #         data = b''
            #         for byte in split_str[4:]:
            #             data += int(byte, 16).to_bytes(1, 'little')
            #         if len(data) > 0 and timestamp > 0 and arbitration_id is not None:
            #             decoded_data = db.decode_message(
            #                 arbitration_id,
            #                 data,
            #                 decode_choices=False,
            #                 decode_containers=False,
            #                 allow_truncated=True,
            #             )
            # except Exception as e:
            #     # print(f"Caught Exception {e}")
            #     pass

print(f"Took {time.perf_counter() - start:4f} seconds to parse {line_counter} lines")

