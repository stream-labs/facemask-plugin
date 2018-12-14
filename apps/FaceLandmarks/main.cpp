#include <iostream>
#include <opencv2/opencv.hpp>
#include "../../smll/FaceLib/FaceDetector.h"
#include "../../smll/FaceLib/FaceLandmarks.h"
#include <future>


int main()
{
	cv::Mat frame;
	cv::VideoCapture camera(0);

	// Initialize DLIB Face detector
	FaceLib::FaceDetector faceDetector;
	FaceLib::FaceLandmarks landmarksDetector("C:/Users/srira/streamLabs/facemask-plugin/data/shape_predictor_68_face_landmarks.dat");

	int imageWriterCounter = 0;

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
		}

		cv::imshow("Image", frame);
		char k = cv::waitKey(1);
		if (k == 27) {
			break;
		}
		if (k == 'c') {
			std::wcout << "True" << std::endl;
			cv::imwrite("../output/Image_" + std::to_string(imageWriterCounter++) + ".png", frame);
		}
	}
	std::cout << "DONE!" << std::endl;
	return 0;
}
