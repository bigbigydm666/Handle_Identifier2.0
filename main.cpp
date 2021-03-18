

/*
 * @Author: your name
 * @Date: 2020-12-15 17:01:23
 * @LastEditTime: 2021-03-18 19:14:24
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /RSDRIVER/main.cpp
 */

#include <opencv2/opencv.hpp>
#include <librealsense2/rs.hpp>
#include <librealsense2/rsutil.h>
#include <iostream>
#include "Realsense.hpp"
using namespace cv;
using namespace std;
using namespace rs2;
#define ON_DR 1
#define OFF_DR 2
#define MODE OFF_DR


Mat resetImg(Mat src)
{
	Mat dst;
	dst.create( src.size(), src.type());
	Mat map_x;
	Mat map_y;
	map_x.create( src.size(), CV_32FC1);
	map_y.create( src.size(), CV_32FC1);
	for( int i = 0; i < src.rows; ++i)
	{
		for( int j = 0; j < src.cols; ++j)
		{
			map_x.at<float>(i, j) = (float) (src.cols - j) ;
			map_y.at<float>(i, j) = (float) (src.rows - i) ;
		}
	}
	remap(src, dst, map_x, map_y, CV_INTER_LINEAR);
   return dst;

}

Realsense d435;
int main()
{
   
   d435.init();
   vector<Mat> src;
   Mat color_img;
   Mat depth_img;
   Mat result_img;
   //d435.setSize(640, 480);
   //d435.setSensor(1);
   Mat dst;
    namedWindow("control", 1);

   int hMax = 110, hMin = 0, sMax = 95, sMin = 0, vMax = 100, vMin = 0;

   cv::createTrackbar("hMax", "control", &hMax, 255);
   cv::createTrackbar("hMin", "control", &hMin, 255);
   cv::createTrackbar("sMax", "control", &sMax, 255);
   cv::createTrackbar("sMin", "control", &sMin, 255);
   cv::createTrackbar("vMax", "control", &vMax, 255);
   cv::createTrackbar("vMin", "control", &vMin, 255);

   Mat temp = imread("xuxieban.png", IMREAD_GRAYSCALE);
   vector<Mat> temp_contours; 
   temp_contours.push_back(temp);
   while(1)
   {
      src = d435.getimg();  
      if(MODE == ON_DR)
      {
         color_img = resetImg(src[0]);
         depth_img = resetImg(src[1]);
      }
      else
      {
         color_img = src[0];
         depth_img = src[1];
      }
      //color_img = resetImg(color_img);
      Mat mask;
      //threshold(depth_img, mask, 1000, 1, THRESH_BINARY);
      //cout << depth_img << endl;
      //cvtColor(mask, mask, COLOR_GRAY2BGR);
      //multiply(color_img, mask, dst);
      Mat color_img_hsv;
      Mat threshed_hsv;
      Mat color_temp;
      Mat gray_temp;
      Mat threshed_img;
      Mat result_img;
      medianBlur(color_img, color_img,5);
      //GaussianBlur(color_img, color_img, Size(11, 11), 0);
      cvtColor(color_img, color_img_hsv, COLOR_BGR2HSV);
      inRange(color_img_hsv, Scalar(hMin, sMin, vMin), Scalar(hMax, sMax, vMax), threshed_hsv);
      Mat kernel1 = getStructuringElement(MORPH_RECT, Size(9, 9));
      morphologyEx(threshed_hsv, threshed_hsv, MORPH_CLOSE, kernel1);
      cvtColor(threshed_hsv, mask, COLOR_GRAY2BGR);
      threshold(mask, mask, 1, 1, THRESH_BINARY);
      imshow("444", threshed_hsv);
      multiply(depth_img, mask, color_temp);
      medianBlur(color_temp, color_temp,5);
      cvtColor(color_temp, gray_temp, COLOR_BGR2GRAY);
      threshold(gray_temp, threshed_img, 20, 255, THRESH_BINARY);
      Mat kernel2 = getStructuringElement(MORPH_RECT, Size(3, 15));
      Mat kernel3 = getStructuringElement(MORPH_RECT, Size(7, 3));
      Mat kernel4 = getStructuringElement(MORPH_RECT, Size(3, 3));
      morphologyEx(threshed_img, result_img, MORPH_OPEN, kernel2);
      morphologyEx(result_img, result_img, MORPH_OPEN, kernel3);
      morphologyEx(result_img, result_img, MORPH_OPEN, kernel4);
      vector<vector<Point>> contours;
      findContours(result_img, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
      if(!contours.empty())
      {
         drawContours(color_img, contours, -1, Scalar(0, 255, 0), 2);
      }
      double pro;
      double pro_thresh = 2.5;
      vector<RotatedRect> selectedRects;

      //开始筛选

      for(int i = 0; i < contours.size(); i++)
      {
         if(contours[i].size() < 150 || contours[i].size() > 600)
         {
            continue;
         }
         pro = matchShapes(contours[i], temp_contours[0], CONTOURS_MATCH_I1, 0);
         if(pro > pro_thresh)
         {
            continue;
         }
         //Rect temp_Rect;
         RotatedRect min_Rect = minAreaRect(contours[i]);
         //temp_Rect = boundingRect(contours[i]);
         Point2f vertex[4];
         min_Rect.points(vertex);
         for (int i = 0; i < 4; i++)
         {
            line(color_img, vertex[i], vertex[(i + 1) % 4], cv::Scalar(255, 100, 200),4);
         }
         if(min_Rect.angle > -45)
         {
            if(min_Rect.size.height/min_Rect.size.width < 2.5)
            {
               continue;
            }
         }
         else
         {
            if(min_Rect.size.width/min_Rect.size.height < 2.5)
            {
               continue;
            }
         }
         selectedRects.push_back(min_Rect);
      }

      for(int i = 0; i < selectedRects.size(); i++)
      {
         Point2f vertex[4];
         selectedRects[i].points(vertex);
         for (int j = 0; j < 4; j++)
         {
            line(color_img, vertex[j], vertex[(j + 1) % 4], cv::Scalar(255, 100, 200),4);
         }
      }


      
      imshow("000", gray_temp);
      imshow("666", color_img);
      //cvtColor(src[2], dst, COLOR_BGR2GRAY);
      imshow("555", color_temp);
      //vector<vector<Point>> contours;
      //findContours(dst, contours, RETR_LIST, CHAIN_APPROX_NONE);
      //if(!contours.empty())
      //{
         //drawContours(src[0], contours, -1, Scalar(255, 255, 255), 2);
      //}
      imshow("777", depth_img);
      imshow("222", result_img);
      selectedRects.clear();
      contours.clear();
      waitKey(1);
   }
   return 0;
}