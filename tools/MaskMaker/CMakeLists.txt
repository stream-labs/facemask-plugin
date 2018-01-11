cmake_minimum_required (VERSION 2.6)
project (MaskMaker)

# assimp
SET(PATH_ASSIMP "" CACHE PATH "Assimp - Open Asset Import Library")
if(PATH_ASSIMP STREQUAL "")
	message(FATAL_ERROR "PATH_ASSIMP not set!")
	return()
endif()
if(NOT EXISTS "${PATH_ASSIMP}/include/assimp/Importer.hpp")
	message(FATAL_ERROR "PATH_ASSIMP is invalid!")
	return()
endif()
include_directories(${PATH_ASSIMP}/include)

# For Zlib
SET(OPENCV_TP_DIR "${PROJECT_SOURCE_DIR}/../../thirdparty/opencv/3rdparty")
SET(ZLIB_LIBS
	debug "${OPENCV_TP_DIR}/lib/Debug/zlibd"
	optimized "${OPENCV_TP_DIR}/lib/Release/zlib"
)
include_directories(${OPENCV_TP_DIR}/include)

# For Base64
SET(FACEMASK_PLUGIN_DIR "${PROJECT_SOURCE_DIR}/../..")
include_directories(${FACEMASK_PLUGIN_DIR})

SET(MaskMaker_HEADERS
	"args.h"
	"command_tweak.h"
	"${FACEMASK_PLUGIN_DIR}/base64.h"
	"fifo_map.hpp"
	"json.hpp"
	"stdafx.h"
	"targetver.h"
	"utils.h"
	"stb_image.h"
	"stb_image_resize.h"
)

SET(MaskMaker_SOURCES
	"${FACEMASK_PLUGIN_DIR}/base64.cpp"
	"args.cpp"
	"MaskMaker.cpp"
	"command_tweak.cpp"
	"stdafx.cpp"
	"utils.cpp"
)

add_executable(MaskMaker ${MaskMaker_HEADERS} ${MaskMaker_SOURCES})
target_link_libraries(MaskMaker
	${PATH_ASSIMP}/build64/code/$<CONFIG>/assimp-vc140-mt.lib
	${ZLIB_LIBS}
)
