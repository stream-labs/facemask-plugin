1>2# : ^
'''
@echo off
rem https://stackoverflow.com/questions/17467441/how-to-embed-python-code-in-batch-script
%userprofile%\\AppData\\Local\\Programs\\Python\\Python36-32\\python "%~f0"
exit /b
rem ^
'''


import sys, subprocess, os, json
from PyQt5.QtWidgets import QApplication, QWidget, QListWidget, QVBoxLayout, QPushButton, QScrollArea, QMainWindow, QCheckBox, QHBoxLayout, QTextEdit
from PyQt5.QtGui import QIcon, QBrush, QColor


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
		raise subprocess.CalledProcessError(return_code, cmd)

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
		metadata["name"] = os.path.basename(fbxfile).lower().replace(".fbx","")
		metadata["description"] = ""
		metadata["author"] = ""
		metadata["tags"] = ""
		metadata["needs_depth"] = False
		metadata["do_not_release"] = False
		metadata["license"] = "Copyright 2017 - General Working Inc. - All rights reserved."		
		metadata["website"] = "http://streamlabs.com"
		f = open(metafile,"w")
		f.write(json.dumps(metadata, indent=4))
		f.close()
		cmd = SVNBIN + " add " + os.path.abspath(metafile)
		for line in execute(cmd):
			print("SVN: " + line)
		
	return metadata
		
		
		
class ArtToolWindow(QMainWindow): 

	def __init__(self, *args): 
		QMainWindow.__init__(self, *args) 
 
		# Main pane for the window
		mainPane = QWidget()
		 
		# Get list of fbx files
		self.fbxfiles = getFileList(".")
		 
		# make a list widget
		fbxlist = QListWidget(mainPane)
		for fbx in self.fbxfiles:
			fbxlist.addItem(fbx[1:])
		fbxlist.setGeometry(10,10,300,500)
		
		# color some items
		fbxlist.item(10).setForeground(QBrush(QColor("#FF0000")))
		fbxlist.item(10).setIcon(QIcon('arttoolicon.png'))
		
		# Show the window
		self.setCentralWidget(mainPane)
		self.setGeometry(100,100, 800, 600)
		self.setWindowTitle('Streamlabs Art Tool')
		self.setWindowIcon(QIcon('arttoolicon.png'))
		
		metadata = createGetMetaData(self.fbxfiles[0])
		print(metadata)
		

if __name__ == '__main__':
	
	# We're a Qt App
	app = QApplication(sys.argv)
		
	# Show the window
	w = ArtToolWindow()
	w.show()

	# Exit properly
	sys.exit(app.exec_())
	
	
