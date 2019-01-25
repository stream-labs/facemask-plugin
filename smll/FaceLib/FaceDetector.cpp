/*
* Face Masks for SlOBS
* smll - streamlabs machine learning library
*
* Copyright (C) 2017 General Workings Inc
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
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include "FaceDetector.h"

namespace FaceLib {
	FaceDetector::FaceDetector()
	{
		Init();
	}

	FaceDetector::FaceDetector(std::string filename) {
		dlib::deserialize(filename) >> _detector;
		dlib::test_box_overlap overlap(0.15, 0.75);
		_detector.set_overlap_tester(overlap);
	}

	FaceDetector::~FaceDetector()
	{
	}

	void FaceDetector::Init() {
		// Initialize the DLIB's Frontal Face Detector
		_detector = dlib::get_frontal_face_detector();
	}

	void FaceDetector::DetectFaces(cv::Mat& image, std::vector<dlib::rectangle>& faces) {
		// Convert cv::Mat to dlib::cv_image
		dlib::cv_image<unsigned char> imageDLIB(image);

		// Detect faces
		std::vector<dlib::rectangle> facesDLIB = _detector(imageDLIB);

		// Swap with the faces
		faces.swap(facesDLIB);
	}
}
