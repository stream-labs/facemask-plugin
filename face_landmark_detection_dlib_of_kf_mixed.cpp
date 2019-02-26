 /*
landmarks in images read from a webcam and points that drew by the program.
 */
#include <dlib/opencv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <stdio.h>
#include <opencv2/video/tracking.hpp>
#include <smll/Face.hpp>
#include <smll/Config.hpp>
#include <smll/TestingPipe.hpp>
#include <smll/DetectionResults.hpp>



using namespace cv;
using namespace dlib;

// calculate the variance between the current points and last points
double cal_dist_diff(std::vector<cv::Point2f> curPoints, std::vector<cv::Point2f> lastPoints) {
    double variance = 0.0;
    double sum = 0.0;
    std::vector<double> diffs;
    if (curPoints.size() == lastPoints.size()) {
        for (int i = 0; i < curPoints.size(); i++) {
            double diff = std::sqrt(std::pow(curPoints[i].x - lastPoints[i].x, 2.0) + std::pow(curPoints[i].y - lastPoints[i].y, 2.0));
            sum += diff;
            diffs.push_back(diff);
        }
        double mean = sum / diffs.size();
        for (int i = 0; i < curPoints.size(); i++) {
            variance += std::pow(diffs[i] - mean, 2);
        }
        return variance / diffs.size();
    }
    return variance;
}



// Initialize prediction points
std::vector<cv::Point2f> predict_points;
// Kalman Filter Setup (68 Points Test)
const int stateNum = 4*7;
const int measureNum = 2*7;
frontal_face_detector detector;
shape_predictor pose_model;
double scaling = 0.5;
int flag = -1;
int count = 0;
bool redetected = true;
// Initialize measurement points
std::vector<cv::Point2f> kalman_points;
KalmanFilter KF;
Mat state; 			 
Mat processNoise; 
Mat measurement ;	
// Initialize Optical Flow
cv::Mat prevgray, gray;
std::vector<cv::Point2f> prevTrackPts;
std::vector<cv::Point2f> nextTrackPts;

VideoCapture cap;


		
std::vector<int> model_indices;
		
// Shows the results that combined with Optical Flow, Kalman Filter and Dlib based on the speed of face movement
cv::Mat landmark_tracking(cv::Mat &raw, smll::DetectionResults &res) {

		static bool inited = false;
		
		if(!inited) {

			model_indices.push_back(smll::LEFT_OUTER_EYE_CORNER);
			model_indices.push_back(smll::RIGHT_OUTER_EYE_CORNER);
			model_indices.push_back(smll::NOSE_1);
			model_indices.push_back(smll::NOSE_2);
			model_indices.push_back(smll::NOSE_3);
			model_indices.push_back(smll::NOSE_4);
			model_indices.push_back(smll:: NOSE_7);

	   
			inited = true;
			detector = get_frontal_face_detector();
			deserialize("C:/Users/glaba/Documents/dev/facemask-plugin/data/shape_predictor_68_face_landmarks.dat") >> pose_model;

			for (int i = 0; i < model_indices.size(); i++) {
				kalman_points.push_back(cv::Point2f(0.0, 0.0));
			}


			for (int i = 0; i < model_indices.size(); i++) {
				predict_points.push_back(cv::Point2f(0.0, 0.0));
			}



			KF = KalmanFilter(stateNum, measureNum, 0);
			state 			= Mat(stateNum, 1, CV_32FC1);
			processNoise 	= Mat(stateNum, 1, CV_32F);
			measurement 	= Mat::zeros(measureNum, 1, CV_32F);

			// Generate a matrix randomly
			randn(state, Scalar::all(0), Scalar::all(0.0));

			// Generate the Measurement Matrix
			KF.transitionMatrix = Mat::zeros(stateNum, stateNum, CV_32F);
			for (int i = 0; i < stateNum; i++) {
				for (int j = 0; j < stateNum; j++) {
					if (i == j || (j - measureNum) == i) {
						KF.transitionMatrix.at<float>(i, j) = 1.0;
					} else {
						KF.transitionMatrix.at<float>(i, j) = 0.0;
					}   
				}
			}

			//!< measurement matrix (H) Measurement Model  
			setIdentity(KF.measurementMatrix);
	  
			//!< process noise covariance matrix (Q)  
			setIdentity(KF.processNoiseCov, Scalar::all(1e-5));
			  
			//!< measurement noise covariance matrix (R)  
			setIdentity(KF.measurementNoiseCov, Scalar::all(1e-1));

			//!< priori error estimate covariance matrix (P'(k)): P'(k)=A*P(k-1)*At + Q)*/  A代表F: transitionMatrix  
			setIdentity(KF.errorCovPost, Scalar::all(1));
		
			randn(KF.statePost, Scalar::all(0), Scalar::all(0.0));


			for (int i = 0; i < model_indices.size(); i++) {
				prevTrackPts.push_back(cv::Point2f(0, 0));
			}
		}



		// Resize
		cv::Mat tmp;
		cv::resize(raw, tmp, cv::Size(), scaling, scaling);

		//Flip
		cv::Mat temp;
		temp = tmp;

		// Turn OpenCV's Mat into something dlib can deal with.  Note that this just
		// wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
		// long as temp is valid.  Also don't do anything to temp that would cause it
		// to reallocate the memory which stores the image as that will make cimg
		// contain dangling pointers.  This basically means you shouldn't modify temp
		// while using cimg.
		dlib::cv_image<bgr_pixel> cimg(temp);

		std::vector<dlib::rectangle> faces = detector(cimg);

		// Find the pose of each face.
		std::vector<full_object_detection> shapes;
		for (unsigned long i = 0; i < faces.size(); ++i) {
			shapes.push_back(pose_model(cimg, faces[i]));
		}
		// We cannot modify temp so we clone a new one

		cv::Mat frame = temp.clone();
		cv::Mat face_5 = temp.clone();

		// This function is to combine the optical flow + kalman filter + dlib to deteck and track the facial landmark
		if (flag == -1 && shapes.size() > 0) {
			cvtColor(frame, prevgray, COLOR_BGR2GRAY);
			const full_object_detection& d = shapes[0];
			for (int j = 0; j < model_indices.size(); j++) {
				int i = model_indices[j];
				prevTrackPts[j].x = d.part(i).x();
				prevTrackPts[j].y = d.part(i).y();
			}
			flag = 1; 
		}

		// Update Kalman Filter Points
/*		if (shapes.size() == 1) {
			const full_object_detection& d = shapes[0];
			for (int j = 0; j < model_indices.size(); j++) {
				int i = model_indices[j];
				kalman_points[j].x = d.part(i).x();
				kalman_points[j].y = d.part(i).y();
			}
		}

		// Kalman Prediction
		Mat prediction = KF.predict();
		for (int i = 0; i < model_indices.size(); i++) {
			predict_points[i].x = prediction.at<float>(i * 2);
			predict_points[i].y = prediction.at<float>(i * 2 + 1);
		}*/

		// Optical Flow + Kalman Filter + Dlib based on the speed of face movement
		if (shapes.size() == 1) {
			
			cvtColor(frame, gray, COLOR_BGR2GRAY);
			if (prevgray.data) {
				std::vector<uchar> status;
				std::vector<float> err;
				calcOpticalFlowPyrLK(prevgray, gray, prevTrackPts, nextTrackPts, status, err);

				double diff = cal_dist_diff(prevTrackPts, nextTrackPts);
				blog(LOG_DEBUG, "Var %f", diff);
				const full_object_detection& d = shapes[0];
				for (int i = 0; i < d.num_parts(); i++) {
					res[0].landmarks68[i] = point(2 * d.part(i).x(), 2 * d.part(i).y());
				}

				if (diff > 1.0) { 
					blog(LOG_DEBUG, "DLib");
					for (int j = 0; j < model_indices.size(); j++) {
						int i = model_indices[j];
						nextTrackPts[j].x = d.part(i).x();
						nextTrackPts[j].y = d.part(i).y();
						//cv::circle(face_5, cv::Point2f(d.part(j).x(), d.part(j).y()), 2, cv::Scalar(0, 0, 255), -1);
					}

					res.length = 1;
				}else if (diff <= 1.0 && diff > 0.005) {
					// In this case, use Optical Flow
					std::cout << "Optical Flow" << std::endl;
					blog(LOG_DEBUG, "OF");
					for (int j = 0; j < model_indices.size(); j++) {
						int i = model_indices[j];
						res[0].landmarks68[i] = point(2 * nextTrackPts[j].x, 2 * nextTrackPts[j].y);
						//cv::circle(face_5, nextTrackPts[j], 2, cv::Scalar(255, 0, 0), -1);
					}
					res.length = 1;
				} else {
					// In this case, use Kalman Filter
					std::cout<< "Kalman Filter" << std::endl;
					blog(LOG_DEBUG, "FK");
					for (int j = 0; j < model_indices.size(); j++) {
						//cv::circle(face_5, predict_points[j], 2, cv::Scalar(0, 255, 0), -1);
						int i = model_indices[j];
						nextTrackPts[j].x = prevTrackPts[j].x;
						nextTrackPts[j].y = prevTrackPts[j].y;

						res[0].landmarks68[i] = point(2 * nextTrackPts[j].x, 2 * nextTrackPts[j].y);
					}
					/*
					for (int j = 0; j < model_indices.size(); j++) {
						int i = model_indices[j];
						res[0].landmarks68[i] = point(2 * nextTrackPts[j].x, 2 * nextTrackPts[j].y);
						//cv::circle(face_5, nextTrackPts[j], 2, cv::Scalar(255, 0, 0), -1);
					}*/
					res.length = 1;
					redetected = false;
				}
			} else {
				redetected = true;
			}

			// previous points should be updated with the current points
			std::swap(prevTrackPts, nextTrackPts);
			std::swap(prevgray, gray);
		} else {
			redetected = true;
		}

	/*	// Update Measurement
		for (int i = 0; i < 2*7; i++) {
			if (i % 2 == 0) {
				measurement.at<float>(i) = (float)kalman_points[i / 2].x;
			} else {
				measurement.at<float>(i) = (float)kalman_points[(i - 1) / 2].y;
			}
		}

		// Update the Measurement Matrix
		measurement += KF.measurementMatrix * state;
		KF.correct(measurement);*/
		return face_5;
}
/*
int main(int argc, char** argv) {

	cap = VideoCapture(0);
	if (!cap.isOpened()) {
		std::cerr << "Unable to connect to caera" << std::endl;
		return 0;
	}
	smll::DetectionResults res;
	while(true) {

		cv::Mat r;
		cap >> r;
		r = landmark_tracking(r, res);
		char key = cv::waitKey(1);
		if (key == 'q') {
			break;
		}

		cv::imshow("KF+OF+DLIB", r);
	}
	return 0;
}
*/