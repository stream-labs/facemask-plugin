@ECHO OFF

set FILE1=%1
set FILE2=%2
set FILE3=%3
set FILE4=%4
set FILE5=%5
set FILE6=%6
set FILE7=%7
set FILE8=%8
set FILE9=%9

%~dp0\MaskMaker.exe merge %FILE1% %FILE2% %FILE3% %FILE4% %FILE5% %FILE6% %FILE7% %FILE8% %FILE9% %FILE1:~0,-5%.merged.json
 