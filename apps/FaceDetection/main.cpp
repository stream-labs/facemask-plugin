#include <iostream>
#include <opencv2/opencv.hpp>
#include "../../smll/FaceLib/FaceDetector.hpp"
#include <future>


int main()
{
	cv::Mat frame;
	cv::VideoCapture camera(0);

	// Initialize DLIB Face detector
	FaceLib::FaceDetector faceDetector;


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
			cv::rectangle(frame, face, cv::Scalar(255, 255, 255), 2);
		}

		cv::imshow("Image", frame);
		char k = cv::waitKey(1);
		if (k == 27) {
			break;
		}
	}
	std::cout << "DONE!" << std::endl;
	return 0;
}
