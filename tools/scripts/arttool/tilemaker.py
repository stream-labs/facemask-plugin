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
import sys, os
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QListWidget, QVBoxLayout, QTabWidget
from PyQt5.QtWidgets import QPushButton, QComboBox, QDateTimeEdit, QDialogButtonBox, QMessageBox
from PyQt5.QtWidgets import QScrollArea, QMainWindow, QCheckBox, QHBoxLayout, QTextEdit
from PyQt5.QtWidgets import QLineEdit, QFrame, QDialog, QFrame, QSplitter
from PyQt5.QtGui import QIcon, QBrush, QColor, QFont, QPixmap, QMovie
from PyQt5.QtCore import QDateTime, Qt
from arttool.utils import *
from PIL import Image

FW = 180
PW = 600


# ==============================================================================
# MAIN WINDOW : TileMakerWindow class
# ==============================================================================

class TileMakerWindow(QMainWindow):

    # --------------------------------------------------
    # CONSTRUCTOR
    # --------------------------------------------------
    def __init__(self, *args):
        super(TileMakerWindow, self).__init__(*args)

        # main widget area
        mainWidget = QWidget()

        # set up our fields
        self.tiledata = {"src": "C:\\STREAMLABS\\slart\\effects_generic\\explosion\\Xplosion%02d.png",
                         "dst": "C:\\Temp\\output.png",
                         "startFrame": 1,
                         "endFrame": 60,
                         "width": 256,
                         "height": 256,
                         "rows": 8,
                         "cols": 8}

        x = 10
        y = 10
        for field in self.tiledata:
            self.createLabelWidget(mainWidget, field, x, y)
            y += 36

        b = QPushButton("DO IT")
        b.pressed.connect(lambda: self.onDoIt())
        b.setParent(mainWidget)
        b.setGeometry(200, y + 20, 200, 30)
        b.setFocus()

        # Show the window
        self.setCentralWidget(mainWidget)
        self.setWindowTitle('Tile Maker Tool')
        self.setGeometry(50, 50, 640, 400)
        self.setWindowIcon(QIcon('arttool/tilemaker.png'))

    # called before exit
    def finalCleanup(self):
        print("do final cleanup")

    def onDoIt(self):

        # get values
        src = self.tiledata["src"]
        dst = self.tiledata["dst"]
        startFrame = self.tiledata["startFrame"]
        endFrame = self.tiledata["endFrame"]
        w = self.tiledata["width"]
        h = self.tiledata["height"]
        size = w, h
        rows = self.tiledata["rows"]
        cols = self.tiledata["cols"]

        # make new image for the tile sheet
        sheetw = w * cols
        sheeth = h * rows
        print("sheet size", sheetw, sheeth)
        sheet = Image.new("RGBA", (sheetw, sheeth))

        # paste images into it
        frame = startFrame
        for row in range(0, rows):
            if frame > endFrame:
                break
            for col in range(0, cols):
                if frame > endFrame:
                    break
                fname = (src % frame)
                if os.path.exists(fname):
                    print("loading", fname)
                    tile = Image.open(src % frame)
                    tile = tile.resize(size, Image.LANCZOS)
                    x = w * col
                    y = h * row
                    sheet.paste(tile, (x, y))
                    frame += 1
                else:
                    print(fname, "does not exist!")

        # write it out
        sheet.save(dst)

    # --------------------------------------------------
    # Create generic widget
    # --------------------------------------------------
    def createLabelWidget(self, parent, field, x, y, noedit=False):

        q = QLabel(field)
        q.setParent(parent)
        q.setGeometry(x, y, FW - 10, 36)
        q.setFont(QFont("Arial", 12, QFont.Bold))
        q.show()

        value = self.tiledata[field]

        if type(value) is str or type(value) is int or type(value) is float:
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
            q.setGeometry(x + FW, y, PW - FW - 10, 30)
            q.show()
            q.setFont(QFont("Arial", 12, QFont.Bold))

    # text field changed
    def onTextFieldChanged(self, text, field, index=0):
        v = self.tiledata[field]

        if type(v) is str:
            self.tiledata[field] = text
        elif type(v) is int:
            try:
                self.tiledata[field] = int(text)
            except:
                self.tiledata[field] = 0
        elif type(v) is float:
            try:
                self.tiledata[field] = float(text)
            except:
                self.tiledata[field] = 0.0
        elif type(v) is list:
            # all vectors are floats
            try:
                self.tiledata[field][index] = float(text)
            except:
                self.tiledata[field][index] = 0.0

    # checkbox changed
    def onCheckboxChanged(self, state, field):
        if state == 0:
            self.tiledata[field] = False
        else:
            self.tiledata[field] = True
