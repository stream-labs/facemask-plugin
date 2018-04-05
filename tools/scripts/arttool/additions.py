# ==============================================================================
# Copyright (C) 2017 General Workings Inc
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
# ==============================================================================


# ==============================================================================
# IMPORTS
# ==============================================================================
from copy import deepcopy
import os
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QListWidget, QVBoxLayout, QTabWidget
from PyQt5.QtWidgets import QPushButton, QComboBox, QDateTimeEdit, QDialogButtonBox, QMessageBox
from PyQt5.QtWidgets import QScrollArea, QMainWindow, QCheckBox, QHBoxLayout, QTextEdit
from PyQt5.QtWidgets import QLineEdit, QFrame, QDialog, QFrame, QSplitter, QFileDialog
from PyQt5.QtGui import QIcon, QBrush, QColor, QFont, QPixmap, QMovie
from PyQt5.QtCore import QDateTime, Qt

from arttool.utils import *

ADDITION_TYPES = ["Image", "Sequence", "Material", "Model", "Emitter", "Tweak"]

ADDITION_IMAGE = {"type": "image",
                  "name": "",
                  "file": ""}

ADDITION_SEQUENCE = {"type": "sequence",
                     "name": "",
                     "image": "",
                     "rows": 1,
                     "cols": 1,
                     "first": 0,
                     "last": 0,
                     "rate": 1.0,
                     "delay": 0.0,
                     "random-start": False,
                     "mode": "repeat"}

ADDITION_MATERIAL = {"type": "material",
                     "name": "",
                     "image": "texture,diffuse-0",
                     "culling": "back",
                     "depth-test": "less",
                     "depth-only": False,
                     "opaque": True}

ADDITION_MODEL = {"type": "model",
                  "name": "",
                  "mesh": "",
                  "material": ""}

ADDITION_EMITTER = {"type": "emitter",
                    "name": "",
                    "model": "",
                    "part": "",
                    "lifetime": 1.0,
                    "scale-start": 1.0,
                    "scale-end": 2.0,
                    "alpha-start": 1.0,
                    "alpha-end": 0.0,
                    "num-particles": 100,
                    "world-space": True,
                    "inverse-rate": False,
                    "z-sort-offset": 0.0,
                    "rate-min": 1.0,
                    "rate-max": 1.0,
                    "friction-min": 1.0,
                    "friction-max": 1.0,
                    "force-min": [0.0, 10.0, 0.0],
                    "force-max": [0.0, 10.0, 0.0],
                    "initial-velocity-min": [0.0, -40.0, 0.0],
                    "initial-velocity-max": [0.0, -40.0, 0.0]}

ADDITION_TWEAK = {"type": "tweak",
                  "name": "tweak",
                  "tweak1": "",
                  "tweak2": "",
                  "tweak3": "",
                  "tweak4": "",
                  "tweak5": "",
                  "tweak6": "",
                  "tweak7": "",
                  "tweak8": "",
                  "tweak9": "",
                  "tweak10": ""}

ADDITIONS = {"image": ADDITION_IMAGE,
             "sequence": ADDITION_SEQUENCE,
             "material": ADDITION_MATERIAL,
             "model": ADDITION_MODEL,
             "emitter": ADDITION_EMITTER,
             "tweak": ADDITION_TWEAK}

DROP_DOWNS = {("sequence", "mode"): ["once", "bounce", "repeat", "bounce-delay", "repeat-delay",
                                     "scroll-left", "scroll-right", "scroll-up", "scroll-down",
                                     "spin-cw", "spin-ccw"],
              ("material", "culling"): ["back", "front", "neither"],
              ("material", "depth-test"): ["less", "always", "less-equal", "equal", "greater-equal",
                                           "greater", "not-equal", "never"]}


def perform_image_addition(addition, jsonfile, outputWindow):
    kvp = dict()
    for i in ["name", "file"]:
        kvp[i] = addition[i]
    kvp["file"] = os.path.abspath(kvp["file"])
    for line in maskmaker("addres", kvp, [jsonfile]):
        outputWindow.append(line)


def perform_general_addition(addition, jsonfile, outputWindow):
    for line in maskmaker("addres", addition, [jsonfile]):
        outputWindow.append(line)


def perform_material_addition(addition, jsonfile, outputWindow):
    kvp = addition.copy()
    kvp["effect"] = "effectDefault"
    for line in maskmaker("addres", kvp, [jsonfile]):
        outputWindow.append(line)


def perform_emitter_addition(addition, jsonfile, outputWindow):
    kvp = addition.copy()
    for k, v in kvp.items():
        if type(v) is list:
            kvp[k] = str(v[0]) + "," + str(v[1]) + "," + str(v[2])

    if kvp["rate-min"] == kvp["rate-max"]:
        kvp["rate"] = kvp["rate-min"]
        del kvp["rate-min"]
        del kvp["rate-max"]
    if kvp["friction-min"] == kvp["friction-max"]:
        kvp["friction"] = kvp["friction-min"]
        del kvp["friction-min"]
        del kvp["friction-max"]
    if kvp["force-min"] == kvp["force-max"]:
        kvp["force"] = kvp["force-min"]
        del kvp["force-min"]
        del kvp["force-max"]
    if kvp["initial-velocity-min"] == kvp["initial-velocity-max"]:
        kvp["initial-velocity"] = kvp["initial-velocity-min"]
        del kvp["initial-velocity-min"]
        del kvp["initial-velocity-max"]

    for line in maskmaker("addres", kvp, [jsonfile]):
        outputWindow.append(line)


def perform_tweak_addition(addition, jsonfile, outputWindow):
    kvp = dict()
    for i in range(1, 11):
        k = "tweak" + str(i)
        if k in addition and len(addition[k]) > 0:
            bits = addition[k].split("=")
            if str_is_int(bits[1]):
                kvp[bits[0]] = int(bits[1])
            elif str_is_float(bits[1]):
                kvp[bits[0]] = float(bits[1])
            else:
                kvp[bits[0]] = bits[1]

    for line in maskmaker("tweak", kvp, [jsonfile]):
        outputWindow.append(line)


def perform_addition(addition, jsonfile, outputWindow):
    if addition["type"] == "image":
        perform_image_addition(addition, jsonfile, outputWindow)
    elif addition["type"] == "material":
        perform_material_addition(addition, jsonfile, outputWindow)
    elif addition["type"] == "emitter":
        perform_emitter_addition(addition, jsonfile, outputWindow)
    elif addition["type"] == "tweak":
        perform_tweak_addition(addition, jsonfile, outputWindow)
    elif addition["type"] in ["sequence", "model"]:
        perform_general_addition(addition, jsonfile, outputWindow)


# ==============================================================================
# NEW ADDITION DIALOG
# ==============================================================================

class NewAdditionDialog(QDialog):

    def __init__(self, parent):
        super(NewAdditionDialog, self).__init__(parent)

        self.combo = QComboBox(self)
        for s in ADDITION_TYPES:
            self.combo.addItem(s)
        self.combo.setCurrentIndex(0)
        self.combo.setGeometry(10, 10, 200, 30)

        # OK and Cancel buttons
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel, Qt.Horizontal, self)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        buttons.setParent(self)
        buttons.setGeometry(10, 40, 200, 30)

    @staticmethod
    def go_modal(parent=None):
        dialog = NewAdditionDialog(parent)
        result = dialog.exec_()
        if result == QDialog.Accepted:
            sel = ADDITION_TYPES[dialog.combo.currentIndex()]
            addn = ADDITIONS[sel.lower()]
            return AdditionDialog.go_modal(parent, deepcopy(addn))
        return None


# ==============================================================================
# ADDITION DIALOG
# ==============================================================================

FW = 180
PW = 400


class AdditionDialog(QDialog):

    def __init__(self, parent, addition):
        super(AdditionDialog, self).__init__(parent)

        self.addition = addition
        self.widgets = dict()

        numitems = len(self.addition)

        x = 10
        y = 10
        dy = 40
        count = 0
        for field in self.addition:
            w = self.createLabelWidget(self, field, x, y, field in ["type"])
            self.widgets[field] = w
            y += dy
            count += 1
            if numitems > 10 and count >= (numitems / 2):
                y = 10
                x += PW + 10
                numitems = 0

        y += dy

        # OK and Cancel buttons
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel, Qt.Horizontal, self)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        buttons.setParent(self)
        buttons.setGeometry(10, y, 500, 30)

    # --------------------------------------------------
    # Create generic widget
    # --------------------------------------------------
    def createLabelWidget(self, parent, field, x, y, noedit=False):

        q = QLabel(field)
        q.setParent(parent)
        q.setGeometry(x, y, FW - 10, 36)
        q.setFont(QFont("Arial", 12, QFont.Bold))
        q.show()

        add_type = self.addition["type"]
        value = self.addition[field]

        if (add_type, field) in DROP_DOWNS:
            dropvals = DROP_DOWNS[(add_type, field)]
            q = QComboBox()
            idx = 0
            selidx = -1
            for s in dropvals:
                if s == str(value):
                    selidx = idx
                q.addItem(s)
                idx += 1
            if selidx < 0:
                selidx = 0
            q.setCurrentIndex(selidx)
            q.currentIndexChanged.connect(lambda state: self.onDropdownChanged(state, field))
        elif type(value) is str or type(value) is int or type(value) is float:
            if noedit:
                q = QLabel(str(value))
            else:
                q = QLineEdit(str(value))
                q.textChanged.connect(lambda text: self.onTextFieldChanged(text, field))
        elif type(value) is bool:
            q = QCheckBox()
            q.setChecked(value)
            q.stateChanged.connect(lambda state: self.onCheckboxChanged(state, field))
        elif type(value) is list:
            xx = x + FW
            idx = 0
            for val in value:
                q = QLineEdit(str(val))
                q.setParent(parent)
                q.setGeometry(xx, y, 75, 30)
                q.setFont(QFont("Arial", 12, QFont.Bold))
                q.show()
                xx += 80
                q.textChanged.connect(lambda text, idx=idx: self.onTextFieldChanged(text, field, idx))
                idx += 1

        if type(value) is not list:
            q.setParent(parent)
            if field == "file":
                q.setGeometry(x + FW, y, PW - FW - 60, 30)
                b = QPushButton("BROWSE")
                b.setParent(parent)
                b.setGeometry(x + PW - 60, y, 50, 30)
                b.pressed.connect(lambda: self.onFileBrowser(field))

            else:
                q.setGeometry(x + FW, y, PW - FW - 10, 30)
            q.show()
            q.setFont(QFont("Arial", 12, QFont.Bold))
        return q


    # file browser button
    def onFileBrowser(self, field):
        fname,filter = QFileDialog.getOpenFileName(self, 'Open file', os.path.abspath("."),
                                                   "Image files (*.png *.gif *.jpg *.tiff *.tga)")
        fname = make_path_relative(os.path.abspath('.'), fname)
        self.addition[field] = fname
        self.widgets[field].setText(fname)

    def onDropdownChanged(self, state, field):
        dd = (self.addition["type"],field)
        if type(self.addition[field]) is int:
            self.addition[field] = int(DROP_DOWNS[dd][state])
        elif type(self.addition[field]) is int:
            self.addition[field] = float(DROP_DOWNS[dd][state])
        else:
            self.addition[field] = DROP_DOWNS[dd][state]


    # text field changed
    def onTextFieldChanged(self, text, field, index=0):
        v = self.addition[field]

        if type(v) is str:
            self.addition[field] = text
        elif type(v) is int:
            try:
                self.addition[field] = int(text)
            except:
                self.addition[field] = 0
        elif type(v) is float:
            try:
                self.addition[field] = float(text)
            except:
                self.addition[field] = 0.0
        elif type(v) is list:
            # all vectors are floats
            try:
                self.addition[field][index] = float(text)
            except:
                self.addition[field][index] = 0.0

    # checkbox changed
    def onCheckboxChanged(self, state, field):
        if state == 0:
            self.addition[field] = False
        else:
            self.addition[field] = True

    @staticmethod
    def go_modal(parent, addition):
        dialog = AdditionDialog(parent, addition)
        result = dialog.exec_()
        if result == QDialog.Accepted:
            return dialog.addition
        return None
