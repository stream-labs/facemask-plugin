@ECHO OFF

set FOLDER=%1
call set GIFFILE=%%FOLDER:json.render\=gif%%
call set GIFFILE=%%GIFFILE:json.render=gif%%
call set MP4FILE=%%FOLDER:json.render\=mp4%%
call set MP4FILE=%%MP4FILE:json.render=mp4%%
call set PNGFILE=%%FOLDER:json.render\=png%%
call set PNGFILE=%%PNGFILE:json.render=png%%

cd %FOLDER%

ffmpeg -y -i frame%%04d.png -vf fps=15,scale=250:-1:flags=lanczos,palettegen palette.png
ffmpeg -y -framerate 60 -i frame%%04d.png -i palette.png -filter_complex "fps=15,scale=250:-1:flags=lanczos[x];[x][1:v]paletteuse" %GIFFILE%
ffmpeg -y -r 60 -f image2 -i frame%%04d.png -an -vcodec libx264 -g 300 -preset veryslow -tune zerolatency -profile:v baseline -level 3.0 -crf 28 -pix_fmt yuv420p -vf scale=250x250 %MP4FILE%
rem gifsicle.exe -O3 -k 64 --batch %GIFFILE%
rem giflossy.exe -O3 --lossy=160 -k 64 --dither=o8 --batch %GIFFILE%
giflossy.exe -O3 --lossy=160 --batch %GIFFILE%

copy last_frame.png "%PNGFILE%"
convert "%PNGFILE%" -resize 250x250 "%PNGFILE%"

cd ..

svn add %GIFFILE%
svn add %MP4FILE%
svn add %PNGFILE%

rd /s /q %FOLDER%

