#ifndef HOGMAT_H
#define HOGMAT_H
#include "dealdata.h"

namespace hogMat {
  //输入是图像的路径
  Mat getHogMat(string path)
  {

    vector<float> descriptors;
    Mat trainData = imread(path.c_str(), CV_8UC1);

    HOGDescriptor *hog = new HOGDescriptor(Size(64, 96), Size(16, 16), Size(8, 8), Size(8, 8), 9);
    hog->compute(trainData, descriptors);

    int d_size = (int)descriptors.size();
    Mat hogMat(1, d_size, CV_32F);
    Mat test(trainData.size(), trainData.type());

    for (int index = 0; index < d_size; ++index)
      hogMat.ptr<float>(0)[index] = descriptors[index];

    return hogMat;
  }
  //输入是图像
  Mat gotHogMat(Mat src)
  {
    vector<float> descriptors;
    HOGDescriptor *hog = new HOGDescriptor(Size(64, 96), Size(16, 16), Size(8, 8), Size(8, 8), 9);
    hog->compute(src, descriptors);

    int d_size = (int)descriptors.size();
    Mat hogMat(1, d_size, CV_32F);

    for (int index = 0; index < d_size; ++index)
      hogMat.ptr<float>(0)[index] = descriptors[index];
    return hogMat;
  }
}
#endif // HOGMAT_H
