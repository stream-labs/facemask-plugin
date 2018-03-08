1>2# : ^
'''
@echo off
rem https://stackoverflow.com/questions/17467441/how-to-embed-python-code-in-batch-script
%userprofile%\\AppData\\Local\\Programs\\Python\\Python36-32\\python "%~f0"
exit /b
rem ^
'''


import sys, subprocess, os, json, uuid
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QListWidget, QVBoxLayout, QPushButton, QComboBox
from PyQt5.QtWidgets import QScrollArea, QMainWindow, QCheckBox, QHBoxLayout, QTextEdit, QLineEdit
from PyQt5.QtGui import QIcon, QBrush, QColor, QFont, QPixmap, QMovie


MAKEARTBATFILE = os.path.join(os.path.dirname(os.path.abspath(__file__)),"makeart.bat")

SVNBIN = os.path.join("c:\\",'"Program Files"',"TortoiseSVN","bin","svn")

	
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
		pass

# Gets a list of files
def getFileList(folder):

	fileList = list()
	for root,subdir,files in os.walk(folder):
		for file in files:
			if file.lower().endswith(".fbx"):
				fileList.append(os.path.join(root,file).replace("\\","/"))

	return fileList

def svnNeedsUpdate():	
	# have
	cmd = 'svn info | grep -i "Last Changed Rev"'
	revHave = 0
	for line in execute(cmd):
		revHave = int(line.split()[-1])
		
	# head
	cmd = 'svn info -r HEAD | grep -i "Last Changed Rev"'
	revHead = 0
	for line in execute(cmd):
		revHead = int(line.split()[-1])
		
	return revHead > revHave

def svnNeedsCommit():
	cmd = 'svn status -uq'
	needCommit = False
	for line in execute(cmd):
		if line.split()[0] in ["A", "M", "D"]:
			needCommit = True
	return needCommit

	
def getMetaFolderName(fbxfile):
	dn = os.path.dirname(fbxfile)
	dn = os.path.join(dn, ".art").replace("\\","/")
	return dn
	
def createMetaFolder(folder):
	if not os.path.exists(folder):
		os.makedirs(folder)
		cmd = SVNBIN + " add " + os.path.abspath(folder)
		for line in execute(cmd):
			print("SVN: " + line)

def getMetaFileName(fbxfile):
	metafolder = getMetaFolderName(fbxfile)
	metafile = os.path.join(metafolder, os.path.basename(fbxfile).lower().replace(".fbx",".meta")).replace("\\","/")
	return metafile
			
def writeMetaData(metafile, metadata):
	f = open(metafile,"w")
	f.write(json.dumps(metadata, indent=4))
	f.close()
	cmd = SVNBIN + " add " + os.path.abspath(metafile)
	for line in execute(cmd):
		print("SVN: " + line)

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
		metadata["name"] = ""
		metadata["description"] = ""
		metadata["author"] = ""
		metadata["tags"] = ""
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
			
	# todo: check for icons
	
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
	
	
	
FIELD_WIDTH = 150
PANE_WIDTH = 700
TEXTURE_SIZES = ["32","64","128","256","512","1024","2048"]
	
def critical(field):
	return field in ["name", "author", "license"]

	
class ArtToolWindow(QMainWindow): 

	def __init__(self, *args): 
		QMainWindow.__init__(self, *args) 
 
		# Main pane for the window
		mainPane = QWidget()
		self.mainLayout = QHBoxLayout(mainPane)
		 
		# Get list of fbx files
		self.fbxfiles = getFileList(".")
		 
		# make a list widget
		self.fbxlist = QListWidget()
		for fbx in self.fbxfiles:
			self.fbxlist.addItem(fbx[2:])
		self.fbxlist.setGeometry(0, 0, 300, 500)
		self.fbxlist.setMaximumWidth(300)
		self.fbxlist.itemClicked.connect(lambda: self.onFbxClicked())
		self.mainLayout.addWidget(self.fbxlist)
		
		# color items
		for idx in range(0, len(self.fbxfiles)):
			self.setFbxColorIcon(idx)
			
		# Show the window
		self.setCentralWidget(mainPane)
		self.setGeometry(2000, 100, 1024, 768)
		self.setWindowTitle('Streamlabs Art Tool')
		self.setWindowIcon(QIcon('arttoolicon.png'))
		
		# Blank pane
		self.rightPane = None
		self.createRightPane(None)
		
		# test
		self.metadata = None 
		self.currentFbx = -1

	
	def createTextUI(self, name, field, y):
		q = QLabel(name)
		q.setParent(self.rightPane)
		q.setGeometry(0, y, FIELD_WIDTH, 36)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		#q.setStyleSheet('color: #FF0000')
		q.show()

		text = self.metadata[field]
		q = QLineEdit(text)
		q.setParent(self.rightPane)
		q.setGeometry(FIELD_WIDTH, y, PANE_WIDTH - FIELD_WIDTH, 30)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		if critical(field) and len(text) == 0:
			q.setStyleSheet("border: 1px solid #FF0000;")
		else:
			q.setStyleSheet("border: 0px;")
		q.textChanged.connect(lambda text: self.onTextFieldChanged(text, field))
		q.show()
	
		self.paneWidgets[field] = q
	
	def createLabelUI(self, name, field, y):
		q = QLabel(name)
		q.setParent(self.rightPane)
		q.setGeometry(0, y, FIELD_WIDTH, 36)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		q.show()

		text = self.metadata[field]
		q = QLabel(text)
		q.setParent(self.rightPane)
		q.setGeometry(FIELD_WIDTH, y, PANE_WIDTH - FIELD_WIDTH, 30)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		q.show()
	
		self.paneWidgets[field] = q

	def createCheckboxUI(self, name, field, y):
		q = QLabel(name)
		q.setParent(self.rightPane)
		q.setGeometry(0, y, FIELD_WIDTH, 36)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		q.show()

		isset = self.metadata[field]
		q = QCheckBox()
		q.setParent(self.rightPane)
		q.setChecked(isset)
		q.setGeometry(FIELD_WIDTH, y, PANE_WIDTH - FIELD_WIDTH, 30)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		q.stateChanged.connect(lambda state: self.onCheckboxChanged(state, field))
		q.show()
	
		self.paneWidgets[field] = q

	def createTextureSizeUI(self, name, field, y):
		q = QLabel(name)
		q.setParent(self.rightPane)
		q.setGeometry(0, y, FIELD_WIDTH, 36)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		q.show()

		texsize = self.metadata[field]
		q = QComboBox()
		idx = 0
		selidx = 0
		for s in TEXTURE_SIZES:
			if int(s) == texsize:
				selidx = idx
			q.addItem(s)
			idx += 1
		q.setCurrentIndex(selidx)
		q.setParent(self.rightPane)
		q.setGeometry(FIELD_WIDTH, y, 100, 30)
		q.currentIndexChanged.connect(lambda state: self.onTextureSizeChanged(state, field))
		q.show()
	
		self.paneWidgets[field] = q

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
			self.fbxlist.item(idx).setIcon(QIcon("unknownicon.png"))
		elif mt == MASK_NORMAL:
			self.fbxlist.item(idx).setIcon(QIcon("maskicon.png"))
		elif mt == MASK_MORPH:
			self.fbxlist.item(idx).setIcon(QIcon("morphicon.png"))

	def setFbxColorIcon(self, idx):
		mdc,mt = checkMetaDataFile(self.fbxfiles[idx])
		self.setFbxColorIconInternal(mdc, mt, idx)

	def updateFbxColorIcon(self):
		mdc,mt = checkMetaData(self.metadata)
		self.setFbxColorIconInternal(mdc, mt, self.currentFbx)
			
	# Creates right pane
	#
	def createRightPane(self, fbxfile):
		if self.rightPane:
			self.mainLayout.removeWidget(self.rightPane)
			self.rightPane.deleteLater()
	
		self.rightPane = QWidget()
		self.mainLayout.addWidget(self.rightPane)
		self.rightPane.setGeometry(0,0,PANE_WIDTH, 500)
		self.rightPane.show()

		# empty pane
		if fbxfile == None:
			return
		
		# mask icon png
		q = QLabel()
		q.setParent(self.rightPane)
		pf = os.path.abspath(fbxfile.lower().replace(".fbx",".gif"))
		m = QMovie(pf)
		q.setMovie(m)
		m.start()
		q.setScaledContents(True)
		q.setGeometry(0, 10, 64, 64)
		q.show()

		# mask file name
		q = QLabel(fbxfile[2:])
		q.setParent(self.rightPane)
		q.setGeometry(66, 44, 600, 36)
		q.setFont(QFont( "Arial", 14, QFont.Bold ))
		q.show()
		
		# widgets below stored in this dict
		self.paneWidgets = dict()
		
		# name
		y = 100
		dy = 40
		self.createTextUI("Pretty Name", "name", y)
		
		# description
		y += dy
		self.createTextUI("Description", "description", y)
		
		# author
		y += dy
		self.createTextUI("Author", "author", y)
		
		# tags
		y += dy
		self.createTextUI("Tags", "tags", y)
		
		# uuid
		y += dy
		self.createLabelUI("UUID", "uuid", y)

		# depth_head
		y += dy
		self.createCheckboxUI("Depth Head", "depth_head", y)
	
		# is_morph
		y += dy
		self.createCheckboxUI("Is a Morph", "is_morph", y)
	
		# is_vip
		y += dy
		self.createCheckboxUI("V.I.P. Mask", "is_vip", y)
	
		# do_not_release
		y += dy
		self.createCheckboxUI("Do Not Release", "do_not_release", y)
	
		# texture_max
		y += dy
		self.createTextureSizeUI("Max Texture Size", "texture_max", y)
	
		# license
		y += dy
		self.createTextUI("License", "license", y)

		# website
		y += dy
		self.createTextUI("Website", "website", y)
		
	
	# FBX file clicked in list
	#
	def onFbxClicked(self):
		if self.metadata:
			fbxfile = self.fbxfiles[self.currentFbx]
			metafile = getMetaFileName(fbxfile)
			writeMetaData(metafile, self.metadata)
			
		self.currentFbx = self.fbxlist.currentRow()
		fbxfile = self.fbxfiles[self.currentFbx]
		self.metadata = createGetMetaData(fbxfile)
		self.updateFbxColorIcon()
		self.createRightPane(fbxfile)

		
	# text field changed
	def onTextFieldChanged(self, text, field):
		self.metadata[field] = text
		if critical(field) and len(text) == 0:
			self.paneWidgets[field].setStyleSheet("border: 1px solid #FF0000;")
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
		







		
if __name__ == '__main__':
	
	if svnNeedsCommit() :
		print("need commit")
	
	# We're a Qt App
	app = QApplication(sys.argv)
		
	# Show the window
	w = ArtToolWindow()
	w.show()

	# Exit properly
	sys.exit(app.exec_())
	
	
