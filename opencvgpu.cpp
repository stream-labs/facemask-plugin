#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <vector>
 
using namespace std;
using namespace cv;
using namespace cv::gpu;
 
void main()
{
   string cascadeName = "haarcascade_frontalface_alt.xml";
 
   CascadeClassifier_GPU cascade_gpu;
   VideoCapture capture(0);
   if(!capture.isOpened()) return;
 
   int gpuCnt = getCudaEnabledDeviceCount();   // gpuCnt >0 if CUDA device detected
   if(gpuCnt==0) return;  // no CUDA device found, quit
 
   if(!cascade_gpu.load(cascadeName))
     return;  // failed to load cascade file, quit
 
   Mat frame;
   long frmCnt = 0;
   double totalT = 0.0;
 
   while(true)
   {
      capture >> frame;   // grab current frame from the camera
      double t = (double)getTickCount();
 
      GpuMat faces;
      Mat frame_gray;
      cvtColor(frame, frame_gray, CV_BGR2GRAY);  // convert to gray image as face detection do NOT use color info
      GpuMat gray_gpu(frame_gray);  // copy the gray image to GPU memory
      equalizeHist(frame_gray,frame_gray);
 
      int detect_num = cascade_gpu.detectMultiScale(
      gray_gpu, faces,
          1.2, 4, Size(20, 20) );  // call face detection routine
      Mat obj_host;
      faces.colRange(0, detect_num).download(obj_host);  // retrieve results from GPU
 
      Rect* cfaces = obj_host.ptr<Rect>();  // results are now in "obj_host"
      t=((double)getTickCount()-t)/getTickFrequency();  // check how long did it take to detect face
      totalT += t;
      frmCnt++;
 
      for(int i=0;i<detect_num;++i)
      {
         Point pt1 = cfaces[i].tl();
         Size sz = cfaces[i].size();
         Point pt2(pt1.x+sz.width, pt1.y+sz.height);
         rectangle(frame, pt1, pt2, Scalar(255));
      }  // retrieve all detected faces and draw rectangles for visualization
      imshow("faces", frame);
      if(waitKey(10)==27) break;
   }
 
   cout << "fps: " << 1.0/(totalT/(double)frmCnt) << endl;
}