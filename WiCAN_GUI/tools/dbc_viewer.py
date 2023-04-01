"""
Tool to print out all messages and their signals in the console
"""
import time

import cantools

start = time.clock()
db = cantools.database.load_file("../resources/2018CAR.dbc")
print(f"Load Time: {time.clock()-start}s")

msg = example_message = db.get_message_by_name('ExampleMessage')

for message in db.messages:
    print(message.name)
    for signal in message.signals:
        print(f"\t{signal.name}")
