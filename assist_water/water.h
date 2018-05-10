#pragma once
#include <iostream>
#include <algorithm>
#include <vector> 
#include <string>
#include <list>
#include <fstream> 
#include <iomanip>
#include <io.h>
#include "MeanShiftSegmentor.h"
struct assist_information {
	// 基本信息
	Mat base_image;
	Mat wrap_image;
	int length;
	int add_row;
	// roi
	vector<double> roi;
	// 校正用点
	vector<vector<double>> point;
	Mat r;
	Mat r_inv;
	double rms_1, rms_2;
	// 分割水位线
	Mat segment_result;
	vector<double> parrallel_lines;
	vector<double> water_lines;
	double water_number;
};
// 读入辅助信息
vector<string> getFiles(string folder, string firstname, string lastname);
bool input_template(string file_name, vector<Mat> &template_image);
bool input_assist(string file_name, vector<assist_information> &assist_files, vector<Mat> template_image);
// 处理主函数
void compute_water_area(Mat im, vector<assist_information> &assist_files);
// 几何校正原始影像
Mat correct_image(Mat im, assist_information &assist_file);
void map_coord(assist_information &assist_file, Mat &map_x, Mat &map_y);
Mat GeoCorrect2Poly(assist_information assist_file,bool flag);
Mat compute_point(Mat point, Mat r);
double compute_rms(Mat base_point, Mat wrap_point, Mat r);
// 水位线识别
void get_water_line(assist_information &assist_file);
bool get_label_mask(Mat mask,int &label, Mat &label_mask, assist_information assist_file);
// 结果保存
void save_file(Mat im, vector<assist_information> assist_files,string image_result, string para_result);
// 功能函数
vector<vector<double>> part_row_point(vector<vector<double>> point,int r1,int r2);
vector<vector<double>> part_col_point(vector<vector<double>> point,int c1,int c2);
vector<vector<double>> swap_point(vector<vector<double>> &data);
Mat vector2Mat(vector<vector<double>> data);
vector<vector<double>> Mat2vector(Mat data);