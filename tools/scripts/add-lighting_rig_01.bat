@ECHO OFF

set FILE1=%1

%~dp0\MaskMaker.exe merge %FILE1% %~dp0\..\lighting_rigs\lighting_rig_01.json %FILE1:~0,-5%.lit.json
 