﻿#pragma once
#include <iostream>
#include <algorithm>
#include <vector> 
#include <string>
#include <list>
#include <fstream> 
#include <iomanip>
#include <math.h>
#include <io.h>
#include"sift.h"
#include"match.h"
#include "MeanShiftSegmentor.h"
struct assist_registration {
	int distance_to_left=9999;
	Mat match_line_image;
	Mat homography;
	vector<vector<double>> points;
	bool flag;
};
struct xy_feature {
	int x;
	int y;
	double dx;
	double dy;
};
struct assist_information {
	// 基本信息
	Mat base_image;
	Mat wrap_image;
	int length;
	int add_row;
	int ruler_number=1;
	// roi 
	Mat assist_image;
	vector<double> roi;
	int roi_order = 1;
	// 摄像头抖动
	bool correct_flag = false;
	bool match_flag = false;
	double correct_score = -1;
	vector<vector<double>> correct_point;
	//float correct_line;
	// 校正用点
	vector<vector<double>> point;
	Mat r;
	Mat r_inv;
	double rms_1, rms_2;
	//ref
	int ref_index;
	// 分割水位线
	Mat segment_result;
	vector<Point2d> parrallel_lines;
	vector<Point2d> parrallel_left;
	vector<Point2d> parrallel_right;
	vector<double> water_lines;
	double water_number;
	// 
	vector<vector<float>> scores;
};

// 读入辅助信息
vector<string> getFiles(string folder, string firstname, string lastname);
bool input_assist(Mat im,map<string, string> main_ini, vector<assist_information> &assist_files, vector<Mat> template_image);
bool get_number(string line_string, vector<double> &temp);
bool input_assist_image(string file_name,assist_information &assist_file);
// 处理主函数
int compute_water_area(Mat im, vector<assist_information> &assist_files,string ref_name);
// 判断是白天还是黑夜
bool isgrayscale(Mat im);
// 对原始影像进行配准
bool correct_control_point(Mat im, assist_information &assist_file);
vector<assist_registration> xy_match(const Mat &image_1, const Mat &image_2, vector<vector<DMatch>> &dmatchs, vector<KeyPoint> keys_1,
	vector<KeyPoint> keys_2, string model,assist_information  assist_file);
Mat edge_match(Mat im1, Mat im2);
Mat GetImageEdge(Mat im, Mat &result);
vector<xy_feature> CalGradientFeatures(Mat im, vector<Point> points);
Mat cluster_score_image(Mat score_image, float score_t, Point point, int number);
// 几何校正原始影像
Mat correct_image(Mat im, assist_information &assist_file);
void map_coord(assist_information &assist_file,Mat &map_x, Mat &map_y,int base_x = 0, int base_y = 0);
Mat GeoCorrect2Poly(assist_information assist_file,bool flag);
Mat compute_point(Mat point, Mat r);
double compute_rms(Mat base_point, Mat wrap_point, Mat r);
// 水位线识别
void get_water_line(assist_information &assist_file);//从上到下
bool get_label_mask(Mat mask,int &label, Mat &label_mask, double,int y_t);
float get_water_line_t2b(Mat im, double length,Mat& segment_result);
float match_template_score(Mat temp1, Mat temp2);
float get_water_line(Mat im, Mat ref_image, double length);
float optimization_water_line(assist_information &assist_file, float water_line, double m_SigmaS, double m_SigmaR);
vector<float> process_score(vector<float> score, float score_t1, float score_t2);
float get_water_line_seg(Mat im,Mat ref_image,double length);
// 结果保存
void save_file(Mat im, vector<assist_information> assist_files,map<string,string> main_ini);
// 功能函数
vector<vector<double>> part_row_point(vector<vector<double>> point,int r1,int r2);
vector<vector<double>> part_col_point(vector<vector<double>> point,int c1,int c2);
vector<vector<double>> swap_point(vector<vector<double>> &data);
Mat vector2Mat(vector<vector<double>> data);
vector<vector<double>> Mat2vector(Mat data);
// 水位线识别 采用之前的信息
double Edge_Detect(Mat im, int aperture_size);
Mat draw_part_E(Mat im, vector<vector<vector<Point3f>>> data);
vector<vector<vector<Point3f>>> get_pointline(Mat dx, Mat dy, vector<Point3f> points, bool flag, Mat flag_name);
vector<Point3f> cluster_point(Mat score_image, float score_t, Point point, Mat flag_image, int number);
vector<vector<Point3f>> cluster_point(Mat score_image, float score_t, int number, Point &index);
vector<vector<Point3f>> cluster_point(Mat score_image, float score_t, int number);
vector<Point3f> cluster_point(vector<Point3f> data, bool flag, Mat &flag_image);
vector<vector<Point3f>> new_point_line1_line2(Mat dx, Mat dy, vector<Point3f> point, vector<Point3f> line1_point, vector<Point3f> line2_point, bool flag);
vector<vector<vector<Point3f>>> select_point_line(vector<vector<vector<Point3f>>> &left_e, vector<vector<vector<Point3f>>> &right_e);
vector<Point3f> localmax_point_score(Mat score_image, int x1, int x2, float d_t, float scale);
vector<Point> get_line_point(Point2f point1, Point2f point2);