1>2# : ^
'''
@echo off
rem https://stackoverflow.com/questions/17467441/how-to-embed-python-code-in-batch-script
if exist "%userprofile%"\\AppData\\Local\\Programs\\Python\\Python36\\python.exe goto PYTHON64
if exist "%userprofile%"\\AppData\\Local\\Programs\\Python\\Python36-32\\python.exe goto PYTHON32
echo "I can't find python (3.6). Please install python from https://www.python.org/downloads/windows/"
pause
exit /b
:PYTHON32
"%userprofile%"\\AppData\\Local\\Programs\\Python\\Python36-32\\python "%~f0" 
exit /b
:PYTHON64
"%userprofile%"\\AppData\\Local\\Programs\\Python\\Python36\\python "%~f0" 
exit /b
rem ^
'''

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
import sys, os, subprocess
try:
	from PyQt5.QtWidgets import QApplication
except:
	print("You don't seem to have PyQt5 installed. I will attempt to install it now.")
	luser = '"' + os.path.expanduser("~") + '"'
	cmd = os.path.join(luser,"AppData","Local","Programs","Python","Python36","python.exe")
	if not os.path.exists(cmd):
		cmd = os.path.join(luser,"AppData","Local","Programs","Python","Python36-32","python.exe")
	if not os.path.exists(cmd):
		print("I can't find python. Really odd, since this is python code.")
	cmd += " -m pip install PyQt5 --user"
	popen = subprocess.Popen(cmd, stdout=subprocess.PIPE, universal_newlines=True, shell=True)
	for stdout_line in iter(popen.stdout.readline, ""):
		print(stdout_line)
	popen.wait()
	print("PyQt5 should be installed now. Try running art tool again.")
	sys.exit(0)
	
	
from arttool import arttool

# ==============================================================================
# MAIN ENTRY POINT
# ==============================================================================
if __name__ == '__main__':
	
	# We're a Qt App
	app = QApplication(sys.argv)
		
	# Show the window
	w = arttool.ArtToolWindow()
	app.focusChanged.connect(lambda old,now: w.onFocusChanged(old,now))
	w.show()
	
	# Run application
	r = app.exec_()
	
	# Final cleanup
	w.finalCleanup()
	
	# Exit properly
	sys.exit(r)
	
	
