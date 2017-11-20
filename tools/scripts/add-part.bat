@ECHO OFF

set FILE1=%1

%~dp0\MaskMaker.exe addpart resource=mesh0Model %FILE1%
