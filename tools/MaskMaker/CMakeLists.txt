cmake_minimum_required (VERSION 2.6)
project (MaskMaker)

# assimp
SET(PATH_ASSIMP "${PROJECT_SOURCE_DIR}/../../thirdparty/assimp")
if(NOT EXISTS "${PATH_ASSIMP}/include/assimp/Importer.hpp")
	message(FATAL_ERROR "PATH_ASSIMP is invalid!")
	return()
endif()

add_subdirectory(${PATH_ASSIMP} ${PATH_ASSIMP}/build)

# assimp include files
include_directories(${PATH_ASSIMP}/include)
include_directories(${PATH_ASSIMP}/build/include)

# For Zlib
SET(OPENCV_TP_DIR "${PROJECT_SOURCE_DIR}/../../thirdparty/opencv/3rdparty")
SET(ZLIB_LIBS
	debug "${OPENCV_TP_DIR}/lib/Debug/zlibd.lib"
	optimized "${OPENCV_TP_DIR}/lib/Release/zlib.lib"
)
include_directories(${OPENCV_TP_DIR}/include)

# For Base64
SET(FACEMASK_PLUGIN_DIR "${PROJECT_SOURCE_DIR}/../../plugin")
include_directories(${FACEMASK_PLUGIN_DIR})

SET(MaskMaker_HEADERS
	"args.h"
	"command_create.h"
	"command_addres.h"
	"command_addpart.h"
	"command_depends.h"
	"command_import.h"
	"command_inspect.h"
	"command_morph_import.h"
	"command_merge.h"
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
	"command_create.cpp"
	"command_addres.cpp"
	"command_addpart.cpp"
	"command_depends.cpp"
	"command_inspect.cpp"
	"command_import.cpp"
	"command_morph_import.cpp"
	"command_merge.cpp"
	"command_tweak.cpp"
	"stdafx.cpp"
	"utils.cpp"
)

add_executable(MaskMaker ${MaskMaker_HEADERS} ${MaskMaker_SOURCES})
target_link_libraries(MaskMaker
	${PATH_ASSIMP}/build/code/$<CONFIG>/assimp-vc140-mt.lib
	${ZLIB_LIBS}
)

