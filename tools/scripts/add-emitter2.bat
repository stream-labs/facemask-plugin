@ECHO OFF

set FILE1=%1

%~dp0\MaskMaker.exe addres type=emitter name=emit model=mesh0Model %FILE1%
