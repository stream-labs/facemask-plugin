@ECHO OFF

set FILE1=%1

%~dp0\MaskMaker.exe addres type=sequence name=partseq image=partimage rows=4 cols=4 first=0 last=15 %FILE1%
 