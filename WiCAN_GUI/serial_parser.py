import serial
import time
import threading


class SerialData:
    def __init__(self):
        # TODO: Handle if no device plugged in
        #   Maybe make it so that it can handle wireless connections as well? -- depends on FW

        # FIXME: Change port to self.find_port()
        self.port = "COM6"
        self.baudrate = 115200
        self.serial_timeout = 0.02

        # FIXME: Increase this once the plot window size settings is changed
        self.max_data = 1000       # Max number of datapoints to store in each array
        self.delimiter = ";"
        self.data_buffer = ""
        self.print_buffer = []

        self.ser = serial.Serial(self.port, self.baudrate, timeout=self.serial_timeout)
        self.data = {}

        # Thread Params
        self.threadStop = False
        self.thread_interval = 0.005      # TODO: Faster thread slows down gui
        self.serial_thread = threading.Thread(target=self.pollingThread)
        self.serial_thread.start()

    def find_port(self):
        """
        Finds the serial port of the serial device
        :return:
        """
        # TODO: Implement
        pass

    def parse_data(self, data_list):
        # TODO: Implement dynamic typecasting - Currently casts everything to floats
        # TODO: Change this to parse CAN Messages
        # Takes in data
        for data in data_list:
            try:
                self.print_buffer.append(data)
                data = data.strip().split(self.delimiter)

                if data[0] != '':
                    for index in data:
                        data_split = index.split(" ")
                        data_key = data_split[0]
                        data_val = data_split[1]

                        # Check if key exists in data dictionary
                        if data_key in self.data.keys():
                            # Ensure list does not grow too large:
                            if len(self.data.get(data_key)["x"]) >= self.max_data:
                                self.data.get(data_key)["x"] = self.data.get(data_key)["x"][1:] + [time.clock()]
                                self.data.get(data_key)["y"] = self.data.get(data_key)["y"][1:] + [float(data_val)]
                            else:
                                self.data.get(data_key)["x"].append(time.clock())
                                self.data.get(data_key)["y"].append(float(data_val))

                        else:
                            # Add key and value to data dict
                            self.data.update({data_key: {"x": [], "y": []}})
                            self.data.get(data_key)["x"].append(time.clock())
                            self.data.get(data_key)["y"].append(float(data_val))

            except Exception as e:
                # Handle bad data at start of connection
                print("Caught", e)

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
        while not self.threadStop:
            try:
                self.data_buffer += self.ser.read(self.ser.inWaiting()).decode()
                new_data = self.data_buffer.split("\r\n")
                if len(new_data) >= 2:
                    self.data_buffer = new_data[-1]
                    self.parse_data(new_data[:-1])

            except Exception as e:
                print(f"Caught {e} in pollingThread")

            time.sleep(self.thread_interval)

    def stop_thread(self):
        """
        Stops the serial parsing thread
        :return:
        """
        self.threadStop = True
        time.sleep(self.thread_interval)
        self.ser.close()


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
