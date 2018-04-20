1>2# : ^
'''
@echo off
rem https://stackoverflow.com/questions/17467441/how-to-embed-python-code-in-batch-script
%userprofile%\\AppData\\Local\\Programs\\Python\\Python36-32\\python "%~f0"
exit /b
rem ^
'''


import sys, subprocess,os
from PyQt5.QtWidgets import QApplication, QFileDialog, QFrame, QWidget, QVBoxLayout, QPushButton, QScrollArea, QMainWindow, QCheckBox, QHBoxLayout, QTextEdit, QLineEdit, QLabel
from PyQt5.QtGui import QIcon

MERGEBATFILE = os.path.join(os.path.dirname(os.path.abspath(__file__)),"maskmaker", "maskmerge.bat")


# Executes a shell command
def execute(cmd):
	popen = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True)
	for stdout_line in iter(popen.stdout.readline, ""):
		yield stdout_line 
	popen.stdout.close()
	return_code = popen.wait()
	if return_code:
		raise subprocess.CalledProcessError(return_code, cmd)
		

		
def onBrowse(sender):
	print("browsing for " + sender.my_title)
	ourpath = os.path.dirname(os.path.abspath(__file__)) + "/"
	fname = QFileDialog.getOpenFileName(sender.my_parent, 'Open file', ourpath, "Face Masks (*.json)")
	if fname[0]:
		ff = fname[0][len(ourpath):].replace("/","\\")
		sender.my_lineedit.setText(ff)
		sender.my_checkbox.setChecked(True)

def onBrowseOutput(sender):
	print("browsing for " + sender.my_title)
	ourpath = os.path.dirname(os.path.abspath(__file__)) + "/"
	fname = QFileDialog.getExistingDirectory (sender.my_parent, 'Output Folder', ourpath)	
	if fname:
		sender.my_lineedit.setText(fname.replace("/","\\"))
		
FileWidgetButtons = dict()
		
def createFileWidget(parent, title):
	global FileWidgetButtons

	r = QWidget(parent)
	w = QHBoxLayout(r)
	lbl = None
	isOutput = (title == "Output Folder")
		
	if isOutput:
		lbl = QLabel(title)
	else:
		lbl = QCheckBox(title)
	w.addWidget(lbl)
	le = QLineEdit()
	le.setGeometry(0,0, 400, 30)
	if isOutput:
		ourpath = os.path.dirname(os.path.abspath(__file__))
		le.setText(os.path.join(ourpath, "combos"))
		#le.setText("C:\\Users\\Ross\\Desktop\\work")
	w.addWidget(le)
	b = QPushButton("Browse")
	b.my_title = title
	b.my_lineedit = le
	b.my_parent = parent
	b.my_checkbox = lbl
	if isOutput:
		b.clicked.connect(lambda: onBrowseOutput(b))	
	else:
		b.clicked.connect(lambda: onBrowse(b))
	b.setGeometry(0,0,75,30)
	FileWidgetButtons[title] = b
	w.addWidget(b)
	return r
		
		
def horizLine():
	line = QFrame()
	line.setGeometry(0,0,500,3)
	line.setFrameShape(QFrame.HLine)
	line.setFrameShadow(QFrame.Sunken)
	return line
		
		
class ArtToolWindow(QMainWindow): 

	def __init__(self, *args): 
		QMainWindow.__init__(self, *args) 

		mainPane = QWidget()
		
		ww = QWidget(mainPane)
		ww.setGeometry(0,0,500,300)
		w = QVBoxLayout(ww)
		
		w.addWidget(createFileWidget(mainPane, "File 1"))
		w.addWidget(createFileWidget(mainPane, "File 2"))
		w.addWidget(createFileWidget(mainPane, "File 3"))
		w.addWidget(createFileWidget(mainPane, "File 4"))
		w.addWidget(createFileWidget(mainPane, "File 5"))
		w.addWidget(createFileWidget(mainPane, "File 6"))
		
		www = QWidget(mainPane)
		www.setGeometry(0,300,500,160)
		x = QVBoxLayout(www)
		
		x.addWidget(horizLine())
		x.addWidget(createFileWidget(mainPane, "Output Folder"))
		x.addWidget(horizLine())
		
		bm = QPushButton("MERGE")
		bm.clicked.connect(lambda: self.onMerge())
		x.addWidget(bm)

		qbm = QPushButton("QUIT")
		qbm.clicked.connect(lambda: QApplication.quit())
		x.addWidget(qbm)

		# Show the window
		self.setCentralWidget(mainPane)
		self.setGeometry(100,100, 500, 470)
		self.setWindowTitle('Mask Merge Tool')
		self.setWindowIcon(QIcon('arttool/mergeicon.png'))

		
	def onMerge(self):
		global FileWidgetButtons
		ourpath = os.path.dirname(os.path.abspath(__file__))
		mmkr = os.path.join(ourpath, "maskmaker", "MaskMaker.exe")		
		outfilename = ""
		cmd = [mmkr, "merge"]
		for f in "File 1.File 2.File 3.File 4.File 5.File 6".split("."):
			if FileWidgetButtons[f].my_checkbox.isChecked():
				ff = os.path.join(ourpath, FileWidgetButtons[f].my_lineedit.text())
				cmd.append(ff)
				if len(outfilename) > 0:
					outfilename += "+"
				outfilename += os.path.basename(ff)[:-5]
		
		ffout = os.path.join(FileWidgetButtons["Output Folder"].my_lineedit.text(), outfilename + ".json")
		cmd.append(ffout)
		print(cmd)
		for line in execute(cmd):
			print(line)
		

if __name__ == '__main__':
	
	# We're a Qt App
	app = QApplication(sys.argv)
		
	# Show the window
	w = ArtToolWindow()
	w.show()

	# Exit properly
	sys.exit(app.exec_())
	
	
