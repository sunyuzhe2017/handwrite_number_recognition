//created 2018.5.6 specilized for three numbers door number recognition
//last changes 2018..5.11 result:be able to recognize door number correctly
#include <ros/ros.h>
#include <stdio.h>
#include <iostream>
#include <sstream> // for converting the command line parameter to integer
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include "dealdata.h"
#include "hogmat.h"
#include <algorithm>
using namespace cv;
using namespace std;
using namespace ml;

Mat trainImage;//用于存放训练图
Mat labelImage;//用于存放标签
int predict(Mat inputImage);
Mat deal_camera(Mat srcImage);
Ptr<KNearest> knn;
int result;//predict result with knn
vector<vector<Point> > contours;//点容器的容器，用于存放轮廓的点的容器的容器
Point positiosn;

int main(int argc, char** argv)
{
  //open the camera
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
    }//
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
        //Mat grayImage, Image;
        //cvtColor(frame, grayImage, COLOR_BGR2GRAY);
        //threshold(grayImage, Image,150,255, CV_THRESH_BINARY);
        frame = deal_camera(frame);
        imshow("original", frame);
        if(waitKey(30) >= 0)
        break;
    }
  /*Mat srcImage = imread("/home/sun/num_pic/eg1.jpg");
  Mat res = deal_camera(srcImage);
  cv::namedWindow("result_image", CV_WINDOW_NORMAL);
  cv::imshow("result_image", res);
  cv::waitKey();*/
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
  Mat grayImage, Image;
  cvtColor(srcImage, grayImage, COLOR_BGR2GRAY);
  //threshold(grayImage, Image,120,255, CV_THRESH_BINARY);
  threshold(grayImage, Image, 0, 255, CV_THRESH_OTSU);
  findContours(Image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
  if (contours.size()!=0)
  {
    //cout<<"find contours first time !"<<contours.size()<<endl;
    //Mat src_bin_tar_contours(Image.size(), CV_8U, cv::Scalar(0));
    //drawContours(srcImage, contours, -1, cv::Scalar(255), 3);

    //ROS_INFO("I'm running...!");
    // 查找外层轮廓中 与 矩形最相近的最大轮廓
    double mymax = 0;
    int max_id = 0;
    for (int i = 0; i != contours.size(); i++){
      double area = cv::contourArea(contours[i]);
      double area_rect = minAreaRect(contours[i]).boundingRect().area();
      double tmp = area / area_rect;
      //cout<<"tmp"<<i<<"is"<<tmp<<endl;
      if (mymax < tmp){
        mymax = tmp;
        max_id = i;
      }
    }
    //cout<<"max_id :"<<max_id<<endl;
    //cv::Mat src_bin_tar_contours_max(Image.size(), CV_8U, cv::Scalar(0));
    //drawContours(srcImage, contours, max_id, cv::Scalar(0,0,255), 3);
    //drawContours(srcImage, contours, max_id, cv::Scalar(0,0,255), 3);
   // cout<<max_id<<endl;

      cv::Rect roi_rect = minAreaRect(contours[max_id]).boundingRect();
    try{
        grayImage = grayImage(roi_rect);
    }
    catch(...){
      cout<<"Wrong roi!"<<endl;
    }
    //cv::Rect roi_rect = minAreaRect(contours[max_id]);//.boundingRect();
    //grayImage = grayImage(roi_rect);//get grey image roi

    threshold(grayImage, Image, 0, 255, CV_THRESH_OTSU | CV_THRESH_BINARY_INV);

    int size = 1;//much better than original 5
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(size, size));
    cv::erode(Image, Image, kernel);
    cv::dilate(Image, Image, kernel);

    // 寻找连通域
    vector<vector<Point> > contours_2;
    findContours(Image, contours_2, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
    if (contours_2.size()>=3)
    {
      // 寻找字符
      //cout<<"find contours second time!:"<<contours_2.size()<<endl;
     // cout<<"Image cols:"<<Image.cols<<"Image rows:"<<Image.rows<<endl;
      std::vector<cv::Rect> vec;
      for (int i = 0; i != contours_2.size(); i++){
        cv::Rect roi = boundingRect(contours_2[i]);

        if (roi.width < Image.cols / 8.0 || roi.width >Image.cols / 3.0)
          continue;
        if (roi.height > 0.6 * Image.rows || roi.height < 0.2 * Image.rows)
          continue;
        //cout<<vec.size()<<endl;
        vec.push_back(roi);
      }
      //cout<<"effective number:"<<vec.size()<<endl;
      if (vec.size() == 3){
        sort(vec.begin(), vec.end(), [](const cv::Rect & a, const cv::Rect & b){return a.x < b.x; });
        vector<Mat> nums;//number pic
        vector<int> num;//int number
        for (auto item : vec){
          //cout<<nums.size()<<endl;
          nums.push_back(Image(item));}
        for (auto item : nums){
          Mat pre;
          resize(item, pre, Size(64,96));
          result = predict(pre);
          //cout<<num.size()<<endl;
          num.push_back(result);
        }
        std::string res;
        std::for_each(num.begin(), num.end(), [&](int data){ res += std::to_string(data); });
        rectangle(srcImage,roi_rect, Scalar(0, 0, 255), 3);
        positiosn = Point(roi_rect.br().x - 7, roi_rect.br().y + 20);
        putText(srcImage,res,positiosn,1, 3.0,Scalar(0, 255, 255),2);//在屏幕上打印字
        cout<<"Recognization result is :"<<res<<endl;
      }
    }
    }
  return srcImage;
}
