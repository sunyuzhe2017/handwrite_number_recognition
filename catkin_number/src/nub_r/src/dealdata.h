#ifndef DEALDATA_H
#define DEALDATA_H
//#pragma once
#include <opencv2/opencv.hpp>
#include <sys/types.h>
#include <iostream>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <time.h>

using namespace std;
using namespace cv;

//样本数据的保存路径
string dir_path = "/home/sun/catkin_nr/src/nub_r/data1/sample/";

using namespace std;

namespace dealData
{
  void samplePath(vector<string> &sample_path, vector<int> &labels)
    /*在指定文件中每个样本文件中读取每个样本的路径并保存*/
  {
    DIR    *dir;
    struct    dirent    *ptr;

    for (int i = 0; i < 10; ++i)
    {
      char path[1024];
      sprintf(path, "%s%d/.", dir_path.c_str(), i);
      dir = opendir(path); ///open the dir
      //读取目录下的所有文件
      while ((ptr = readdir(dir)) != NULL) ///read the list of this dir
      {
        char tmp[1024];

        if (ptr->d_name != string(".") && ptr->d_name != string(".."))
        {
          sprintf(tmp, "%s%d/%s", dir_path.c_str(), i, ptr->d_name);
          sample_path.push_back(tmp);
          labels.push_back(i);
        }

      }

      closedir(dir);
    }
  }
  void  samplePath(string dirPath, vector<string> &sample_path)
    /*在指定样本文件中读取每个样本的路径并保存*/
  {
    DIR    *dir;
    struct    dirent    *ptr;

    char path[1024];
    sprintf(path, "%s.", dirPath.c_str());

    dir = opendir(path);

    while ((ptr = readdir(dir)) != NULL)
    {
      char tmp[1024];

      if (ptr->d_name != string(".") && ptr->d_name != string(".."))
      {
        sprintf(tmp, "%s%s", dir_path.c_str(), ptr->d_name);
        sample_path.push_back(tmp);
      }

    }

    closedir(dir);
  }

}
#endif // DEALDATA_H
