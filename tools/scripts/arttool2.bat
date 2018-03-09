1>2# : ^
'''
@echo off
rem https://stackoverflow.com/questions/17467441/how-to-embed-python-code-in-batch-script
if exist %userprofile%\\AppData\\Local\\Programs\\Python\\Python36\\python.exe goto PYTHON64
if exist %userprofile%\\AppData\\Local\\Programs\\Python\\Python36-32\\python.exe goto PYTHON32
echo "I can't find python (3.6). Please install python from https://www.python.org/downloads/windows/"
pause
exit /b
:PYTHON32
%userprofile%\\AppData\\Local\\Programs\\Python\\Python36-32\\python "%~f0" 
exit /b
:PYTHON64
%userprofile%\\AppData\\Local\\Programs\\Python\\Python36\\python "%~f0" 
exit /b
rem ^
'''

# ==============================================================================
# IMPORTS
# ==============================================================================
import sys, subprocess, os, json, uuid
try:
	from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QListWidget, QVBoxLayout
	from PyQt5.QtWidgets import QPushButton, QComboBox, QDateTimeEdit, QDialogButtonBox, QMessageBox
	from PyQt5.QtWidgets import QScrollArea, QMainWindow, QCheckBox, QHBoxLayout, QTextEdit
	from PyQt5.QtWidgets import QLineEdit, QFrame, QDialog, QFrame, QSplitter
	from PyQt5.QtGui import QIcon, QBrush, QColor, QFont, QPixmap, QMovie
	from PyQt5.QtCore import QDateTime, Qt
except:
	print("You don't seem to have PyQt5 installed. I will attempt to install it now.")
	cmd = os.path.join(os.path.expanduser("~"),"AppData","Local","Programs","Python","Python36","python.exe")
	if not os.path.exists(cmd):
		cmd = os.path.join(os.path.expanduser("~"),"AppData","Local","Programs","Python","Python36-32","python.exe")
	if not os.path.exists(cmd):
		print("I can't find python. Really odd, since this is python code.")
	cmd += " -m pip install PyQt5 --user"
	popen = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True, shell=True)
	for stdout_line in iter(popen.stdout.readline, ""):
		print(stdout_line)
	popen.wait()
	print("PyQt5 should be installed now. Try running art tool again.")
	sys.exit(0)
	
# ==============================================================================
# FILE LOCATIONS
# ==============================================================================
SVNBIN = os.path.abspath(os.path.join("c:\\",'"Program Files"',"TortoiseSVN","bin","svn.exe"))
MASKMAKERBIN = os.path.abspath("./maskmaker/maskmaker.exe")
MORPHRESTFILE = os.path.abspath("./morphs/morph_rest.fbx")



# ==============================================================================
# FIELD SEVERITIES
# ==============================================================================
def critical(field):
	return field in ["name", "author", "license"]

def desired(field):
	return field in ["website"]


# ==============================================================================
# SYSTEM STUFF
# ==============================================================================

# Executes a shell command
# usage:
# 
# for line in execute(cmd):
#   print(line)
#
def execute(cmd):
	popen = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True, shell=True)
	for stdout_line in iter(popen.stdout.readline, ""):
		yield stdout_line 
	popen.stdout.close()
	return_code = popen.wait()
	if return_code:
		# raise subprocess.CalledProcessError(return_code, cmd)
		yield "ERROR " + cmd.split()[0] + " FAILED EXECUTION."

# Gets a list of files
def getFileList(folder):

	fileList = list()
	for root,subdir,files in os.walk(folder):
		for file in files:
			if file.lower().endswith(".fbx"):
				fileList.append(os.path.join(root,file).replace("\\","/"))

	return fileList

def jsonFromFbx(fbxfile):
	return os.path.abspath(fbxfile).replace(".fbx",".json").replace(".FBX",".json")
	
# ==============================================================================
# SVN
# ==============================================================================
	
def svnFileMissing():
	cmd = SVNBIN + ' status -uq'
	for line in execute(cmd):
		if line.split()[0] in ["!"]:
			return True
	return False
	
def svnNeedsUpdate():	
	# have
	cmd = SVNBIN + 'info | grep -i "Last Changed Rev"'
	revHave = 0
	for line in execute(cmd):
		revHave = int(line.split()[-1])
		
	# head
	cmd = SVNBIN + ' info -r HEAD | grep -i "Last Changed Rev"'
	revHead = 0
	for line in execute(cmd):
		revHead = int(line.split()[-1])
		
	return (revHead > revHave) or svnFileMissing()

def svnNeedsCommit():
	cmd = SVNBIN + ' status -uq'
	for line in execute(cmd):
		if line.split()[0] in ["A", "M", "D"]:
			return True
	return False

def svnGetFileStatus(filename):
	cmd = SVNBIN + ' status ' + os.path.abspath(filename)
	for line in execute(cmd):
		return line.split()[0]
	return ""
	
def svnIsFileNew(filename):
	return svnGetFileStatus(filename) == "?"
	
def svnAddFile(filename):
	if svnIsFileNew(filename):
		cmd = SVNBIN + " add " + os.path.abspath(filename)
		for line in execute(cmd):
			pass

# ==============================================================================
# MASKMAKER
# ==============================================================================
def maskmaker(command, kvpairs, files):
	cmd = MASKMAKERBIN + " " + command 
	for k,v in kvpairs.items():
		if type(v) is str:
			cmd += " " + k + '="' + v + '"'
		else:
			cmd += " " + k + '=' + str(v) 
	
	for f in files:
		cmd += " " + f
	
	print("----------")
	print(cmd)
	for line in execute(cmd):
		yield line[:-1]
		
def mmImport(fbxfile, metadata):
	CREATEKEYS = ["name", "uuid", "description", "author", "tags", "category", "license", "website"]
	d = dict()
	for k in CREATEKEYS:
		d[k] = metadata[k]
		
	jsonfile = jsonFromFbx(fbxfile)
	if metadata["is_morph"]:
		d["restfile"] = MORPHRESTFILE
		d["posefile"] = os.path.abspath(fbxfile)
		for line in maskmaker("morphimport", d, [jsonfile]):
			yield line
	else:
		d["file"] = os.path.abspath(fbxfile)
		for line in maskmaker("import", d, [jsonfile]):
			yield line
	
def mmDepends(fbxfile):
	cmd = MASKMAKERBIN + " depends " + '"' + os.path.abspath(fbxfile) + '"'
	deps = list()
	for line in execute(cmd):
		deps.append(line[:-1])
	return deps
	
			
# ==============================================================================
# META DATA
# ==============================================================================
def getMetaFolderName(fbxfile):
	dn = os.path.dirname(fbxfile)
	dn = os.path.join(dn, ".art").replace("\\","/")
	return dn
	
def createMetaFolder(folder):
	if not os.path.exists(folder):
		os.makedirs(folder)
		svnAddFile(folder)

def getMetaFileName(fbxfile):
	metafolder = getMetaFolderName(fbxfile)
	metafile = os.path.join(metafolder, os.path.basename(fbxfile).lower().replace(".fbx",".meta")).replace("\\","/")
	return metafile
			
def writeMetaData(metafile, metadata):
	f = open(metafile,"w")
	f.write(json.dumps(metadata, indent=4))
	f.close()
	svnAddFile(metafile)

def createGetMetaData(fbxfile):
	metafolder = getMetaFolderName(fbxfile)
	createMetaFolder(metafolder)
	metafile = getMetaFileName(fbxfile)
	metadata = dict()
	if os.path.exists(metafile):
		# read existing metadata
		f = open(metafile,"r")
		metadata = json.loads(f.read())
		f.close()
	else:
		# create new metadata
		metadata["fbx"] = fbxfile
		metadata["name"] = ""
		metadata["description"] = ""
		metadata["author"] = ""
		metadata["tags"] = ""
		metadata["category"] = ""
		metadata["fbx_modtime"] = os.path.getmtime(fbxfile)
		metadata["uuid"] = str(uuid.uuid4())
		metadata["depth_head"] = False
		metadata["is_morph"] = False
		metadata["is_vip"] = False		
		metadata["texture_max"] = 256
		metadata["do_not_release"] = False
		metadata["license"] = "Copyright 2017 - General Working Inc. - All rights reserved."		
		metadata["website"] = "http://streamlabs.com"
		# write it
		writeMetaData(metafile, metadata)
		
	return metadata
	
# checks status of meta data 
#
CHECKMETA_GOOD = 0
CHECKMETA_ERROR = 1
CHECKMETA_WARNING = 2
CHECKMETA_NORELEASE = 3
MASK_UNKNOWN = -1
MASK_NORMAL = 0
MASK_MORPH = 1
def checkMetaData(metadata):
	masktype = MASK_UNKNOWN

	# is morph
	if metadata["is_morph"]:
		masktype = MASK_MORPH
	else:
		masktype = MASK_NORMAL
	
	# not for release
	if metadata["do_not_release"]:
		return CHECKMETA_NORELEASE,masktype
	
	# check for errors
	for field,value in metadata.items():
		if type(value) is str and critical(field) and len(value) == 0:
			return CHECKMETA_ERROR,masktype
		elif type(value) is str and desired(field) and len(value) == 0:
			return CHECKMETA_WARNING,masktype
			
	# check for icons
	fbxfile = metadata["fbx"]
	pf = os.path.abspath(fbxfile.lower().replace(".fbx",".gif"))
	if not os.path.exists(pf):
		return CHECKMETA_WARNING,masktype
	
	# good
	return CHECKMETA_GOOD,masktype
	
def checkMetaDataFile(fbxfile):
	masktype = MASK_UNKNOWN
	metafile = getMetaFileName(fbxfile)
	if not os.path.exists(metafile):
		return CHECKMETA_ERROR,masktype
		
	# read existing metadata
	metadata = dict()
	f = open(metafile,"r")
	metadata = json.loads(f.read())
	f.close()

	# check it
	return checkMetaData(metadata)
	
# ==============================================================================
# CONFIG
# ==============================================================================

def createGetConfig():
	createMetaFolder("./.art")
	metafile = "./.art/config.meta"
	config = dict()
	if os.path.exists(metafile):
		f = open(metafile,"r")
		config = json.loads(f.read())
		f.close()
	else:
		config["x"] = 50
		config["y"] = 50
		writeMetaData(metafile, config)
	return config
	

# ==============================================================================
# TEST DIALOG
# ==============================================================================

class DateTimeDialog(QDialog):

	def __init__(self, parent = None):
		super(DateTimeDialog, self).__init__(parent)

		layout = QVBoxLayout(self)

		# nice widget for editing the date
		self.datetime = QDateTimeEdit(self)
		self.datetime.setCalendarPopup(True)
		self.datetime.setDateTime(QDateTime.currentDateTime())
		layout.addWidget(self.datetime)

		# OK and Cancel buttons
		buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel, Qt.Horizontal, self)
		buttons.accepted.connect(self.accept)
		buttons.rejected.connect(self.reject)
		layout.addWidget(buttons)

	# get current date and time from the dialog
	def dateTime(self):
		return self.datetime.dateTime()

	# static method to create the dialog and return (date, time, accepted)
	@staticmethod
	def getDateTime(parent = None):
		dialog = DateTimeDialog(parent)
		result = dialog.exec_()
		date = dialog.dateTime()
		return (date.date(), date.time(), result == QDialog.Accepted)


# ==============================================================================
# MAIN WINDOW : ArtToolWindow class
# ==============================================================================
	
FIELD_WIDTH = 150
PANE_WIDTH = 690
TEXTURE_SIZES = ["32","64","128","256","512","1024","2048"]
MASK_UI_FIELDS = { "name" : "Pretty Name",
				   "description" : "Description",
				   "author" : "Author",
				   "tags" : "Tags",
				   "category" : "Category",
				   "uuid" : "UUID",
				   "depth_head" : "Depth Head",
				   "is_morph" : "Morph Mask",
				   "is_vip" : "V.I.P. Mask",
				   "do_not_release" : "DO NOT RELEASE",
				   "texture_max" : "Max Texture Size",
				   "license" : "License",
				   "website" : "Website" }

	
class ArtToolWindow(QMainWindow): 

	# --------------------------------------------------
	# CONSTRUCTOR
	# --------------------------------------------------
	def __init__(self, *args): 
		super(ArtToolWindow, self).__init__(*args) 
 
		# Load our config
		self.config = createGetConfig()
 
		# Get list of fbx files
		self.fbxfiles = getFileList(".")
		
		# Left Pane
		leftPane = QWidget()
		leftLayout = QVBoxLayout(leftPane)

		# Streamlabs logo
		slabslogo = QLabel()
		slabslogo.setPixmap(QPixmap("icons/streamlabs.png"))
		slabslogo.setScaledContents(True)
		slabslogo.show()
		leftLayout.addWidget(slabslogo)
		slabslogo.setMaximumWidth(300)
		slabslogo.setMaximumHeight(53)
		
		# Filter box
		self.fbxfilter = QLineEdit()
		leftLayout.addWidget(self.fbxfilter)
		
		# make a list widget
		self.fbxlist = QListWidget()
		for fbx in self.fbxfiles:
			self.fbxlist.addItem(fbx[2:])
		self.fbxlist.itemClicked.connect(lambda: self.onFbxClicked())
		self.fbxlist.setParent(leftPane)
		self.fbxlist.setMinimumHeight(560)
		leftLayout.addWidget(self.fbxlist)

		# top splitter
		topSplitter = QSplitter(Qt.Horizontal)
		topSplitter.addWidget(leftPane)

		# Layout for edit pane
		rightPane = QWidget()
		self.mainLayout = QHBoxLayout(rightPane)
		topSplitter.addWidget(rightPane)
		
		# Edit pane 
		self.editPane = None
		self.createMaskEditPane(None)
		
		# color fbxlist items
		for idx in range(0, len(self.fbxfiles)):
			self.setFbxColorIcon(idx)
			self.fbxlist.item(idx).setFont(QFont( "Arial", 12, QFont.Bold ))
			
		# crate main splitter
		mainSplitter = QSplitter(Qt.Vertical)
		mainSplitter.addWidget(topSplitter)
		
		# bottom pane
		bottomPane = QWidget()
		bottomArea = QHBoxLayout(bottomPane)
		
		# output window
		self.outputWindow = QTextEdit()
		self.outputWindow.setMinimumHeight(90)
		bottomArea.addWidget(self.outputWindow)

		# buttons area
		buttonArea = QWidget()
		buttonArea.setMinimumWidth(150)
		buttonArea.setMinimumHeight(60)
		b = QPushButton("Refresh")
		b.setParent(buttonArea)
		b.setGeometry(0,0,75,30)
		b = QPushButton("Autobuild")
		b.setParent(buttonArea)
		b.setGeometry(0,30,75,30)
		b = QPushButton("Rebuild All")
		b.setParent(buttonArea)
		b.setGeometry(0,60,75,30)
		b = QPushButton("Make Release")
		b.setParent(buttonArea)
		b.setGeometry(75,0,75,30)
		b = QPushButton("SVN Update")
		b.setParent(buttonArea)
		b.setGeometry(75,30,75,30)
		b = QPushButton("SVN Commit")
		b.setParent(buttonArea)
		b.setGeometry(75,60,75,30)

		bottomArea.addWidget(buttonArea)
		
		mainSplitter.addWidget(bottomPane)
			
		# Show the window
		self.setCentralWidget(mainSplitter)
		self.setGeometry(self.config["x"], self.config["y"], 1024, 768)
		self.setWindowTitle('Streamlabs Art Tool')
		self.setWindowIcon(QIcon('icons/arttoolicon.png'))
		
		# State
		self.metadata = None 
		self.currentFbx = -1
		
		# Check our binaries
		self.checkBinaries()

	def checkBinaries(self):
		gotSVN = os.path.exists(SVNBIN.replace('"',''))
		gotMM = os.path.exists(MASKMAKERBIN)
		gotRP = os.path.exists(MORPHRESTFILE)
		
		if not gotSVN:
			msg = QMessageBox()
			msg.setIcon(QMessageBox.Information)
			msg.setText("You seem to be missing " + os.path.basename(SVNBIN))
			msg.setInformativeText("You should (re)install tortoiseSVN, and be sure to install the command line tools.")
			msg.setWindowTitle("Missing Binary File")
			msg.setStandardButtons(QMessageBox.Ok)
			msg.exec_()
		if not gotMM:
			msg = QMessageBox()
			msg.setIcon(QMessageBox.Information)
			msg.setText("You seem to be missing " + os.path.basename(MASKMAKERBIN))
			msg.setWindowTitle("Missing Binary File")
			msg.setStandardButtons(QMessageBox.Ok)
			msg.exec_()
		if not gotRP:
			msg = QMessageBox()
			msg.setIcon(QMessageBox.Information)
			msg.setText("You seem to be missing " + os.path.basename(MORPHRESTFILE))
			msg.setWindowTitle("Missing Binary File")
			msg.setStandardButtons(QMessageBox.Ok)
			msg.exec_()
			
	# --------------------------------------------------
	# Create generic widget
	# --------------------------------------------------
	def createLabelWidget(self, parent, name, field, y, noedit=False):
		q = QLabel(name)
		q.setParent(parent)
		q.setGeometry(0, y, FIELD_WIDTH, 36)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		q.show()

		value = self.metadata[field]
		
		if type(value) is str:
			if noedit:
				q = QLabel(value)
			else:
				q = QLineEdit(value)
				q.textChanged.connect(lambda text: self.onTextFieldChanged(text, field))
			if critical(field) and len(value) == 0:
				q.setStyleSheet("border: 1px solid #FF0000;")
			elif desired(field) and len(value) == 0:
				q.setStyleSheet("border: 1px solid #FF7F50;")
			else:
				q.setStyleSheet("border: 0px;")
		elif type(value) is bool:
			q = QCheckBox()
			q.setChecked(value)
			q.stateChanged.connect(lambda state: self.onCheckboxChanged(state, field))
		elif type(value) is int:
			q = QComboBox()
			idx = 0
			selidx = 0
			for s in TEXTURE_SIZES:
				if int(s) == value:
					selidx = idx
				q.addItem(s)
				idx += 1
			q.setCurrentIndex(selidx)
			q.currentIndexChanged.connect(lambda state: self.onTextureSizeChanged(state, field))
			
		q.setParent(parent)
		q.setGeometry(FIELD_WIDTH, y, PANE_WIDTH - FIELD_WIDTH, 30)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		q.show()
		return q
	
	# --------------------------------------------------
	# Colors and Icons for main FBX list
	# --------------------------------------------------
	def setFbxColorIconInternal(self, mdc, mt, idx):
		if mdc == CHECKMETA_GOOD:
			self.fbxlist.item(idx).setForeground(QBrush(QColor("#32CD32")))
		elif mdc == CHECKMETA_ERROR:
			self.fbxlist.item(idx).setForeground(QBrush(QColor("#FF0000")))
		elif mdc == CHECKMETA_WARNING:
			self.fbxlist.item(idx).setForeground(QBrush(QColor("#FF7F50")))
		elif mdc == CHECKMETA_NORELEASE:
			self.fbxlist.item(idx).setForeground(QBrush(QColor("#000000")))
			
		if mt == MASK_UNKNOWN:
			self.fbxlist.item(idx).setIcon(QIcon("icons/unknownicon.png"))
		elif mt == MASK_NORMAL:
			self.fbxlist.item(idx).setIcon(QIcon("icons/maskicon.png"))
		elif mt == MASK_MORPH:
			self.fbxlist.item(idx).setIcon(QIcon("icons/morphicon.png"))

	def setFbxColorIcon(self, idx):
		mdc,mt = checkMetaDataFile(self.fbxfiles[idx])
		self.setFbxColorIconInternal(mdc, mt, idx)

	def updateFbxColorIcon(self):
		mdc,mt = checkMetaData(self.metadata)
		self.setFbxColorIconInternal(mdc, mt, self.currentFbx)
			
			
	# --------------------------------------------------
	# createMaskEditPane
	# --------------------------------------------------
	def createMaskEditPane(self, fbxfile):
		if self.editPane:
			self.mainLayout.removeWidget(self.editPane)
			self.editPane.deleteLater()
	
		self.editPane = QWidget()
		self.mainLayout.addWidget(self.editPane)
		self.editPane.setMinimumWidth(PANE_WIDTH)
		self.editPane.show()

		# empty pane
		if fbxfile == None:
			return
		
		# mask icon png
		q = QLabel()
		q.setParent(self.editPane)
		pf = os.path.abspath(fbxfile.lower().replace(".fbx",".gif"))
		if os.path.exists(pf):
			m = QMovie(pf)
		else:
			m = QMovie("icons/noicon.png")
		q.setMovie(m)
		m.start()
		q.setScaledContents(True)
		q.setGeometry(0, 10, 64, 64)
		q.show()

		# mask file name
		q = QLabel(fbxfile[2:])
		q.setParent(self.editPane)
		q.setGeometry(66, 44, 600, 36)
		q.setFont(QFont( "Arial", 14, QFont.Bold ))
		q.setToolTip("This is a tip.\nHopefully on two\nlines.")
		q.show()
		
		# buttons
		b = QPushButton("BUILD")
		b.setParent(self.editPane)
		b.setGeometry(66, 10, 64, 32)
		q.setFont(QFont( "Arial", 14, QFont.Bold ))
		b.pressed.connect(lambda: self.onBuild())
		b.show()
		
		# mask meta data fields
		y = 100
		dy = 40
		self.paneWidgets = dict()
		for field in MASK_UI_FIELDS:
			w = self.createLabelWidget(self.editPane, MASK_UI_FIELDS[field], field, y, field in ["uuid"])
			self.paneWidgets[field] = w
			y += dy
			
		
	# --------------------------------------------------
	# saveCurrentMetadata
	# --------------------------------------------------
	def saveCurrentMetadata(self):
		if self.metadata:
			fbxfile = self.fbxfiles[self.currentFbx]
			metafile = getMetaFileName(fbxfile)
			writeMetaData(metafile, self.metadata)
	
	# --------------------------------------------------
	# WIDGET SIGNALS CALLBACKS
	# --------------------------------------------------
	
	# FBX file clicked in list
	def onFbxClicked(self):
		self.saveCurrentMetadata()
		self.currentFbx = self.fbxlist.currentRow()
		fbxfile = self.fbxfiles[self.currentFbx]
		self.metadata = createGetMetaData(fbxfile)
		self.updateFbxColorIcon()
		self.createMaskEditPane(fbxfile)
		
	# text field changed
	def onTextFieldChanged(self, text, field):
		self.metadata[field] = text
		if critical(field) and len(text) == 0:
			self.paneWidgets[field].setStyleSheet("border: 1px solid #FF0000;")
		elif desired(field) and len(text) == 0:
			self.paneWidgets[field].setStyleSheet("border: 1px solid #FF7F50;")
		else:
			self.paneWidgets[field].setStyleSheet("border: 0px;")
		self.updateFbxColorIcon()
			
	# checkbox changed
	def onCheckboxChanged(self, state, field):
		if state == 0:
			self.metadata[field] = False
		else:
			self.metadata[field] = True
		self.updateFbxColorIcon()
		
	# texsize changed
	def onTextureSizeChanged(self, state, field):
		self.metadata[field] = int(TEXTURE_SIZES[state])
		
	# called before exit
	def finalCleanup(self):
		self.saveCurrentMetadata()
		metafile = "./.art/config.meta"
		geo = self.geometry()
		self.config["x"] = geo.x()
		self.config["y"] = geo.y()
		writeMetaData(metafile, self.config)

	# build
	def onBuild(self):
		fbxfile = self.fbxfiles[self.currentFbx]
		
		for line in mmImport(fbxfile, self.metadata):
			self.outputWindow.append(line)
		
		jsonfile = jsonFromFbx(fbxfile)
		if self.metadata["depth_head"]:
			# material
			kvp = { "type":"material", 
					"name":"depth_head_mat", 
					"effect":"effectDefault", 
					"depth-only":True }			
			for line in maskmaker("addres", kvp, [jsonfile]):
				self.outputWindow.append(line)
			# model
			kvp = { "type":"model", 
					"name":"depth_head_mdl", 
					"mesh":"meshHead", 
					"material":"depth_head_mat" }			
			for line in maskmaker("addres", kvp, [jsonfile]):
				self.outputWindow.append(line)
			# part
			kvp = { "type":"model", 
					"name":"depth_head", 
					"resource":"depth_head_mdl" }			
			for line in maskmaker("addpart", kvp, [jsonfile]):
				self.outputWindow.append(line)
		
		# DEPS TEST
		#deps = mmDepends(fbxfile)
		#dirname = os.path.dirname(fbxfile)
		#for d in deps:
		#	print(os.path.abspath(os.path.join(dirname, d)))
		



# ==============================================================================
# MAIN ENTRY POINT
# ==============================================================================
if __name__ == '__main__':
	
	# We're a Qt App
	app = QApplication(sys.argv)
		
	# Show the window
	w = ArtToolWindow()
	w.show()
	
	# Run application
	r = app.exec_()
	
	# Final cleanup
	w.finalCleanup()
	
	# Exit properly
	sys.exit(r)
	
	
