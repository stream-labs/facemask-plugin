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
#pragma once
#include <vector>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>

namespace FaceLib {
	/*
	A FaceLib class wrapped around DLIB+OpenCV to detect face(s)
	*/

	class FaceDetector
	{
	public:
		/**
		Constructor to initialize the FaceDetector object.
		Intializes the DLIB Frontal Face Detector.

		@usage FaceLib::FaceDetector detector()
		*/
		FaceDetector();

		/**
		Constructor to initialize the custom FaceDetector object.
		Intializes the DLIB Frontal Face Detector.

		@param filename The full path to the face detector file.
		@usage FaceLib::FaceDetector detector(<path_to_model>)
		*/
		FaceDetector(std::string filename);

		/**
		Destructor for the FaceDetector object.
		*/
		~FaceDetector();

		/**
		Returns all the faces in the image in std::vector<dlib::rectangle> format.

		@param image The image to detect faces.
		@param faces Fills current face detections in vector<dlib::rectangle>.
		@return None.
		*/
		void DetectFaces(cv::Mat& image, std::vector<dlib::rectangle>& faces);

	private:
		dlib::frontal_face_detector _detector;

		/**
		Function to initialize the DLIB's frontal face detector.
		*/
		void Init();

	};
}
