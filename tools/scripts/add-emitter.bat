@ECHO OFF

set FILE1=%1

%~dp0\MaskMaker.exe addres file=..\pooHat\pooSplat_grid_256x256.png name=partimage %FILE1%
%~dp0\MaskMaker.exe addres type=material name=partmat effect=effectDefault image=texture,partimage %FILE1%
%~dp0\MaskMaker.exe addres type=model name=particle mesh=meshQuad material=partmat %FILE1%
%~dp0\MaskMaker.exe addres type=emitter name=emit model=particle %FILE1%
