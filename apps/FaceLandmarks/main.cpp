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

#include <iostream>
#include <opencv2/opencv.hpp>
#include "../../smll/FaceLib/FaceDetector.h"
#include "../../smll/FaceLib/FaceLandmarks.h"
#include <future>

void DrawPoseAxis(cv::Mat& frame, cv::Mat& R, cv::Point2d origin, float scale) {
	cv::Mat RotMat; cv::Rodrigues(R, RotMat);

	cv::Mat axis3D = (cv::Mat_<double>(3, 3) << 1, 0, 0, 0, -1, 0, 0, 0, -1);
	
	axis3D = axis3D * RotMat.t();
	
	cv::Mat axis2D = axis3D(cv::Rect(0, 0, 2, 3));
	cv::Point2d x = cv::Point2d(axis2D.at<double>(0, 0), axis2D.at<double>(0, 1));
	cv::Point2d y = cv::Point2d(axis2D.at<double>(1, 0), axis2D.at<double>(1, 1));
	cv::Point2d z = cv::Point2d(axis2D.at<double>(2, 0), axis2D.at<double>(2, 1));

	cv::line(frame, origin, x * scale + origin, cv::Scalar(255, 0, 0), 3);
	cv::line(frame, origin, y * scale + origin, cv::Scalar(0, 255, 0), 3);
	cv::line(frame, origin, z * scale + origin, cv::Scalar(0, 0, 255), 3);

	//// Convert radians to Angles
	//double angleX = R.at<double>(0, 0) * 180 / 3.14;
	//double angleY = R.at<double>(1, 0) * 180 / 3.14;
	//double angleZ = R.at<double>(2, 0) * 180 / 3.14;

	std::ostringstream out;
	out.str("");
	out << "X = " << std::fixed << std::setprecision(2) << R.at<double>(0, 0)
		<< ", Y = " << std::fixed << std::setprecision(2) << R.at<double>(1, 0)
		<< ", Z = " << std::fixed << std::setprecision(2) << R.at<double>(2, 0);
	cv::putText(frame, out.str(), cv::Point2f(0, 45), cv::FONT_HERSHEY_TRIPLEX, 0.5,
		cv::Scalar(255, 0, 0));
}

int main()
{
	cv::Mat frame;
	cv::VideoCapture camera(0);

	// Initialize DLIB Face detector
	FaceLib::FaceDetector faceDetector;
	// TODO: Replace the path here to the file on your system.
	FaceLib::FaceLandmarks landmarksDetector("C:/Users/srira/streamLabs/facemask-plugin/data/shape_predictor_68_face_landmarks.dat");

	int imageWriterCounter = 0;

	// landmarks2D
	std::vector<int> idxs2D = { 36, 45, 27, 28, 29, 30, 33 };

	// Get first image and compute K, D
	camera.read(frame);
	cv::Mat K = (cv::Mat_<double>(3, 3) << frame.cols, 0, frame.cols/2,
										  0, frame.cols, frame.rows/2,
										  0, 0, 1);
	cv::Mat D = cv::Mat::zeros(4, 1, CV_64F);
	cv::Mat R = cv::Mat::zeros(3, 1, CV_64F);
	cv::Mat t = cv::Mat::zeros(3, 1, CV_64F);

	while (true) {
		bool success = camera.read(frame);
		if (!success) break;

		// Image Pre-processing
		cv::Mat gray; cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
		cv::equalizeHist(gray, gray);
		std::vector<dlib::rectangle> faces;

		faceDetector.DetectFaces(gray, faces);

		// Draw faces on frame
		for (int i = 0; i < faces.size(); i++) {
			cv::Rect face = cv::Rect(cv::Point(faces[i].left(), faces[i].top()), cv::Point(faces[i].right(), faces[i].bottom()));
			std::vector<dlib::point> landmarks;
			landmarksDetector.DetectLandmarks(gray, faces[i], landmarks);

			// Draw Methods
			cv::rectangle(frame, face, cv::Scalar(255, 255, 255), 2);

			for (auto landmark : landmarks) {
				cv::circle(frame, cv::Point(landmark.x(), landmark.y()), 1, cv::Scalar(0, 0, 255), 1);
			}

			// Compute pose for each face
			std::vector<cv::Point2d> landmarks2D;
			landmarks2D.reserve(7);
			for (auto idx : idxs2D) {
				landmarks2D.emplace_back(landmarks[idx].x(), landmarks[idx].y());
			}
			
			landmarksDetector.DetectPose(landmarks2D, K, D, R, t);
			DrawPoseAxis(frame, R, cv::Point2d(100, 100), 30);
			
		}

		cv::imshow("Image", frame);
		char k = cv::waitKey(1);
		if (k == 27) {
			break;
		}
		if (k == 'c') {
			cv::imwrite("../output/Image_" + std::to_string(imageWriterCounter++) + ".png", frame);
		}

	}
	std::cout << "DONE!" << std::endl;
	return 0;
}