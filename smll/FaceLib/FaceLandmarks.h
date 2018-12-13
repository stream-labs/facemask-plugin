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

#include <dlib/image_processing.h>
#include <opencv2/opencv.hpp>

namespace FaceLib {
	class FaceLandmarks {
	public:
		FaceLandmarks();
		~FaceLandmarks();
		FaceLandmarks(char* filename);
		void Init(char* filename);
		void DetectLandmarks(cv::Mat& image, std::vector<cv::Point2d>& landmarks);

	private:
		dlib::shape_predictor _landmarksPredictor;
	};
}
