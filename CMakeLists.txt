cmake_minimum_required(VERSION 2.8.12)
PROJECT(facemask-plugin)
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(DownloadProject)

SET(PATH_DLIB "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/dlib")
set(USE_AVX_INSTRUCTIONS ON CACHE BOOL "Use AVX instructions" FORCE)
set(USE_SSE4_INSTRUCTIONS ON CACHE BOOL "Use SSE4 instructions" FORCE)
set(DLIB_NO_GUI_SUPPORT OFF CACHE STRING "DLib GUI support" FORCE)
set(DLIB_GIF_SUPPORT OFF CACHE STRING "DLib GIF support" FORCE)
set(DLIB_JPEG_SUPPORT OFF CACHE STRING "DLib JPEG support"  FORCE)
set(DLIB_PNG_SUPPORT OFF CACHE STRING "DLib PNG support"  FORCE)
set(DLIB_USE_BLAS OFF CACHE STRING "DLib BLAS support"  FORCE)
set(DLIB_USE_LAPACK OFF CACHE STRING "DLib LAPACK support"  FORCE)
set(DLIB_USE_MKL_FFT OFF CACHE STRING "DLib MKL FFT support"  FORCE)
set(DLIB_USE_MKL_SEQUENTIAL OFF CACHE STRING "DLib MKL Sequential support"  FORCE)
set(DLIB_USE_MKL_WITH_TBB OFF CACHE STRING "DLib MKL with TBB support"  FORCE)



SET(PATH_FREETYPE "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/freetype2")

if(TARGET libobs)
	set(FACEMASK_EXTERNAL FALSE)
else()
	set(FACEMASK_EXTERNAL TRUE)
endif()

set(FACEMASK_EXTERNAL FALSE)

################################################################################
# Configuration
################################################################################

string(
	CONCAT DEFAULT_LIBOBS_URL
	"https://s3-us-west-2.amazonaws.com/faces-internal.streamlabs.com/"
	"libobs-debug-22.0.3-sl.219-win32-x64-wdeps.7z"
)

set(
	LIBOBS_URL
	${DEFAULT_LIBOBS_URL}
	CACHE STRING "Location of libobs archive to base on"
)

download_project(
	PROJ libobs
	URL ${LIBOBS_URL}
	UPDATE_DISCONNECTED 1
)



# dlib
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

OPTION(BUILD_UNIT_TESTS "Build Unit tests for facemask plugin" OFF)

if(BUILD_UNIT_TESTS)
	# CPPUTEST
	SET(PATH_CPP_UTEST "" CACHE PATH "CPPUtest test framework")
	SET(EXTENSIONS OFF CACHE BOOL "Use the CppUTest extenstion library" FORCE)
	SET(TESTS OFF CACHE BOOL "Compile and make tests for the code?" FORCE)
	if(PATH_CPP_UTEST STREQUAL "")
		message(FATAL_ERROR "PATH_CPP_UTEST not set!")
		return()
	endif()
	if(NOT EXISTS "${PATH_CPP_UTEST}/include/CppUTest/MemoryLeakDetectorNewMacros.h")
		message(FATAL_ERROR "PATH_CPP_UTEST is invalid!")
		return()
	endif()

	add_subdirectory(${PATH_CPP_UTEST} ${PATH_CPP_UTEST})
	include_directories(${PATH_CPP_UTEST})
endif()

# OpenCV
SET(OpenCV_DIR "${PROJECT_SOURCE_DIR}/thirdparty/opencv")
SET(OpenCV_VERSION "400" CACHE STRING "Version of OpenCV to use.")
SET(OpenCV_LIBS
	debug "${OpenCV_DIR}/lib/Debug/opencv_world${OpenCV_VERSION}d.lib"
	optimized "${OpenCV_DIR}/lib/Release/opencv_world${OpenCV_VERSION}.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/zlibd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/zlib.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/libpngd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/libpng.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/ittnotifyd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/ittnotify.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/ippiwd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/ippiw.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/IlmImfd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/IlmImf.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/libjpeg-turbod.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/libjpeg-turbo.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/libtiffd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/libtiff.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/libjasperd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/libjasper.lib"
	debug "${OpenCV_DIR}/3rdparty/lib/Debug/libwebpd.lib"
	optimized "${OpenCV_DIR}/3rdparty/lib/Release/libwebp.lib"
	debug "${OpenCV_DIR}/3rdparty/ippicv/ippicv_win_${OpenCV_VERSION}/lib/intel64/ippicvmt.lib"
	optimized "${OpenCV_DIR}/3rdparty/ippicv/ippicv_win_${OpenCV_VERSION}/lib/intel64/ippicvmt.lib"
)
include_directories("${OpenCV_DIR}/include")

# zlib
include_directories("${OpenCV_DIR}/3rdparty/include")

# intel IPP
include_directories("${OpenCV_DIR}/3rdparty/ippicv/ippicv_win_${OpenCV_VERSION}/include")
include_directories("${OpenCV_DIR}/3rdparty/ippicv/ippiw_win_${OpenCV_VERSION}/include")

# smll
include_directories("smll")
SET(SMLLDir "${PROJECT_SOURCE_DIR}/smll")

# tinyobjloader
include_directories("thirdparty/tinyobjloader")
SET(TINYOBJDir "thirdparty/tinyobjloader")

# freetype
if(PATH_FREETYPE STREQUAL "")
	message(FATAL_ERROR "PATH_FREETYPE not set!")
	return()
endif()
if(NOT EXISTS "${PATH_FREETYPE}/src/base/ftsystem.c")
	message(FATAL_ERROR "PATH_FREETYPE is invalid!")
	return()
endif()
SET(freetype_SOURCES
	"${PATH_FREETYPE}/src/base/ftsystem.c"
    "${PATH_FREETYPE}/src/base/ftinit.c"
    "${PATH_FREETYPE}/src/base/ftdebug.c"
    "${PATH_FREETYPE}/src/base/ftbase.c"
    "${PATH_FREETYPE}/src/base/ftbbox.c"
    "${PATH_FREETYPE}/src/base/ftglyph.c"
    "${PATH_FREETYPE}/src/base/ftbitmap.c"
    "${PATH_FREETYPE}/src/truetype/truetype.c"
    "${PATH_FREETYPE}/src/smooth/smooth.c"
    "${PATH_FREETYPE}/src/base/ftstroke.c"
    "${PATH_FREETYPE}/src/sfnt/sfnt.c"
    "${PATH_FREETYPE}/src/psnames/psnames.c"
    "${PATH_FREETYPE}/src/autofit/autofit.c"
	)
add_definitions(-DFT2_BUILD_LIBRARY)
include_directories(${PATH_FREETYPE}/include)

# OBS Studio
if(FACEMASK_EXTERNAL)
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
#else()
#	SET(PATH_OBS_STUDIO ${CMAKE_SOURCE_DIR})
endif()



include("${libobs_SOURCE_DIR}/cmake/LibObs/LibObsConfig.cmake")

# Distribute format
OPTION(BUILD_FOR_PUBLIC "Build for public release" ON)
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
	"${SMLLDir}/Face.hpp"
	"${SMLLDir}/FaceDetector.hpp"
	"${SMLLDir}/ImageWrapper.hpp"
	"${SMLLDir}/landmarks.hpp"
	"${SMLLDir}/MorphData.hpp"
	"${SMLLDir}/OBSRenderer.hpp"
	"${SMLLDir}/sarray.hpp"
	"${SMLLDir}/TriangulationResult.hpp"
	"${SMLLDir}/TestingPipe.hpp"
)
SET(gs_HEADERS
	"${PROJECT_SOURCE_DIR}/gs/gs-effect.h"
	"${PROJECT_SOURCE_DIR}/gs/gs-vertex.h"
	"${PROJECT_SOURCE_DIR}/gs/gs-vertexbuffer.h"
	"${PROJECT_SOURCE_DIR}/gs/gs-indexbuffer.h"
	"${PROJECT_SOURCE_DIR}/gs/gs-rendertarget.h"
	"${PROJECT_SOURCE_DIR}/gs/gs-texture.h"
)
SET(mask_HEADERS
	"${PROJECT_SOURCE_DIR}/mask/mask.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-instance-data.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-animation.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-image.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-sequence.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-effect.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-light.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-material.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-mesh.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-model.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-morph.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-skinned-model.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-sound.h"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-emitter.h"
)
SET(facemask-plugin_HEADERS
	"${PROJECT_SOURCE_DIR}/plugin/base64.h"
	"${PROJECT_SOURCE_DIR}/plugin/exceptions.h"
	"${PROJECT_SOURCE_DIR}/plugin/face-mask-filter.h"
	"${PROJECT_SOURCE_DIR}/plugin/plugin.h"
	"${PROJECT_SOURCE_DIR}/plugin/strings.h"
	"${PROJECT_SOURCE_DIR}/plugin/utils.h"
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
	"${SMLLDir}/TestingPipe.cpp"
)
SET(gs_SOURCES
	"${PROJECT_SOURCE_DIR}/gs/gs-effect.cpp"
	"${PROJECT_SOURCE_DIR}/gs/gs-indexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/gs/gs-vertexbuffer.cpp"
	"${PROJECT_SOURCE_DIR}/gs/gs-rendertarget.cpp"
	"${PROJECT_SOURCE_DIR}/gs/gs-texture.cpp"
)
SET(mask_SOURCES
	"${PROJECT_SOURCE_DIR}/mask/mask.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-animation.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-image.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-sequence.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-effect.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-light.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-material.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-mesh.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-model.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-morph.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-skinned-model.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-sound.cpp"
	"${PROJECT_SOURCE_DIR}/mask/mask-resource-emitter.cpp"
)
SET(facemask-plugin_SOURCES
	"${PROJECT_SOURCE_DIR}/plugin/base64.cpp"
	"${PROJECT_SOURCE_DIR}/plugin/exceptions.cpp"
	"${PROJECT_SOURCE_DIR}/plugin/face-mask-filter.cpp"
	"${PROJECT_SOURCE_DIR}/plugin/plugin.cpp"
	"${PROJECT_SOURCE_DIR}/plugin/utils.cpp"
	"${TINYOBJDir}/tiny_obj_loader.cc"
	"${gs_SOURCES}"
	"${mask_SOURCES}"
	"${smll_SOURCES}"
	"${freetype_SOURCES}"
)
if(BUILD_UNIT_TESTS)
	SET(facemask-plugin_TEST_SOURCES
		"${PROJECT_SOURCE_DIR}/test/test.cpp"
		"${PROJECT_SOURCE_DIR}/test/test-utils.cpp"
		"${PROJECT_SOURCE_DIR}/test/test-image.cpp"
		"${PROJECT_SOURCE_DIR}/test/test-base64.cpp"
		"${PROJECT_SOURCE_DIR}/plugin/base64.cpp"
		"${PROJECT_SOURCE_DIR}/plugin/exceptions.cpp"
		"${PROJECT_SOURCE_DIR}/plugin/utils.cpp"
		"${SMLLDir}/ImageWrapper.cpp"
	)
endif()
SET(facemask-plugin_DATA
	"${PROJECT_SOURCE_DIR}/data/effects/color_alpha_tex.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/color_conversion.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/default.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/denoise.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/merge.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/phong.effect"
	"${PROJECT_SOURCE_DIR}/data/effects/aa.effect"
	"${PROJECT_SOURCE_DIR}/data/locale/en-US.ini"
)
SET(facemask-plugin_LIBRARIES
	dlib::dlib
	${LIBOBS_LIBRARIES}
	${OpenCV_LIBS}
	avrt.lib
	Shlwapi.lib
)



INCLUDE_DIRECTORIES(
	"${PROJECT_BINARY_DIR}"
	"${PROJECT_SOURCE_DIR}"
	"${LIBOBS_INCLUDE_DIRS}"
)

source_group("Data Files" FILES ${facemask-plugin_DATA})
source_group("Plugin\\Headers" FILES ${facemask-plugin_HEADERS})
source_group("Plugin\\Sources" FILES ${facemask-plugin_SOURCES})
source_group("SMLL\\Headers" FILES ${smll_HEADERS})
source_group("SMLL\\Sources" FILES ${smll_SOURCES})
source_group("FreeType" FILES ${freetype_SOURCES})
source_group("GS\\Headers" FILES ${gs_HEADERS})
source_group("GS\\Sources" FILES ${gs_SOURCES})
source_group("Mask\\Headers" FILES ${mask_HEADERS})
source_group("Mask\\Sources" FILES ${mask_SOURCES})

# Compiler Configuration
add_definitions(-D_CRT_SECURE_NO_WARNINGS) # Hide Microsofts insecurities
add_definitions(-DUNICODE -D_UNICODE)      # Use Unicode Charset
add_definitions(-D_DISABLE_EXTENDED_ALIGNED_STORAGE) # To supress aligned storage error from Visual Studio 15.8 
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

target_include_directories(facemask-plugin PUBLIC "${LIBOBS_INCLUDE_DIRS}")
file(MAKE_DIRECTORY "libobs")
file(MAKE_DIRECTORY "deps")
file(COPY "${LIBOBS_INCLUDE_DIRS}/" DESTINATION "libobs" FILES_MATCHING PATTERN "*")
file(COPY "${libobs_SOURCE_DIR}/deps/" DESTINATION "deps" FILES_MATCHING PATTERN "*")


if(FACEMASK_EXTERNAL)
	TARGET_LINK_LIBRARIES(facemask-plugin
		${LIBOBS_LIBRARIES}
		${facemask-plugin_LIBRARIES}
	)
	
	if(BUILD_UNIT_TESTS)
		add_executable(facemask-plugin-test
			${facemask-plugin_TEST_SOURCES}
		)
		TARGET_LINK_LIBRARIES(facemask-plugin-test
			${facemask-plugin_LIBRARIES} CppUTest
		)
	endif()	
else()
	TARGET_LINK_LIBRARIES(facemask-plugin
		${facemask-plugin_LIBRARIES}
	)
endif()

if(MSVC)
	target_compile_definitions(facemask-plugin
		PRIVATE _ENABLE_EXTENDED_ALIGNED_STORAGE
	)
endif()

################################################################################
# Installation / Distribution
################################################################################
if(FACEMASK_EXTERNAL)
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
#else()
#	install_obs_plugin_with_data(facemask-plugin data)
endif()
