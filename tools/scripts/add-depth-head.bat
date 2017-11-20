@ECHO OFF

set FILE1=%1

%~dp0\MaskMaker.exe merge %FILE1% %~dp0\..\heads\depth_head.json %FILE1:~0,-5%.head.json
 