#include <string>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

void test_detection(const string videoFile, void * detector) {
	VideoCapture cap(videoFile);

	if (!cap.isOpened()) {
		return -1;
	}

	//Mat frame;

	//imwrite("C:\Users\brank\kpi\detection_dataset_", frame);

}

void convert_video_into_imgs(const string videoFile) {
	int frame_count = 0;
	bool should_stop = false;

	VideoCapture cap(videoFile);

	while (!should_stop) 
	{
		cv::Mat frame;
		cap >> frame; //get a new frame from the video
		if (frame.empty())
		{
			should_stop = true; //we arrived to the end of the video
			continue;
		}

		char filename[128];
		sprintf(filename, "C:\\\Users\\brank\\kpi\\DanielQA\\frame_%06d.jpg", frame_count);
		cv::imwrite(filename, frame);
		frame_count++;
	}
}

int main() {
	convert_video_into_imgs("C:\\Users\\brank\\Downloads\\DanielQA.mp4")
}
