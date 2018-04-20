#pragma once
#include "water_line.h"

Mat Edge_Detect(Mat im,int aperture_size);
vector<vector<vector<Point3f>>> select_pointline(Mat dx, Mat dy, vector<Point3f> points, bool flag, Mat flag_name);
vector<Point3f> cluster_point(Mat score_image, float score_t, Point point,Mat flag_image,int number);
vector<vector<Point3f>> cluster_point(Mat score_image, float score_t, int number, Point &index);
vector<vector<Point3f>> cluster_point(Mat score_image, float score_t, int number);
vector<Point3f> cluster_point(vector<Point3f> data, bool flag, Mat &flag_image);
vector<vector<Point3f>> new_point_line1_line2(Mat dx,Mat dy,vector<Point3f> point, vector<Point3f> line1_point, vector<Point3f> line2_point,bool flag);