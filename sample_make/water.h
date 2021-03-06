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
#include "GeoMatch.h"
#define match_t 0.5
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
	int ruler_number=1;
	// 用于配准
	vector<KeyPoint> keypoints;
	Mat descriptors;
	// roi 
	Mat assist_image;
	vector<double> roi;
	vector<double> sub_roi;
	int roi_order = 1;
	// 摄像头抖动
	bool correct_flag = false;
	double correct_score = -1;
	vector<vector<double>> correct_point;
	//float correct_line;
	// 校正用点
	bool correct2poly = true;
	vector<vector<double>> point;
	Mat r;
	Mat r_inv;
	double rms_1, rms_2;
	Mat mask;
	//ref
	int ref_index;
	Mat ref_image;
	// left_right_water
	bool left_right_no_water;
	// 分割水位线
	Mat segment_result;
	Mat left_image;
	Mat right_image;
	Mat expand_wrap_image;
	vector<float> scores1;
	vector<float> scores2;
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
bool input_assist(Mat im,map<string, string> main_ini, vector<assist_information> &assist_files);
bool get_number(string line_string, vector<double> &temp);
bool input_assist_image(string file_name,assist_information &assist_file);
// 处理主函数
void compute_water_area(Mat im, vector<assist_information> &assist_files,string ref_name);
void opt_assist_files(vector<assist_information> &assist_files);
// 判断是白天还是黑夜
bool isgrayscale(Mat im);
// 判断是否是纯黑
bool isblack(Mat im, assist_information assist_file);
// 判断是否全是水
bool isblank(Mat im, assist_information &assist_file);
// 判断是否已经是底部了，白天；判断左右是否有水，晚上；
bool notall(Mat im, assist_information &assist_file);
// 判断water_line 是否可信
bool water_line_isok(int water_line, int all_length, int n_length);
// 夜晚是否过亮 并优化
bool isTooHighLightInNight(Mat im,int &water_line,int &gray_value);
// 对原始影像进行配准
bool correct_control_point(Mat im, assist_information &assist_file);
vector<assist_registration> xy_match(const Mat &image_1, const Mat &image_2, vector<vector<DMatch>> &dmatchs, vector<KeyPoint> keys_1,
	vector<KeyPoint> keys_2, string model,int  ruler_number);

void GetImageThreshold(Mat im, int &bestLowThresh, int& bestHighThresh);

// 几何校正原始影像
Mat correct_image(Mat im, assist_information &assist_file);
void map_coord(assist_information &assist_file,Mat &map_x, Mat &map_y,int base_x = 0, int base_y = 0);
Mat GeoCorrect2Poly(assist_information assist_file,bool flag);
Mat compute_point(Mat point, Mat r);
double compute_rms(Mat base_point, Mat wrap_point, Mat r);
// 水位线识别
bool get_label_mask(Mat mask,int &label, Mat &label_mask, double,int y_t);
float get_water_line_t2b(Mat im, double length,Mat& segment_result);
float match_template_score(Mat temp1, Mat temp2);
vector<float> process_score(vector<float> score, float score_t1, float score_t2);

// 白天水位线
float get_water_line_day(assist_information &assist_file, int water_line);
float get_water_line_day(Mat gc_im, assist_information &assist_file, int water_line, float scale = 0.4);
int get_mask_line(Mat mask,int n_length, float scale=0.4, int class_n=3);
int get_water_line(assist_information &assist_file);
Mat getBinMaskByMask(Mat mask);
int get_best_line(Mat mask, int x, int y);
Mat guidedFilter(Mat srcImage, Mat &guideImage, int radius, double eps);
Mat color_tansform(Mat data, Mat ref_image);
int optimization_water_line(assist_information &assist_file, float water_line, double m_SigmaS, double m_SigmaR);
// 夜晚水位线
float get_water_line_night(assist_information &assist_file);
bool left_right_water(Mat gc_im,int length);
int get_water_line_seg(Mat im, int length, int &line, float scale = 0.2);
// 结果保存
void save_file(Mat im, vector<assist_information> assist_files,map<string,string> main_ini);
// 功能函数
vector<vector<double>> part_row_point(vector<vector<double>> point,int r1,int r2);
vector<vector<double>> part_col_point(vector<vector<double>> point,int c1,int c2);
vector<vector<double>> swap_point(vector<vector<double>> &data);
Mat vector2Mat(vector<vector<double>> data);
vector<vector<double>> Mat2vector(Mat data);
int otsu(Mat im);


