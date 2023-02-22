import random
import sys
import time

from PyQt5 import QtGui, QtCore, QtWidgets, uic
import pyqtgraph as pg
from pyqtgraph.ptime import time as ptime
import numpy as np

from serial_parser import SerialData
from GUI.config_parser import get_dbc_path

class LiveDataPlot:
    def __init__(self, data):
        # Create a plot window to avoid the plot from opening in a separate window
        self.plot_window = pg.GraphicsLayoutWidget(show=False)
        self.plot = self.plot_window.addPlot()
        self.plot.addLegend()
        self.plot.showGrid(x=True, y=True)

        # Enable Anti-aliasing for nice plots
        pg.setConfigOptions(antialias=True)

        # Auto-fit data + enables scrolling effect
        self.plot.enableAutoRange('xy', True)
        self.data = data

        self.curve_objs = {}

    def update_plot(self, selected_signals):
        # Get new serial data and plot each signal
        data_dict = self.data.get_data()

        # Check only selected signals
        for signal in selected_signals:
            if signal in data_dict.keys():
                if signal in self.curve_objs.keys():
                    # Exists already
                    self.curve_objs.get(signal).setData(data_dict.get(signal)['x'], data_dict.get(signal)['y'])
                else:
                    # Curve Obj needs to be created
                    if len(data_dict.get(signal)['x']) > 5:  # Ensure there is more than 5 data points to plot
                        print("New Plot for", signal)

                        self.curve_objs.update(
                            {signal: pg.PlotCurveItem(
                                x=data_dict.get(signal)['x'],
                                y=data_dict.get(signal)['y'],
                                name=signal,
                                pen=self.select_color())}
                        )
                        self.plot.addItem(self.curve_objs.get(signal))

        # Check if plotted signals are not part of selected - want to remove
        for plotted_signal in self.curve_objs.keys():
            if plotted_signal not in selected_signals:
                # Remove from plot and dict:
                self.plot.removeItem(self.curve_objs[plotted_signal])
                del self.curve_objs[plotted_signal]
                break

    def select_color(self):
        """
        Selects the color for the new plot, ensures that colors cannot be duplicated
        :return:
        """
        plotted_colors = [curve.opts.get("pen").color() for curve in self.curve_objs.values()]
        c_iter = 0
        color = pg.intColor(c_iter)
        while color in plotted_colors and c_iter < 9:
            c_iter += 1
            color = pg.intColor(c_iter)

        return color


class CanLiveDataView:
    """
    All details of CAN data being received
    Details required:
        - CAN-ID (hex)
        - DLC
        - Symbol (Message Name)
        - Signal Name
        - Signal Value (converted)
        - Units (optional)
        - Cycle Time (ms)
        - Receive Count
    """
    def __init__(self, tableView: QtWidgets.QTableWidget):
        self.table = tableView
        self.displayed_signals = {}  # Signal: RowNum

        self.data = {}

        # TODO: Setup column widths, font sizes, etc.

    def update_items(self, data):
        # Signal Column = index 3
        # Value Column = index 4

        for key in data.keys():
            if key not in self.displayed_signals.keys() and len(data.get(key).get("x")) > 5:
                numRows = self.add_row()
                self.table.setItem(numRows, 3, QtWidgets.QTableWidgetItem(key))  # Signal

                val = "{0:.2f}".format(data.get(key).get("y")[-1])
                self.table.setItem(numRows, 4, QtWidgets.QTableWidgetItem(str(val)))  # Value
                self.displayed_signals.update({key: numRows})

            elif len(data.get(key).get("x")) > 5:
                # Has already been added, just update values
                row = self.displayed_signals.get(key)
                val = "{0:.2f}".format(data.get(key).get("y")[-1])
                self.table.item(row, 4).setText(val)

    def add_row(self):
        # Create a empty row at bottom of table
        numRows = self.table.rowCount()
        self.table.insertRow(numRows)
        return numRows


class CanSignalTable:
    def __init__(self, treeView: QtWidgets.QTreeWidget):
        self.tree = treeView
        self.display_signals = []
        self.selected_signals = []

    def update_items(self, data_keys):
        """
        :param data_keys: Keys from main data dict - don't care about actual data so save space/ increase speed
        :return:
        """
        for key in data_keys:
            if key not in self.display_signals:
                treeItem = QtWidgets.QTreeWidgetItem(self.tree)
                treeItem.setText(0, key)
                treeItem.setFlags(treeItem.flags() | QtCore.Qt.ItemIsUserCheckable)
                treeItem.setCheckState(0, QtCore.Qt.CheckState.Unchecked)
                self.tree.addTopLevelItem(treeItem)
                self.display_signals.append(key)

    def update_selected(self):
        """
        Return list of selected signals to plot
        :return:
        """
        for item_num in range(self.tree.topLevelItemCount()):
            sig_name = self.tree.topLevelItem(item_num).text(0)
            checked = bool(self.tree.topLevelItem(item_num).checkState(0))

            if checked and sig_name not in self.selected_signals:
                self.selected_signals.append(sig_name)
            elif not checked and sig_name in self.selected_signals:
                self.selected_signals.remove(sig_name)

    def get_selected(self):
        return self.selected_signals


class Ui(QtWidgets.QMainWindow):
    def __init__(self):
        super(Ui, self).__init__()
        uic.loadUi('GUI/main_window.ui', self)

        self.frame_rate = 0
        self.fps_time = ptime()

        self.data = SerialData()
        self.data_plotter = LiveDataPlot(self.data)

        self.plotLayout.addWidget(self.data_plotter.plot_window)

        # Create signal selector for GUI
        self.signal_table = CanSignalTable(self.can_signal_tree)
        self.data_table = CanLiveDataView(self.CANdataTable)

        # Setup Table of Signals
        # TODO: Make class for signal table

        self.show()

        self.pause_graph = False

        # Timer to Update the Graph
        self.update_timer = QtCore.QTimer()
        self.update_timer.setInterval(1)
        self.update_timer.timeout.connect(self.update_plot)
        self.update_timer.timeout.connect(self.fps_counter)

        self.update_timer.start()

        # Timer to update other things
        self.slow_update_timer = QtCore.QTimer()
        self.slow_update_timer.setInterval(50)      # TODO: Can likely make this slower
        self.slow_update_timer.timeout.connect(self.update_time)
        self.slow_update_timer.timeout.connect(self.update_serial_viewer)
        self.slow_update_timer.timeout.connect(self.update_tree)
        self.slow_update_timer.timeout.connect(self.update_table)
        self.slow_update_timer.start()

        # Connect Buttons
        self.play_pause_btn.clicked.connect(self.toggle_pause)
        self.dbc_filediag_btn.clicked.connect(self.get_dbc_path)

    def update_plot(self):
        self.data_plotter.update_plot(self.signal_table.get_selected())

    def update_time(self):
        self.CurrTimeLabel.setText(f"Current Time: {time.clock():.2f}s")

    def toggle_pause(self):
        self.pause_graph = not self.pause_graph

        if self.pause_graph:
            self.update_timer.timeout.disconnect(self.update_plot)
        else:
            self.update_timer.timeout.connect(self.update_plot)

    def get_dbc_path(self):
        dbc_path = get_dbc_path()
        self.dbc_path_textedit.clear()
        self.dbc_path_textedit.insertPlainText(str(dbc_path))
        self.dbc_path_textedit.moveCursor(QtGui.QTextCursor.End)

    def fps_counter(self):
        now = ptime()
        dt = now - self.fps_time
        self.fps_time = now
        if self.frame_rate is None:
            self.frame_rate = 1.0/dt
        else:
            s = np.clip(dt*3., 0, 1)
            self.frame_rate = self.frame_rate * (1-s) + (1.0/dt) * s

        self.FPS_label.setText(f"FPS: {self.frame_rate:.2f}")

    def update_serial_viewer(self):
        if len(self.data.print_buffer) != 0:
            for received in self.data.print_buffer:
                self.plainTextEdit.insertPlainText(received + "\n")
                self.plainTextEdit.moveCursor(QtGui.QTextCursor.End)  # Ensure Last line is always shown
            self.data.print_buffer = []     # FIXME: might not want to completely reset the list but probably ok

    def update_tree(self):
        keys = [k for k in self.data.data.keys() if len(self.data.data[k]["x"]) > 5]
        self.signal_table.update_items(keys)
        self.signal_table.update_selected()

    def update_table(self):
        self.data_table.update_items(self.data.data)

    def on_close_handler(self):
        self.data.stop_thread()


def start_GUI():
    app = QtWidgets.QApplication(sys.argv)  # Create an instance of QtWidgets.QApplication
    window = Ui()  # Create an instance of our class
    # Ensure threads are stopped on window close
    app.aboutToQuit.connect(window.on_close_handler)
    app.exec_()  # Start the application
