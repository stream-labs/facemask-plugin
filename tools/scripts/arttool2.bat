1>2# : ^
'''
@echo off
rem https://stackoverflow.com/questions/17467441/how-to-embed-python-code-in-batch-script
%userprofile%\\AppData\\Local\\Programs\\Python\\Python36-32\\python "%~f0"
exit /b
rem ^
'''


import sys, subprocess, os, json, uuid
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QListWidget, QVBoxLayout, QPushButton, QScrollArea, QMainWindow, QCheckBox, QHBoxLayout, QTextEdit, QLineEdit
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

def getMetaFolder(fbxfile):
	dn = os.path.dirname(fbxfile)
	dn = os.path.join(dn, ".art").replace("\\","/")
	return dn
	
def createMetaFolder(folder):
	if not os.path.exists(folder):
		os.makedirs(folder)
		cmd = SVNBIN + " add " + os.path.abspath(folder)
		for line in execute(cmd):
			print("SVN: " + line)

def writeMetaData(metafile, metadata):
	f = open(metafile,"w")
	f.write(json.dumps(metadata, indent=4))
	f.close()
	cmd = SVNBIN + " add " + os.path.abspath(metafile)
	for line in execute(cmd):
		print("SVN: " + line)

def createGetMetaData(fbxfile):
	metafolder = getMetaFolder(fbxfile)
	createMetaFolder(metafolder)
	metafile = os.path.join(metafolder, os.path.basename(fbxfile).lower().replace(".fbx",".meta")).replace("\\","/")
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
		metadata["texture_max"] = 256
		metadata["do_not_release"] = False
		metadata["license"] = "Copyright 2017 - General Working Inc. - All rights reserved."		
		metadata["website"] = "http://streamlabs.com"
		# write it
		writeMetaData(metafile, metadata)
		
	return metadata

	

	
	
	
	
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
		
		# color some items
		self.fbxlist.item(0).setForeground(QBrush(QColor("#FF0000")))
		self.fbxlist.item(0).setIcon(QIcon('maskicon.png'))
		self.fbxlist.item(1).setForeground(QBrush(QColor("#32CD32")))
		self.fbxlist.item(1).setIcon(QIcon('morphicon.png'))
		self.fbxlist.item(2).setForeground(QBrush(QColor("#FF7F50")))
		self.fbxlist.item(2).setIcon(QIcon('maskicon.png'))
		
		# Show the window
		self.setCentralWidget(mainPane)
		self.setGeometry(2000, 100, 1000, 600)
		self.setWindowTitle('Streamlabs Art Tool')
		self.setWindowIcon(QIcon('arttoolicon.png'))
		
		# Blank pane
		self.rightPane = None
		self.createRightPane(None)
		
		# test
		self.metadata = createGetMetaData(self.fbxfiles[0])
	
	def createTextUI(self, name, field, y):
	
		critical = field in ["name", "author", "license"]
	
		q = QLabel(name)
		q.setParent(self.rightPane)
		q.setGeometry(0, y, 500, 36)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		#q.setStyleSheet('color: #FF0000')
		q.show()

		text = self.metadata[field]
		q = QLineEdit(text)
		q.setParent(self.rightPane)
		q.setGeometry(100, y, 500, 30)
		q.setFont(QFont( "Arial", 12, QFont.Bold ))
		if critical and len(text) == 0:
			q.setStyleSheet("border: 1px solid #FF0000;")
		else:
			q.setStyleSheet("border: 0px;")
		q.textChanged.connect(lambda text: self.onTextFieldChanged(text, field))
		q.show()
	
		self.paneWidgets[field] = q
	
	
	# Creates right pane
	#
	def createRightPane(self, fbxfile):
		if self.rightPane:
			self.mainLayout.removeWidget(self.rightPane)
			self.rightPane.deleteLater()
	
		self.rightPane = QWidget()
		self.mainLayout.addWidget(self.rightPane)
		self.rightPane.setGeometry(0,0,500, 500)
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
		q.setGeometry(66, 44, 500, 36)
		q.setFont(QFont( "Arial", 14, QFont.Bold ))
		q.show()
		
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
		

	
	# FBX file clicked in list
	#
	def onFbxClicked(self):
		fbxfile = self.fbxfiles[self.fbxlist.currentRow()]
		self.createRightPane(fbxfile)

	# text field changed
	def onTextFieldChanged(self, text, field):
		critical = field in ["name", "author", "license"]
		self.metadata[field] = text
		if critical and len(text) == 0:
			self.paneWidgets[field].setStyleSheet("border: 1px solid #FF0000;")
		else:
			self.paneWidgets[field].setStyleSheet("border: 0px;")		
			
		
		
		
if __name__ == '__main__':
	
	# We're a Qt App
	app = QApplication(sys.argv)
		
	# Show the window
	w = ArtToolWindow()
	w.show()

	# Exit properly
	sys.exit(app.exec_())
	
	
