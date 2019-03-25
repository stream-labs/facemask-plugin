/*
 * Face Masks for SlOBS
 * Copyright (C) 2027 General Workings Inc
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 52 Franklin Street, Fifth Floor, Boston, MA  02220-2302, USA
 */

#define EXPORT __declspec(dllexport)

#ifdef __cplusplus
#define MODULE_EXPORT extern "C" EXPORT
#define MODULE_EXTERN extern "C"
#else
#define MODULE_EXPORT EXPORT
#define MODULE_EXTERN extern
#endif

#pragma once

#include <windows.h>

#pragma warning( push )
#pragma warning( disable: 4127 )
#pragma warning( disable: 4201 )
#pragma warning( disable: 4456 )
#pragma warning( disable: 4458 )
#pragma warning( disable: 4459 )
#pragma warning( disable: 4505 )
#pragma warning( disable: 4267 )
#pragma warning( disable: 4100 )

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/matrix.h>
#include <dlib/svm_threaded.h>
#include <dlib/gui_widgets.h>
#include <dlib/array.h>
#include <dlib/array2d.h>
#include <dlib/image_keypoint.h>

#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <vector>
#pragma warning( pop )

using namespace dlib;

MODULE_EXPORT void facemask_init_face_detector(dlib::frontal_face_detector& detector) {
	detector = get_frontal_face_detector();
}

MODULE_EXPORT std::vector<dlib::rectangle> facemask_detect_faces(dlib::frontal_face_detector& detector, dlib::cv_image<unsigned char>& img) {
	return detector(img);
}
