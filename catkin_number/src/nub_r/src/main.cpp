//created 2018.4.17
#include <ros/ros.h>
#include <stdio.h>
#include <iostream>
#include <sstream> // for converting the command line parameter to integer
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include "dealdata.h"
#include "hogmat.h"
using namespace cv;
using namespace std;
using namespace ml;

Mat trainImage;//用于存放训练图
Mat labelImage;//用于存放标签
int predict(Mat inputImage);
Mat deal_camera(Mat srcImage);
Ptr<KNearest> knn;
int result;
//load picture part added
vector<Mat> ROI;//用于存放图中抠出的数字区域
vector<Rect> ROIposition;//ROI在图像中的位置
vector<vector<Point> > contours;//点容器的容器，用于存放轮廓的点的容器的容器
vector<Vec4i> hierarchy;//点的指针容器
int result_2;//预测的结果
int weigth;//宽度
int height;//高度
Mat _roi;
Rect rect;
Point positiosn;
int brx; //右下角的横坐标
int bry; //右下角的竖坐标
int tlx; //左上角的横坐标
int tly; //左上角的竖坐标
int main(int argc, char** argv)
{   //open the camera
    if(argv[1] == NULL)
      {
          ROS_INFO("argv[1]=NULL\n");
          return 1;
      }
    istringstream video_sourceCmd(argv[1]);//local computer is 0,usb camera is 1!
    int video_source;
    if(!(video_sourceCmd >> video_source))
     {   ROS_INFO("video_sourceCmd is %d\n",video_source);
         return 1;
     }
     VideoCapture cap(video_source);
    if (!cap.isOpened())
    {   cout << "Failed to open camera." << endl;
        return -1;
    }
    //Train data!
    vector<string> samplePath;
    vector<int> labels;
    dealData::samplePath(samplePath, labels);
    //导入样本，并做好标签图
    for (int i = 0, _size = (int)samplePath.size(); i < _size; ++i)
    {
      Mat tmp = hogMat::getHogMat(samplePath[i]);
      trainImage.push_back(tmp);
      labelImage.push_back(labels[i]);
    }
    //创建KNN，并且设置N值为5
    knn = KNearest::create();
    knn->setDefaultK(5);
    knn->setIsClassifier(true);
    //生成训练数据
    Ptr<TrainData> tData = TrainData::create(trainImage, ROW_SAMPLE, labelImage);
    cout << "It's training!" << endl;
    knn->train(tData);
    //send picture
    for(;;)
    {   Mat frame;
        cap >> frame;
        frame = deal_camera(frame);
        imshow("original", frame);
        if(waitKey(30) >= 0)
        break;
    }
    return 0;
}
int predict(Mat inputImage)
{
  //预测函数
  if (trainImage.size == 0)
  {
    cout << "请先初始化" << endl;
    return -1;
  }

  Mat input = hogMat::gotHogMat(inputImage);
  return (int)knn->predict(input); //返回预测结果
}
Mat deal_camera(Mat srcImage)
{
  Mat dstImage, grayImage, Image;
  srcImage.copyTo(dstImage);
  //blur(dstImage, dstImage, Size(3, 3));
  //GaussianBlur(dstImage, dstImage, Size(3, 3), 0.5, 0.5);
  //medianBlur(dstImage, dstImage, 3);
  cvtColor(dstImage, grayImage, COLOR_BGR2GRAY);
  threshold(grayImage, Image, 120, 255, CV_THRESH_BINARY_INV);
  findContours(Image,contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
  //added for picture
  vector< vector<Point> >::iterator It;
  for (It = contours.begin(); It < contours.end(); It++)
  {   //画出可包围数字的最小矩
    rect = boundingRect(*It);
    weigth = rect.br().x - rect.tl().x;//宽
    height = rect.br().y - rect.tl().y;//高
    if ((weigth < height && height< 4*weigth)  && ((weigth > 10) || (height > 10)))
    {  //根据数字的特征排除掉一些可能不是数字的图形，然后进行一下处理
      Mat roi = Image(rect);
      roi.copyTo(_roi);//深拷贝出来
      ROI.push_back(_roi);//保存，方便操作
      ROIposition.push_back(rect);
      rectangle(srcImage, rect, Scalar(255, 255, 255), 1);
      if ((height * 2) < weigth)
        result = 1;
      else{
        Mat pre;
        resize(_roi, pre, Size(64,96));
        threshold(pre, pre, 120, 255, CV_THRESH_BINARY);
        result = predict(pre);
      }
      char output[10] = { 0 };
      sprintf(output, "%d", result);
      positiosn = Point(rect.br().x - 7, rect.br().y + 25);
      putText(srcImage,output,positiosn,1, 1.0,Scalar(0, 0, 0),1);//在屏幕上打印字
      cout<<result<<endl;
    }
  }
  return srcImage ;
}
