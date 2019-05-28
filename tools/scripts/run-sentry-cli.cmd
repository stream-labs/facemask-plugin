md syms

rem -> copy all .pdb files from plugins dir
cd "\projects\source\build\distribute\slobs\RelWithDebInfo\data\obs-plugins\"
for /f %%f in ('dir /b "\projects\source\build\distribute\slobs\RelWithDebInfo\data\obs-plugins\*.pdb"') do "\projects\source\dump_syms.exe" %%f > "\projects\source\syms\%%~nf.sym"

cd "\projects\source\"

"sentry-cli.exe" --auth-token "d6526d57bb84421eaaeff7983639897de9a0c51ab5274cf4b89d3ad3944d3cbd" upload-dif --org streamlabs-obs --project obs-server "syms\"
"sentry-cli.exe" --auth-token "d6526d57bb84421eaaeff7983639897de9a0c51ab5274cf4b89d3ad3944d3cbd" upload-dif --org streamlabs-obs --project obs-server-preview "syms\"
"sentry-cli.exe" --auth-token "d6526d57bb84421eaaeff7983639897de9a0c51ab5274cf4b89d3ad3944d3cbd" upload-dif --org streamlabs-obs --project obs-client "syms\"