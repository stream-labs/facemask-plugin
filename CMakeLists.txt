cmake_minimum_required(VERSION 2.8.12)
PROJECT(facemask-plugin)
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

################################################################################
# Configuration
################################################################################

# dlib
SET(PATH_DLIB "" CACHE PATH "Dlib Machine Learning C++ Toolkit Source Code Directory")
if(PATH_DLIB STREQUAL "")
	message(FATAL_ERROR "PATH_DLIB not set!")
	return()
endif()
if(NOT EXISTS "${PATH_DLIB}/dlib/image_processing/object_detector.h")
	message(FATAL_ERROR "PATH_DLIB is invalid!")
	return()
endif()

include(${PATH_DLIB}/dlib/cmake)
include_directories(${PATH_DLIB})

# OpenCV
SET(OpenCV_DIR "${PROJECT_SOURCE_DIR}/thirdparty/opencv")
SET(OpenCV_VERSION "331" CACHE STRING "Version of OpenCV to use.")
SET(OpenCV_LIBS
	debug "${OpenCV_DIR}/lib/Debug/opencv_calib3d${OpenCV_VERSION}d.lib"
	optimized "${OpenCV_DIR}/lib/Release/opencv_calib3d${OpenCV_VERSION}.lib"
	debug "${OpenCV_DIR}/lib/Debug/opencv_core${OpenCV_VERSION}d.lib"
	optimized "${OpenCV_DIR}/lib/Release/opencv_core${OpenCV_VERSION}.lib"
	debug "${OpenCV_DIR}/lib/Debug/opencv_highgui${OpenCV_VERSION}d.lib"
	optimized "${OpenCV_DIR}/lib/Release/opencv_highgui${OpenCV_VERSION}.lib"
	debug "${OpenCV_DIR}/lib/Debug/opencv_imgcodecs${OpenCV_VERSION}d.lib"
	optimized "${OpenCV_DIR}/lib/Release/opencv_imgcodecs${OpenCV_VERSION}.lib"
	debug "${OpenCV_DIR}/lib/Debug/opencv_imgproc${OpenCV_VERSION}d.lib"
	optimized "${OpenCV_DIR}/lib/Release/opencv_imgproc${OpenCV_VERSION}.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/libpngd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/libpng.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/zlibd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/zlib.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/ittnotifyd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/ittnotify.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/ippiwd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/ippiw.lib"
	debug "${OpenCV_DIR}/3rdparty/ippicv/ippicv_win_${OpenCV_VERSION}/lib/intel64/ippicvmt.lib"
	optimized "${OpenCV_DIR}/3rdparty/ippicv/ippicv_win_${OpenCV_VERSION}/lib/intel64/ippicvmt.lib"
)
include_directories("${OpenCV_DIR}/include")

# zlib
include_directories("${OpenCV_DIR}/3rdparty/include")

# intel IPP
include_directories("${OpenCV_DIR}/3rdparty/ippicv/ippicv_win_331/include")
include_directories("${OpenCV_DIR}/3rdparty/ippicv/ippiw_win_331/include")

# smll
include_directories("smll")
SET(SMLLDir "${PROJECT_SOURCE_DIR}/smll")

# tinyobjloader
include_directories("thirdparty/tinyobjloader")
SET(TINYOBJDir "thirdparty/tinyobjloader")

# OBS Studio
SET(PATH_OBS_STUDIO "" CACHE PATH "OBS Studio Source Code Directory")
if(PATH_OBS_STUDIO STREQUAL "")
	message(FATAL_ERROR "PATH_OBS_STUDIO not set!")
	return()
endif()
if(NOT EXISTS "${PATH_OBS_STUDIO}/libobs/obs-module.h")
	message(FATAL_ERROR "PATH_OBS_STUDIO is invalid!")
	return()
endif()
SET(obsPath "${PATH_OBS_STUDIO}")
INCLUDE("${PATH_OBS_STUDIO}/cmake/external/Findlibobs.cmake")

# Distribute format
OPTION(BUILD_FOR_PUBLIC "Build for public release" OFF)
if(BUILD_FOR_PUBLIC)
	add_definitions(-DPUBLIC_RELEASE)
endif()


# Distribute format
OPTION(BUILD_SLOBS "Build for Streamlabs OBS" OFF)
if(BUILD_SLOBS)
	SET(DISTRIBUTE_PATH "${PROJECT_BINARY_DIR}/distribute/slobs")
	SET(DISTRIBUTE_PATH_BIN "obs-plugins")
	add_definitions(-DSLOBS_BUILD)
else()
	SET(DISTRIBUTE_PATH "${PROJECT_BINARY_DIR}/distribute/obs")
	SET(DISTRIBUTE_PATH_BIN "obs-plugins/${BITS}bit")
	add_definitions(-DOBS_STUDIO_BUILD)
endif()

################################################################################
# Setup
################################################################################

# Version
SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 1)
SET(VERSION_PATCH 1)
configure_file(
	"${PROJECT_SOURCE_DIR}/version.in.h"
	"${PROJECT_BINARY_DIR}/version.h"
)

# Code
SET(smll_HEADERS
	"${SMLLDir}/Common.hpp"
	"${SMLLDir}/Config.hpp"
	"${SMLLDir}/DetectionResults.hpp"
	"${SMLLDir}/DLibImageWrapper.hpp"
	"${SMLLDir}/Face.hpp"
	"${SMLLDir}/FaceDetector.hpp"
	"${SMLLDir}/Kalman.hpp"
	"${SMLLDir}/ImageWrapper.hpp"
	"${SMLLDir}/landmarks.hpp"
	"${SMLLDir}/MorphData.hpp"
	"${SMLLDir}/OBSRenderer.hpp"
	"${SMLLDir}/OBSTexture.hpp"
	"${SMLLDir}/sarray.hpp"
	"${SMLLDir}/TriangulationResult.hpp"
	"${SMLLDir}/SingleValueKalman.hpp"
)
SET(gs_HEADERS
	"${PROJECT_SOURCE_DIR}/gs-effect.h"
	"${PROJECT_SOURCE_DIR}/gs-vertex.h"
	"${PROJECT_SOURCE_DIR}/gs-vertexbuffer.h"
	"${PROJECT_SOURCE_DIR}/gs-indexbuffer.h"
	"${PROJECT_SOURCE_DIR}/gs-rendertarget.h"
	"${PROJECT_SOURCE_DIR}/gs-texture.h"
)
SET(mask_HEADERS
	"${PROJECT_SOURCE_DIR}/mask.h"
	"${PROJECT_SOURCE_DIR}/mask-instance-data.h"
	"${PROJECT_SOURCE_DIR}/mask-resource.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-animation.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-image.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-sequence.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-effect.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-light.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-material.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-mesh.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-model.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-morph.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-skinned-model.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-sound.h"
	"${PROJECT_SOURCE_DIR}/mask-resource-emitter.h"
)
SET(facemask-plugin_HEADERS
	"${PROJECT_SOURCE_DIR}/base64.h"
	"${PROJECT_SOURCE_DIR}/exceptions.h"
	"${PROJECT_SOURCE_DIR}/face-mask-filter.h"
	"${PROJECT_SOURCE_DIR}/plugin.h"
	"${PROJECT_SOURCE_DIR}/strings.h"
	"${PROJECT_SOURCE_DIR}/utils.h"
	"${PROJECT_BINARY_DIR}/version.h"
	"${TINYOBJDir}/tiny_obj_loader.h"
	"${gs_HEADERS}"
	"${mask_HEADERS}"
	"${smll_HEADERS}"
)
SET(smll_SOURCES
	"${SMLLDir}/Config.cpp"
	"${SMLLDir}/DetectionResults.cpp"
	"${SMLLDir}/Face.cpp"
	"${SMLLDir}/FaceDetector.cpp"
	"${SMLLDir}/OBSRenderer.cpp"
	"${SMLLDir}/ImageWrapper.cpp"
	"${SMLLDir}/landmarks.cpp"
	"${SMLLDir}/MorphData.cpp"
	"${SMLLDir}/TriangulationResult.cpp"
	"${SMLLDir}/SingleValueKalman.cpp"
)
SET(gs_SOURCES
	"${PROJECT_SOURCE_DIR}/gs-effect.cpp"
	"${PROJECT_SOURCE_DIR}/gs-indexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/gs-vertexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/gs-rendertarget.cpp"
	"${PROJECT_SOURCE_DIR}/gs-texture.cpp"
)
SET(mask_SOURCES
	"${PROJECT_SOURCE_DIR}/mask.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-animation.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-image.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-sequence.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-effect.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-light.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-material.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-mesh.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-model.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-morph.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-skinned-model.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-sound.cpp"
	"${PROJECT_SOURCE_DIR}/mask-resource-emitter.cpp"
)
SET(facemask-plugin_SOURCES
	"${PROJECT_SOURCE_DIR}/base64.cpp"
	"${PROJECT_SOURCE_DIR}/exceptions.cpp"
	"${PROJECT_SOURCE_DIR}/face-mask-filter.cpp"
	"${PROJECT_SOURCE_DIR}/plugin.cpp"
	"${PROJECT_SOURCE_DIR}/utils.cpp"
	"${TINYOBJDir}/tiny_obj_loader.cc"
	"${gs_SOURCES}"
	"${mask_SOURCES}"
	"${smll_SOURCES}"
)
SET(facemask-plugin_DATA
	"${PROJECT_SOURCE_DIR}/data/effects/color_conversion.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/default.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/denoise.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/merge.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/phong.effect"
	"${PROJECT_SOURCE_DIR}/data/locale/en-US.ini"
)
SET(facemask-plugin_LIBRARIES
	dlib::dlib
	${OpenCV_LIBS}
	avrt.lib
	Shlwapi.lib
)

INCLUDE_DIRECTORIES(
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}"
	"${PATH_OBS_STUDIO}/"
)

source_group("Data Files" FILES ${facemask-plugin_DATA})
source_group("Plugin\\Headers" FILES ${facemask-plugin_HEADERS})
source_group("Plugin\\Sources" FILES ${facemask-plugin_SOURCES})
source_group("SMLL\\Headers" FILES ${smll_HEADERS})
source_group("SMLL\\Sources" FILES ${smll_SOURCES})
source_group("GS\\Headers" FILES ${gs_HEADERS})
source_group("GS\\Sources" FILES ${gs_SOURCES})
source_group("Mask\\Headers" FILES ${mask_HEADERS})
source_group("Mask\\Sources" FILES ${mask_SOURCES})

# Compiler Configuration
add_definitions(-D_CRT_SECURE_NO_WARNINGS) # Hide Microsofts insecurities
## All Warnings, Extra Warnings, Pedantic
if (MSVC)
	if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
		string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
	endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

################################################################################
# Building
################################################################################
ADD_LIBRARY(facemask-plugin MODULE
	${facemask-plugin_HEADERS}
	${facemask-plugin_SOURCES}
	${facemask-plugin_DATA}
)
TARGET_LINK_LIBRARIES(facemask-plugin
	${LIBOBS_LIBRARIES}
	${facemask-plugin_LIBRARIES}
)

################################################################################
# Installation / Distribution
################################################################################


add_custom_command(TARGET facemask-plugin POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${PROJECT_SOURCE_DIR}/data"
	"${DISTRIBUTE_PATH}/$<CONFIG>/data/obs-plugins/facemask-plugin"
)
add_custom_command(TARGET facemask-plugin POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"$<TARGET_FILE:facemask-plugin>"
	"${DISTRIBUTE_PATH}/$<CONFIG>/${DISTRIBUTE_PATH_BIN}/$<TARGET_FILE_NAME:facemask-plugin>"
)
add_custom_command(TARGET facemask-plugin POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy
	"$<TARGET_FILE_DIR:facemask-plugin>/facemask-plugin.pdb"
	"${DISTRIBUTE_PATH}/$<CONFIG>/${DISTRIBUTE_PATH_BIN}/facemask-plugin.pdb"
)
