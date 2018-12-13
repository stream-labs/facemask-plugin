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

using namespace cv;
using namespace dlib;

void print_usage() {
    std::cout << "Usage:" << std::endl;
    std::cout << "./face_landmark_detection [path/to/shape_predictor_68_face_landmarks.dat]" << std::endl;
}

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

// Shows the results that combined with Optical Flow, Kalman Filter and Dlib based on the speed of face movement
int main(int argc, char** argv) {
    try {
        if (argc > 2) {
            print_usage();
            return 0;
        }

        std::cout << "Press q to exit." << std::endl;

        VideoCapture cap(0);
		sleep(5000);
        if (!cap.isOpened()) {
            std::cerr << "Unable to connect to camera" << std::endl;
            return 1;
       }

        // Load face detection and pose estimation models.
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;

        if (argc == 2) {
            deserialize(argv[1]) >> pose_model;
        } else {
            deserialize("C:/Users/glaba/Documents/dev/facemask-plugin/data/shape_predictor_68_face_landmarks.dat") >> pose_model;
        }

        // Initialize the points of last frame
        std::vector<cv::Point2f> last_object;
        for (int i = 0; i < 68; ++i) {
            last_object.push_back(cv::Point2f(0.0, 0.0));
        }

        double scaling = 0.5;
        int flag = -1;
        int count = 0;
        bool redetected = true;
        // std::vector<dlib::rectangle> faces;

        // Initialize measurement points
        std::vector<cv::Point2f> kalman_points;
        for (int i = 0; i < 68; i++) {
            kalman_points.push_back(cv::Point2f(0.0, 0.0));
        }

        // Initialize prediction points
        std::vector<cv::Point2f> predict_points;
        for (int i = 0; i < 68; i++) {
            predict_points.push_back(cv::Point2f(0.0, 0.0));
        }

        // Kalman Filter Setup (68 Points Test)
        const int stateNum = 272;
        const int measureNum = 136;

        KalmanFilter KF(stateNum, measureNum, 0);
        Mat state(stateNum, 1, CV_32FC1);
        Mat processNoise(stateNum, 1, CV_32F);
        Mat measurement = Mat::zeros(measureNum, 1, CV_32F);

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

        // Initialize Optical Flow
        cv::Mat prevgray, gray;
        std::vector<cv::Point2f> prevTrackPts;
        std::vector<cv::Point2f> nextTrackPts;
        for (int i = 0; i < 68; i++) {
            prevTrackPts.push_back(cv::Point2f(0, 0));
        }

        // Grab and process frames until the main window is closed by the user.
        while(true) {
            // Grab a frame
            cv::Mat raw;
            cap >> raw;
            
            // Resize
            cv::Mat tmp;
            cv::resize(raw, tmp, cv::Size(), scaling, scaling);

            //Flip
            cv::Mat temp;
            cv::flip(tmp, temp, 1);

            // Turn OpenCV's Mat into something dlib can deal with.  Note that this just
            // wraps the Mat object, it doesn't copy anything.  So cimg is only valid as
            // long as temp is valid.  Also don't do anything to temp that would cause it
            // to reallocate the memory which stores the image as that will make cimg
            // contain dangling pointers.  This basically means you shouldn't modify temp
            // while using cimg.
            cv_image<bgr_pixel> cimg(temp);

            // Detect faces, load the vertexes as vector 
            // if (redetected == true || faces.size() == 0) {
            //     faces = detector(cimg);
            // }
            std::vector<dlib::rectangle> faces = detector(cimg);

            // Find the pose of each face.
            std::vector<full_object_detection> shapes;
            for (unsigned long i = 0; i < faces.size(); ++i) {
                shapes.push_back(pose_model(cimg, faces[i]));
            }

            // We cannot modify temp so we clone a new one
            cv::Mat face = temp.clone();
            cv::Mat face_2 = temp.clone();
            cv::Mat face_3 = temp.clone();
            cv::Mat frame = temp.clone();
            cv::Mat face_5 = temp.clone();

            // This function is to combine the optical flow + kalman filter + dlib to deteck and track the facial landmark
            if (flag == -1) {
                cvtColor(frame, prevgray, COLOR_BGR2GRAY);
                const full_object_detection& d = shapes[0];
                for (int i = 0; i < d.num_parts(); i++) {
                    prevTrackPts[i].x = d.part(i).x();
                    prevTrackPts[i].y = d.part(i).y();
                }
                flag = 1; 
            }

            // Update Kalman Filter Points
            if (shapes.size() == 1) {
                const full_object_detection& d = shapes[0];
                for (int i = 0; i < d.num_parts(); i++) {
                    kalman_points[i].x = d.part(i).x();
                    kalman_points[i].y = d.part(i).y();
                }
            }

            // Kalman Prediction
            Mat prediction = KF.predict();
            // std::vector<cv::Point2f> predict_points;
            for (int i = 0; i < 68; i++) {
                predict_points[i].x = prediction.at<float>(i * 2);
                predict_points[i].y = prediction.at<float>(i * 2 + 1);
            }

            // Optical Flow + Kalman Filter + Dlib based on the speed of face movement
            if (shapes.size() == 1) {
                cvtColor(frame, gray, COLOR_BGR2GRAY);
                if (prevgray.data) {
                    std::vector<uchar> status;
                    std::vector<float> err;
                    calcOpticalFlowPyrLK(prevgray, gray, prevTrackPts, nextTrackPts, status, err);
                    std::cout << "variance:" <<cal_dist_diff(prevTrackPts, nextTrackPts) << std::endl;

                    double diff = cal_dist_diff(prevTrackPts, nextTrackPts);
                    if (diff > 1.0) { // the threshold value here depends on the system, camera specs, etc
                         // if the face is moving so fast, use dlib to detect the face
                        const full_object_detection& d = shapes[0];
                        std::cout<< "DLIB" << std::endl;
                        for (int i = 0; i < d.num_parts(); i++) {
                            cv::circle(face_5, cv::Point2f(d.part(i).x(), d.part(i).y()), 2, cv::Scalar(0, 0, 255), -1);
                            nextTrackPts[i].x = d.part(i).x();
                            nextTrackPts[i].y = d.part(i).y();
                        }
                    } else if (diff <= 1.0 && diff > 0.005){
                        // In this case, use Optical Flow
                        std::cout<< "Optical Flow" << std::endl;
                        for (int i = 0; i < nextTrackPts.size(); i++) {
                            cv::circle(face_5, nextTrackPts[i], 2, cv::Scalar(255, 0, 0), -1);
                        }
                    } else {
                        // In this case, use Kalman Filter
                        std::cout<< "Kalman Filter" << std::endl;
                        for (int i = 0; i < predict_points.size(); i++) {
                            cv::circle(face_5, predict_points[i], 2, cv::Scalar(0, 255, 0), -1);
                            nextTrackPts[i].x = predict_points[i].x;
                            nextTrackPts[i].y = predict_points[i].y;
                        }
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

            // Update Measurement
            for (int i = 0; i < 136; i++) {
                if (i % 2 == 0) {
                    measurement.at<float>(i) = (float)kalman_points[i / 2].x;
                } else {
                    measurement.at<float>(i) = (float)kalman_points[(i - 1) / 2].y;
                }
            }

            // Update the Measurement Matrix
            measurement += KF.measurementMatrix * state;
            KF.correct(measurement);

            cv::imshow("KF+OF+DLIB", face_5);

            char key = cv::waitKey(1);
            if (key == 'q') {
                break;
            }
        }
    } catch(serialization_error& e) {
        print_usage();
    } catch(std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
