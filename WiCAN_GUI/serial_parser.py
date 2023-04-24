import serial
import time
import threading
import cantools
import can.interfaces.slcan as slcan
import sys


class SerialData:
    def __init__(self):
        # TODO: Handle if no device plugged in
        #   Maybe make it so that it can handle wireless connections as well? -- depends on FW

        # FIXME: Change port to self.find_port()
        self.port = "COM12"
        self.baudrate = 1000000
        self.serial_timeout = 0.001

        # FIXME: Increase this once the plot window size settings is changed
        self.max_data = 1000       # Max number of datapoints to store in each array
        self.delimiter = ";"
        self.data_buffer = ""
        self.print_buffer = []

        self.db = cantools.database.load_file("resources/2018CAR.dbc")
        self.bus = slcan.slcanBus(channel=self.port, ttyBaudrate=self.baudrate, bitrate=500000)
        self.data = {}

        # sys.setswitchinterval(0.0005)

        # Thread Params
        self.threadStop = False
        self.thread_interval = 0.001      # TODO: Faster thread slows down gui
        self.serial_thread = threading.Thread(target=self.pollingThread)
        self.serial_thread.start()

    def find_port(self):
        """
        Finds the serial port of the serial device
        :return:
        """
        # TODO: Implement
        pass

    def parse_data(self, signal_dict):
        # TODO: Implement dynamic typecasting - Currently casts everything to floats
        # Takes in data
        for signal in signal_dict:
            try:
                data_val = signal_dict[signal]

                # Check if key exists in data dictionary
                if signal in self.data.keys():
                    # Ensure list does not grow too large:
                    if len(self.data.get(signal)["x"]) >= self.max_data:
                        self.data.get(signal)["x"] = self.data.get(signal)["x"][1:] + [time.perf_counter()]
                        self.data.get(signal)["y"] = self.data.get(signal)["y"][1:] + [float(data_val)]
                    else:
                        self.data.get(signal)["x"].append(time.perf_counter())
                        self.data.get(signal)["y"].append(float(data_val))

                else:
                    # Add key and value to data dict
                    self.data.update({signal: {"x": [], "y": []}})
                    self.data.get(signal)["x"].append(time.perf_counter())
                    self.data.get(signal)["y"].append(float(data_val))

            except Exception as e:
                # Handle bad data at start of connection
                # print("Caught", e)
                pass

        return None

    def get_data(self):
        """
        Returns the latest
        :return:
        """

        return self.data

    def pollingThread(self):
        """
        Thread that continually polls the serial port for new data
        :return:
        """

        # TODO: Improve this to use some sort of timer instead of a sleep function
        count = 0
        start_time = time.perf_counter()
        counter_start_time = time.perf_counter()
        incoming_queue = []
        while not self.threadStop:
            while len(incoming_queue) < 50 and (time.perf_counter() - start_time) < 0.5:
                try:
                    message = self.bus.recv(.1)    # Need to Add timeout or else app wont close :(

                    if message is not None:
                        count += 1

                        if time.perf_counter()-counter_start_time >= 1:
                            print(f"{count} msgs/s")
                            count = 0
                            counter_start_time = time.perf_counter()
                        decoded_data = self.db.decode_message(
                            message.arbitration_id,
                            message.data,
                            decode_choices=False,   # TODO: Handle signals with text choices
                            decode_containers=False,    # might have to be true and handle the different containers (possibly with cell temps
                            allow_truncated=True,
                        )
                        incoming_queue.append(decoded_data)

                        self.parse_data(decoded_data)

                except Exception as e:
                    # TODO: Handle not in DBC
                    print(f"Caught {e} in pollingThread")
                    # pass

            # s_parse = time.perf_counter()
            # # for i in incoming_queue:
            # #     self.parse_data(i)
            # print(f"took {time.perf_counter()-s_parse}s to parse")
            incoming_queue = []
            time.sleep(self.thread_interval)
            start_time = time.perf_counter()

    def stop_thread(self):
        """
        Stops the serial parsing thread
        :return:
        """
        self.threadStop = True
        time.sleep(self.thread_interval)
        self.bus.close()


# data = SerialData()
#
# for i in range(200):
#     # print(data.get_data())
#     # print(i)
#     time.sleep(0.1)
#
# print(data.get_data())
#
# data.stop_thread()
#
# import matplotlib.pyplot as plt
#
# s_data = data.get_data()
#
# fig, ax = plt.subplots()
#
# for signal in s_data.keys():
#     print(signal)
#     print("X", s_data.get(signal)['x'])
#     print("Y", s_data.get(signal)['y'])
#     ax.plot(s_data.get(signal)['x'], s_data.get(signal)['y'], label=signal)
#
# plt.locator_params(nbins=10)
# plt.legend()
# plt.show()
