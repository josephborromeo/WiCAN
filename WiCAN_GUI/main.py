#!/usr/bin/env python
from GUI.GUI_methods import start_GUI
import argparse
from serial_parser import SerialData
from file_parser import FileData
import pathlib

def main():
    parser = argparse.ArgumentParser(prog="WiCAN", description="CAN Log Visualizer")
    parser.add_argument('-f', '--file', type=pathlib.Path)
    args = parser.parse_args()

    if args.file is not None:
        data_handler = FileData(args.file)
    else:
        data_handler = SerialData()

    start_GUI(data_handler)


if __name__ == '__main__':
    main()
