1>2# : ^
'''
@echo off
rem https://stackoverflow.com/questions/17467441/how-to-embed-python-code-in-batch-script
%userprofile%\\AppData\\Local\\Programs\\Python\\Python36-32\\python "%~f0"
exit /b
rem ^
'''


import sys, subprocess, os, json, uuid
from PyQt5.QtWidgets import QApplication, QWidget, QLabel, QListWidget, QVBoxLayout, QPushButton, QScrollArea, QMainWindow, QCheckBox, QHBoxLayout, QTextEdit
from PyQt5.QtGui import QIcon, QBrush, QColor, QFont, QPixmap


MAKEARTBATFILE = os.path.join(os.path.dirname(os.path.abspath(__file__)),"makeart.bat")

SVNBIN = os.path.join("c:\\",'"Program Files"',"TortoiseSVN","bin","svn")

	
# Executes a shell command
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
		self.mainPane = QWidget()
		 
		# Get list of fbx files
		self.fbxfiles = getFileList(".")
		 
		# make a list widget
		self.fbxlist = QListWidget(self.mainPane)
		for fbx in self.fbxfiles:
			self.fbxlist.addItem(fbx[2:])
		self.fbxlist.setGeometry(10, 10, 300, 500)
		self.fbxlist.itemClicked.connect(lambda: self.onFbxClicked())
		
		# color some items
		self.fbxlist.item(10).setForeground(QBrush(QColor("#FF0000")))
		self.fbxlist.item(10).setIcon(QIcon('maskicon.png'))
		self.fbxlist.item(11).setForeground(QBrush(QColor("#32CD32")))
		self.fbxlist.item(11).setIcon(QIcon('morphicon.png'))
		self.fbxlist.item(12).setForeground(QBrush(QColor("#FF7F50")))
		self.fbxlist.item(12).setIcon(QIcon('maskicon.png'))
		
		# Show the window
		self.setCentralWidget(self.mainPane)
		self.setGeometry(100,100, 1000, 600)
		self.setWindowTitle('Streamlabs Art Tool')
		self.setWindowIcon(QIcon('arttoolicon.png'))
		
		self.fag = None
		
		metadata = createGetMetaData(self.fbxfiles[0])
		print(metadata)
		
	def onFbxClicked(self):
	
		fbxfile = self.fbxfiles[self.fbxlist.currentRow()]
		
		if self.fag:
			self.fag.setParent(None)
			self.fag = None
		
		f = QFont( "Arial", 12, QFont.Bold )
		self.fag = QWidget(self.mainPane)

		# mask icon png
		q = QLabel()
		q.setParent(self.fag)
		pf = os.path.abspath(fbxfile.lower().replace(".fbx",".png"))
		q.setPixmap(QPixmap(pf))
		q.setScaledContents(True)
		q.setGeometry(330, 10, 64, 64)
		q.show()

		# mask file name
		q = QLabel(fbxfile[2:])
		q.setParent(self.fag)
		q.setGeometry(400, 10, 500, 36)
		q.setFont(f)
		q.show()
				
		self.fag.show()
		

if __name__ == '__main__':
	
	# We're a Qt App
	app = QApplication(sys.argv)
		
	# Show the window
	w = ArtToolWindow()
	w.show()

	# Exit properly
	sys.exit(app.exec_())
	
	
