:: Left Input
set INPUT1=input1.mp4 
:: Right Output
set INPUT2=input2.mp4
:: Before Text
set TEXT1='Before'
:: After Text
set TEXT2='After'
:: Output File
set OUT=output.mp4

:: FONT
set FONT='/Windows/Fonts/Arial.ttf'

if NOT ["%~1"]==[] set INPUT1="%~1"
if NOT ["%~2"]==[] set INPUT2="%~2"
if NOT ["%~3"]==[] set TEXT2='%~3'
if NOT ["%~4"]==[] set TEXT2='%~4'
if NOT ["%~5"]==[] set OUT="%~5"

:: Clean
rm temp1.mp4
rm temp2.mp4

ffmpeg  -y -i %INPUT1% -i %INPUT2% -filter_complex hstack temp1.mp4
ffmpeg  -y  -i temp1.mp4 -vf drawtext="fontfile=%FONT%: text=%TEXT1%: fontcolor=black: fontsize=72: box=1: boxcolor=yellow@0.4: boxborderw=5: x=10: y=10" -codec:a copy temp2.mp4
ffmpeg  -y  -i temp2.mp4 -vf drawtext="fontfile=%FONT%: text=%TEXT2%: fontcolor=black: fontsize=72: box=1: boxcolor=orange@0.4: boxborderw=5: x=w/2 + 12: y=10" -codec:a copy %OUT%

:: Clean
rm temp1.mp4
rm temp2.mp4
