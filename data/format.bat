@ECHO OFF

set INPUT=input.mp4

if NOT [%~1]==[] set INPUT=%1
echo %INPUT%

:: Clean
rm list	
rm tmpFile


(ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=s=x:p=0 %INPUT%)>tmpFile
set /P DIM= <tmpFile
rm tmpFile
	
ffmpeg -y -f lavfi -i color=red:s=%DIM%:r=24000/1001 -f lavfi -i anullsrc  -ar 48000 -ac 2 -t 1 red.mp4


echo file %INPUT% >> list
echo file red.mp4 >> list

:: Concatenate Files
ffmpeg -y -f concat -i list -c copy output.mp4

:: Clean
rm red.mp4
rm list