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
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QListWidget, QVBoxLayout, QTabWidget
from PyQt5.QtWidgets import QPushButton, QComboBox, QDateTimeEdit, QDialogButtonBox, QMessageBox
from PyQt5.QtWidgets import QScrollArea, QMainWindow, QCheckBox, QHBoxLayout, QTextEdit
from PyQt5.QtWidgets import QLineEdit, QFrame, QDialog, QFrame, QSplitter
from PyQt5.QtGui import QIcon, QBrush, QColor, QFont, QPixmap, QMovie
from PyQt5.QtCore import QDateTime, Qt


# ==============================================================================
# ADDITION DIALOG
# ==============================================================================

class AdditionDialog(QDialog):

	def __init__(self, parent, addition=None):
		super(AdditionDialog, self).__init__(parent)

		if not addition:
			addition = dict()
			addition["name"] = "fuck"
			addition["int value"] = 34
			addition["float value"] = 69.69
			addition["bool value"] = True
			addition["int value2"] = 34
			addition["float value2"] = 69.69
			addition["bool value2"] = True
			addition["sdf"] = "fuck"
			addition["dfdf"] = "fuck"
			addition["ffff"] = "fuck"
		self.addition = addition
		
		y = 10
		dy = 40
		self.widgets = dict()
		for field in self.addition:
			w = self.createLabelWidget(self, field, y, field in ["name"])
			self.widgets[field] = w
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
	def createLabelWidget(self, parent, field, y, noedit=False):
	
		FW = 180
		PW = 400
	
		q = QLabel(field)
		q.setParent(parent)
		q.setGeometry(10, y, FW-10, 36)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		q.show()

		value = self.addition[field]
		
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
		q.setParent(parent)
		q.setGeometry(FW, y, PW - FW - 10, 30)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		q.show()
		return q
		
	# text field changed
	def onTextFieldChanged(self, text, field):
		self.metadata[field] = text
			
	# checkbox changed
	def onCheckboxChanged(self, state, field):
		if state == 0:
			self.metadata[field] = False
		else:
			self.metadata[field] = True

			
	@staticmethod
	def newAddition(parent = None):
		dialog = AdditionDialog(parent)
		result = dialog.exec_()
