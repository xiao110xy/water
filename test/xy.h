#include "water_line.h"
#pragma once

Mat Edge_Detect(Mat im,int aperture_size);

Mat draw_part_E(Mat im, vector<vector<vector<Point3f>>> data);

vector<vector<vector<Point3f>>> get_pointline(Mat dx, Mat dy, vector<Point3f> points, bool flag, Mat flag_name);
vector<Point3f> cluster_point(Mat score_image, float score_t, Point point,Mat flag_image,int number);
vector<vector<Point3f>> cluster_point(Mat score_image, float score_t, int number, Point &index);
vector<vector<Point3f>> cluster_point(Mat score_image, float score_t, int number);
vector<Point3f> cluster_point(vector<Point3f> data, bool flag, Mat &flag_image);
vector<vector<Point3f>> new_point_line1_line2(Mat dx,Mat dy,vector<Point3f> point, vector<Point3f> line1_point, vector<Point3f> line2_point,bool flag);

vector<vector<vector<Point3f>>> select_point_line(vector<vector<vector<Point3f>>> &left_e, vector<vector<vector<Point3f>>> &right_e);

void get_E_area(Mat im, vector<vector<vector<Point3f>>> left_e, vector<vector<vector<Point3f>>> right_e, Mat dx, Mat dy);