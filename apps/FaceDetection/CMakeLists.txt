cmake_minimum_required(VERSION 2.8.12)
PROJECT(FaceDetection)
math(EXPR BITS "8*${CMAKE_SIZEOF_VOID_P}")

SET(PATH_DLIB "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/dlib")
set(USE_AVX_INSTRUCTIONS ON CACHE BOOL "Use AVX instructions" FORCE)
set(USE_SSE4_INSTRUCTIONS ON CACHE BOOL "Use SSE4 instructions" FORCE)
set(DLIB_NO_GUI_SUPPORT OFF CACHE STRING "DLib GUI support" FORCE)
set(DLIB_GIF_SUPPORT OFF CACHE STRING "DLib GIF support" FORCE)
set(DLIB_JPEG_SUPPORT OFF CACHE STRING "DLib JPEG support"  FORCE)
set(DLIB_PNG_SUPPORT OFF CACHE STRING "DLib PNG support"  FORCE)

################################################################################
# Configuration
################################################################################

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

# OpenCV
SET(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty/opencv")
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

# FaceLib
include_directories("FaceLib")
SET(FaceLibDir "${CMAKE_CURRENT_SOURCE_DIR}/../../smll/FaceLib")

# Code
SET(FaceLib_HEADERS
	"${FaceLibDir}/FaceDetector.hpp"
)

SET(FaceDetection_HEADERS
	"${FaceLib_HEADERS}"
)

SET(FaceLib_SOURCES
	"${FaceLibDir}/FaceDetector.cpp"
)

SET(FaceDetection_SOURCES
	"main.cpp"
	"${FaceLib_SOURCES}"
)

SET(FaceDetection_LIBRARIES
	dlib::dlib
	${OpenCV_LIBS}
)


source_group("FaceDetection\\Headers" FILES ${FaceDetection_HEADERS})
source_group("FaceDetection\\Sources" FILES ${FaceDetection_SOURCES})
source_group("FaceLib\\Headers" FILES ${smll_HEADERS})
source_group("FaceLib\\Sources" FILES ${smll_SOURCES})


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
		libobs
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
else()
	install_obs_plugin_with_data(facemask-plugin data)
endif()
