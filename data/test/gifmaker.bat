@ECHO OFF

set FOLDER=%1
call set GIFFILE=%%FOLDER:json.render\=gif%%
call set GIFFILE=%%GIFFILE:json.render=gif%%
call set MP4FILE=%%FOLDER:json.render\=mp4%%
call set MP4FILE=%%MP4FILE:json.render=mp4%%
call set PNGFILE=%%FOLDER:json.render\=png%%
call set PNGFILE=%%PNGFILE:json.render=png%%

cd %FOLDER%

ffmpeg -y -i frame%%04d.png -vf fps=20,scale=320:-1:flags=lanczos,palettegen palette.png
ffmpeg -y -framerate 60 -i frame%%04d.png -i palette.png -filter_complex "fps=15,scale=256:-1:flags=lanczos[x];[x][1:v]paletteuse" %GIFFILE%
ffmpeg -y -r 60 -f image2 -s 256x256 -i frame%%04d.png -an -vcodec libx264  -g 75 -keyint_min 12 -vb 4000k -vprofile high -level 40 -crf 20  -pix_fmt yuv420p %MP4FILE%

copy frame0180.png "%PNGFILE%"

cd ..

svn add %GIFFILE%
svn add %MP4FILE%
svn add %PNGFILE%

rd /s /q %FOLDER%

