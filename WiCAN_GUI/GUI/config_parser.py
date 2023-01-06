import json
from pathlib import Path
from PyQt5 import QtCore, QtWidgets

# Read Json file and set settings


def get_dbc_path():
    dbc_path = ""
    dlg = QtWidgets.QFileDialog()
    dlg.setFileMode(QtWidgets.QFileDialog.AnyFile)
    dlg.setNameFilter("DBC Files (*.DBC)")

    if dlg.exec_():
        filenames = dlg.selectedFiles()
        dbc_path = Path(filenames[0])
        print(f"DBC Path: {dbc_path}")

    return str(dbc_path)
