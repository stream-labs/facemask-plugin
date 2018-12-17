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

#include "FaceLandmarks.h"

namespace FaceLib {
	FaceLandmarks::FaceLandmarks() {
		
	}

	FaceLandmarks::~FaceLandmarks() {

	}

	FaceLandmarks::FaceLandmarks(char* filename) {
		Init(filename);
	}

	void FaceLandmarks::Init(char* filename) {
#ifdef _WIN32
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring wide_filename(converter.from_bytes(filename));

		/* DLIB will not accept a wifstream or widestring to construct
		 * an ifstream or wifstream itself. Here we use a non-standard
		 * constructor provided by Microsoft and then the direct
		 * serialization function with an ifstream. */
		std::ifstream predictor68_file(wide_filename.c_str(), std::ios::binary);

		if (!predictor68_file) {
			throw std::runtime_error("Failed to open predictor68 file");
		}

		dlib::deserialize(_landmarksPredictor, predictor68_file);
#else
		dlib::deserialize(filename) >> _landmarksPredictor;
#endif
		InitLandmarks3D();
	}

	FaceLandmarks::FaceLandmarks(std::string filename) {
		dlib::deserialize(filename) >> _landmarksPredictor;
		InitLandmarks3D();
	}

	void FaceLandmarks::DetectLandmarks(cv::Mat& image, dlib::rectangle& face, std::vector<dlib::point>& landmarks) {
		// Convert OpenCV image to DLIB style
		dlib::cv_image<unsigned char> imageDLIB(image);

		// Detect landmarks using the image and face rect
		dlib::full_object_detection predictedLandmarks = _landmarksPredictor(imageDLIB, face);

		std::vector<dlib::point> results;
		results.reserve(68); // 68 landmarks
		for (int j = 0; j < 68; j++) {
			results.emplace_back(predictedLandmarks.part(j).x(), predictedLandmarks.part(j).y());
		}

		landmarks.swap(results);
	}

	void FaceLandmarks::DetectPose(std::vector<cv::Point2f>& landmarks2D, std::vector<cv::Point3f>& landmarks3D, const cv::Mat& K, const cv::Mat& D, cv::Mat& R, cv::Mat& t, bool useExtrinsicGuess) {
		cv::solvePnP(landmarks3D, landmarks2D, K, D, R, t, useExtrinsicGuess, cv::SOLVEPNP_EPNP);

		if (t.at<double>(2, 0) < 0) {
			CorrectPoseFlip(R, t);
		}
	}

	void FaceLandmarks::DetectPose(std::vector<cv::Point2d>& landmarks2D, cv::Mat& K, cv::Mat& D, cv::Mat& R, cv::Mat& t) {
		cv::solvePnP(_landmarks3D, landmarks2D, K, D, R, t, false, cv::SOLVEPNP_EPNP);
		
		if (t.at<double>(2, 0) < 0) {
			CorrectPoseFlip(R, t);
		}
	}

	void FaceLandmarks::InitLandmarks3D() {
		// Our custom Maya landmarks
		_landmarks3D.clear();
		_landmarks3D.reserve(7);
		_landmarks3D.emplace_back(-2.379, -1.8, 2.442238431);
		_landmarks3D.emplace_back(2.379, -1.8, 2.442238431);
		_landmarks3D.emplace_back(0.0, -1.83012447, 1.2548274);
		_landmarks3D.emplace_back(0.0, -1.146743411, 0.7348212125);
		_landmarks3D.emplace_back(0.0f, -0.5915072192f, 0.3310879765f);
		_landmarks3D.emplace_back(0.0, -0.0, -0.0);
		_landmarks3D.emplace_back(0.0, 0.7203533372, 0.8644320921);
	}

	void FaceLandmarks::CorrectPoseFlip(cv::Mat& R, cv::Mat& t) {
		// TODO: Buggy!!!!
		// Convert Eulers to Quaternion
		double angle = sqrt(R.dot(R));
		double quatRot[4];
		if (angle < 0.0001) {
			quatRot[0] = 0.0;
			quatRot[1] = 1.0;
			quatRot[2] = 0.0;
			quatRot[3] = 0.0;
		}
		else {
			quatRot[0] = R.at<double>(0, 0) / angle;
			quatRot[1] = R.at<double>(1, 0) / angle;
			quatRot[2] = R.at<double>(2, 0) / angle;
			quatRot[3] = angle;
		}

		// Flip translation
		t = -t;

		// Flip z and angle
		quatRot[2] = -quatRot[2];
		quatRot[3] = -quatRot[3];

		// Flip 180 degrees around z
		// Simplified Rodrigues for rotation around Z
		double cos1 = cos(quatRot[3] / 2.0);
		double sin1 = sin(quatRot[3] / 2.0);
		double cos2 = cos(CV_PI / 2.0);
		double sin2 = sin(CV_PI / 2.0);

		double a = acos(cos1 * cos2 - sin1 * sin2 * quatRot[2]) * 2.0;
		double x = sin1 * cos2 * quatRot[0] + sin1 * sin2 * quatRot[1];
		double y = sin1 * cos2 * quatRot[1] + sin1 * sin2 * quatRot[0];
		double z = sin1 * cos2 * quatRot[2] + cos1 * sin2;

		double sina = sin(a / 2.0);
		if (sina < 0.0001)
			sina = 1.0;
		quatRot[0] = x / sina;
		quatRot[1] = y / sina;
		quatRot[2] = z / sina;
		quatRot[3] = a;

		// Replace the contents of R
		R.at<double>(0, 0) = quatRot[0] * quatRot[3];
		R.at<double>(1, 0) = quatRot[1] * quatRot[3];
		R.at<double>(2, 0) = quatRot[2] * quatRot[3];
	}

	//void FaceLandmarks::DetectLandmarks(cv::Mat& image, dlib::rectangle& face, std::vector<cv::Point2d>& landmarks) {
	//	//// Convert OpenCV image to DLIB style
	//	//dlib::cv_image<unsigned char> imageDLIB(image);

	//	//// Detect landmarks using the image and face rect
	//	//dlib::full_object_detection predictedLandmarks = _landmarksPredictor(image, face);

	//	//std::vector<dlib::point> results;
	//	//results.reserve(68); // 68 landmarks
	//	//for (int j = 0; j < 68; j++) {
	//	//	results.emplace_back(predictedLandmarks.part(j));
	//	//}

	//	//landmarks.swap(results);
	//}
}
