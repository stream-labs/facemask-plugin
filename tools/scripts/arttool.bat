1>2# : ^
'''
@echo off
rem https://stackoverflow.com/questions/17467441/how-to-embed-python-code-in-batch-script
%userprofile%\\AppData\\Local\\Programs\\Python\\Python36-32\\python "%~f0"
exit /b
rem ^
'''


import sys, subprocess,os
from PyQt5.QtWidgets import QApplication, QWidget, QVBoxLayout, QPushButton, QScrollArea, QMainWindow, QCheckBox, QHBoxLayout, QTextEdit
from PyQt5.QtGui import QIcon


MAKEARTBATFILE = os.path.join(os.path.dirname(os.path.abspath(__file__)),"makeart.bat")



# Executes a shell command
def execute(cmd):
	popen = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True)
	for stdout_line in iter(popen.stdout.readline, ""):
		yield stdout_line 
	popen.stdout.close()
	return_code = popen.wait()
	if return_code:
		raise subprocess.CalledProcessError(return_code, cmd)
		

		
class ArtToolWindow(QMainWindow): 

	def __init__(self, *args): 
		QMainWindow.__init__(self, *args) 
 
		self.checkBoxes = dict()

		# Set up a vertical layout that scrolls
		central = QWidget()
		scroll = QScrollArea()
		layout = QVBoxLayout(central)
		scroll.setWidget(central)
		scroll.setWidgetResizable(True)
		
		# Add widgets to the layout
		for t in tags:
			b = QCheckBox(t)
			self.checkBoxes[t] = b
			layout.addWidget(b)

		# buttons
		rightPane = QWidget()
		buttonsArea = QVBoxLayout(rightPane)
		# Select All
		b = QPushButton("Select All")
		b.clicked.connect(lambda: self.onSelectAll())
		buttonsArea.addWidget(b)
		# Select None
		b = QPushButton("Select None")
		b.clicked.connect(lambda: self.onSelectNone())
		buttonsArea.addWidget(b)
		# Padding
		pad = QWidget()
		pad.setGeometry(0,0,200,150)
		buttonsArea.addWidget(pad)
		# Build Art
		b = QPushButton("BUILD ART")
		b.clicked.connect(lambda: self.onBuildArt())
		buttonsArea.addWidget(b)
		# Padding
		pad = QWidget()
		pad.setGeometry(0,0,200,150)
		buttonsArea.addWidget(pad)
		# Quit
		q = QPushButton("Quit")
		q.clicked.connect(lambda: QApplication.quit())
		buttonsArea.addWidget(q)

		# Main area
		mainPane = QWidget()
		mainArea = QVBoxLayout(mainPane)
		
		# Top area
		topPane = QWidget()
		topArea = QHBoxLayout(topPane)
		
		# Add Stuff to top area
		topArea.addWidget(scroll)
		topArea.addWidget(rightPane)
			
		# Add Stuff to main area
		mainArea.addWidget(topPane)
		self.outputPane = QTextEdit()
		mainArea.addWidget(self.outputPane)
			
		# Show the window
		self.setCentralWidget(mainPane)
		self.setGeometry(100,100, 500, 520)
		self.setWindowTitle('Streamlabs Art Tool')
		self.setWindowIcon(QIcon('icons/arttoolicon.png'))
				

	def onSelectAll(self):
		for t,b in self.checkBoxes.items():
			b.setChecked(True)

	def onSelectNone(self):
		for t,b in self.checkBoxes.items():
			b.setChecked(False)

	def onBuildArt(self):
		for t,b in self.checkBoxes.items():
			if b.isChecked():
				self.outputPane.append("")
				self.outputPane.append("----- BUILDING " + t + " -----")
				QApplication.processEvents()
				for line in execute([MAKEARTBATFILE, t]):
					self.outputPane.append(line.replace("\n",""))
					QApplication.processEvents()
				self.outputPane.append("----- DONE BUILDING " + t + " -----")
				QApplication.processEvents()


if __name__ == '__main__':
	
	# load the makeart.bat file
	f = open(MAKEARTBATFILE,"r")
	makeartContents = f.read()
	f.close()
	
	# get list of labels (tags)
	lines = makeartContents.replace("\r","").split("\n")
	tags = list()
	for line in lines:
		if len(line) > 0:
			if line[0] == ":":
				tags.append(line[1:])
	
	# We're a Qt App
	app = QApplication(sys.argv)
		
	# Show the window
	w = ArtToolWindow()
	w.show()

	# Exit properly
	sys.exit(app.exec_())
	
	
