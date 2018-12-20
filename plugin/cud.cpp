#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/ocl.hpp>
#include <vector>
#include <iostream>
#include <conio.h>
#include <windows.h>
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

using namespace std;
using namespace cv;
void main()
{
	VideoCapture vcap(0);
	CascadeClassifier fd("C:/Users/glaba/Documents/dev/streamlabs-obs/node_modules/obs-studio-node/libobs/obs-plugins/haarcascade_frontalface_alt.xml");
	UMat frame, frameGray;
	vector<Rect> faces;

	if (cv::ocl::haveOpenCL())
	{
		cv::ocl::setUseOpenCL(true);

		cv::ocl::Context mainContext =  cv::ocl::Context();

		if (!mainContext.create(cv::ocl::Device::TYPE_ALL))
		{
			cout << "Unable to create Integrated GPU OpenCL Context" << endl;
		}
		cout << mainContext.ndevices() << endl;
		for (int i = 0; i < mainContext.ndevices(); i++)
		{
			cv::ocl::Device device = mainContext.device(i);
			cout << "Device Name: %s"<<device.name().c_str()<< endl;
			cout << "Available: %i" << device.available()<< endl;
			cout << "imageSupport: %i" << device.imageSupport()<< endl;
			cout << "OpenCL_C_Version: %s" << device.OpenCL_C_Version().c_str()<< endl;
		}

		cv::ocl::Device(mainContext.device(0)); //Here is where you change which GPU to use (e.g. 0 or 1)
	}
	else {
		cout << "Nope" << endl;
	}

	for (;;) {
		// processing loop
		vcap >> frame;
		//cvtColor(frame, frameGray, CV_BGR2GRAY);
		//equalizeHist(frameGray, frameGray);
		fd.detectMultiScale(frame, faces);
		// draw rectangles …
		// show image …
		//Mat image_cpu = frame.getMat(cv::ACCESS_WRITE);
		//for(int i = 0; i < faces.size(); ++i)
		//   cv::rectangle(image_cpu, faces[i], Scalar(255));
		//if(waitKey(1)==27) break;
		//imshow("Faces", frame);
		//image_cpu.release();
		//dlib::sleep(20);
		char key = cv::waitKey(1);
		if (key == 'q') {
			break;
		}
	}
   /*VideoCapture vcap(0);
   if(!capture.isOpened()) return;
	while(true){
		//Ptr<cuda::CascadeClassifier> cascade_gpu = cuda::CascadeClassifier::create("");
		
		Mat image_cpu;
		capture >> image_cpu; 
		GpuMat image_gpu(image_cpu);
		
		GpuMat objbuf;
		//cascade_gpu->detectMultiScale(image_gpu, objbuf);
		//
		//std::vector<Rect> faces;
		//cascade_gpu->convert(objbuf, faces);
		//
		//for(int i = 0; i < faces.size(); ++i)
		//   cv::rectangle(image_cpu, faces[i], Scalar(255));
		//if(waitKey(1)==27) break;
		//imshow("Faces", image_cpu);
	}*/
}