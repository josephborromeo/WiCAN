import time
import threading
import cantools
import traceback

class FileData:
    def __init__(self, file_path):

        self.max_data = 1000000       # Max number of datapoints to store in each array
        self.delimiter = ";"
        self.data_buffer = ""
        self.print_buffer = []
        self.init_timestamp = -1

        self.db = cantools.database.load_file("resources/2018CAR.dbc")
        self.file_path = file_path
        self.data = {}

        # Thread Params
        self.threadStop = False
        self.serial_thread = threading.Thread(target=self.pollingThread)
        self.serial_thread.start()

    def parse_data(self, signal_dict, timestamp):
        for signal in signal_dict:
            try:
                if self.init_timestamp == -1:
                    self.init_timestamp = timestamp

                abs_timestamp = timestamp - self.init_timestamp
                data_val = signal_dict[signal]

                # Check if key exists in data dictionary
                if signal in self.data.keys():
                    # Ensure list does not grow too large:
                    if len(self.data.get(signal)["x"]) >= self.max_data:
                        self.data.get(signal)["x"] = self.data.get(signal)["x"][1:] + [abs_timestamp]
                        self.data.get(signal)["y"] = self.data.get(signal)["y"][1:] + [float(data_val)]
                    else:
                        self.data.get(signal)["x"].append(abs_timestamp)
                        self.data.get(signal)["y"].append(float(data_val))

                else:
                    # Add key and value to data dict
                    self.data.update({signal: {"x": [], "y": []}})
                    self.data.get(signal)["x"].append(abs_timestamp)
                    self.data.get(signal)["y"].append(float(data_val))

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
        with open(self.file_path, 'r') as file:
            while not self.threadStop:
                try:
                    line = file.readline()
                    if len(line) > 1:
                        split_str = line.split()
                        timestamp = float(split_str[0].strip("()"))
                        arbitration_id = int(split_str[2], 16)
                        data = b''
                        for byte in split_str[4:]:
                            data += int(byte, 16).to_bytes(1, 'little')
                        if len(data) > 0 and timestamp > 0 and arbitration_id is not None:
                            decoded_data = self.db.decode_message(
                                arbitration_id,
                                data,
                                decode_choices=False,
                                decode_containers=False,
                                allow_truncated=True,
                            )

                            self.parse_data(decoded_data, timestamp)
                except KeyError as ke:
                    pass
                except Exception as e:
                    print(f"Caught {e} in pollingThread")
                    traceback.print_exc()


    def stop_thread(self):
        """
        Stops the file reading thread
        :return:
        """
        self.threadStop = True
        time.sleep(0.5)

