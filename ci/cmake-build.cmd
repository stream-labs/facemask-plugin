cmake ^
	-H. ^
	-B"%SLBuildDirectory%" ^
	-G"%SLGenerator%" ^
	-DCMAKE_INSTALL_PREFIX="%SLFullDistributePath%\facemask-plugin" ^
	-FACEMASK_EXTERNAL=OFF 

cmake --build %SLBuildDirectory% --target install --config RelWithDebInfo