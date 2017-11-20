@ECHO OFF

set FBXFILE=%1

%~dp0\MaskMaker.exe import file=%FBXFILE% %FBXFILE:~0,-4%.json
 