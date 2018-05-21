#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <ros/ros.h>


using namespace cv;
using namespace std;
#include <sstream> // for converting the command line parameter to integer

int main(int argc, char** argv)
{
  // Check if video source has been passed as a parameter
    if(argv[1] == NULL)
      {
          ROS_INFO("argv[1]=NULL\n");
          return 1;
      }
    istringstream video_sourceCmd(argv[1]);
    int video_source;
     // Check if it is indeed a number
     if(!(video_sourceCmd >> video_source))
     {
         ROS_INFO("video_sourceCmd is %d\n",video_source);
         return 1;
     }
     VideoCapture cap(video_source);
    //VideoCapture cap("nvcamerasrc ! video/x-raw(memory:NVMM), width=(int)1280, height=(int)720,format=(string)I420, framerate=(fraction)24/1 ! nvvidconv flip-method=2 ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink");
    if (!cap.isOpened())
    {
        cout << "Failed to open camera." << endl;
        return -1;
    }

    for(;;)
    {
        Mat frame;
        Mat frame_id;
        cap >> frame;
        cvtColor(frame,frame_id, CV_RGB2GRAY);   //convert RGB to GRAY
        //dealImage::cut_ROI(frame_id,frame);
        imshow("original", frame_id);
        //waitKey(1);
        if(waitKey(30) >= 0)
        break;
    }
    return 0;
}
