@ECHO OFF

set INPUT=input.mp4

if NOT ["%~1"]==[""] set INPUT="%~1"
echo %INPUT%

:: Clean
del list	
del tmpFile


(ffprobe -v error -select_streams v:0 -show_entries stream=width,height -of csv=s=x:p=0 %INPUT%)>tmpFile
set /P DIM= <tmpFile
del tmpFile
	
ffmpeg -y -f lavfi -i color=red:s=%DIM% -f lavfi -i anullsrc -ar 48000 -ac 2 -t 2 red.mp4

echo file red.mp4 >> list
echo file %INPUT% >> list

:: Concatenate Files
ffmpeg -y -f concat -safe 0 -i list -c copy formatted.mp4

:: Clean
del red.mp4
del list