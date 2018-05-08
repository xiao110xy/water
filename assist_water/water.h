#pragma once
#include <iostream>
#include <algorithm>
#include <vector> 
#include <string>
#include <list>
#include <fstream> 
#include <iomanip>
#include <io.h>
struct water_result {
	Mat data;
	vector<float> parrallel_lines;
	vector<float> water_lines;
	vector<vector<float>> points;
	vector<vector<float>> number;
	int water_line;
	float water_number;
};
struct assist_information {
	Mat base_image;
	vector<double> roi;
	vector<vector<double>> point;
	int length;
};
// 读入辅助信息
vector<string> getFiles(string folder, string firstname, string lastname);
bool input_template(string file_name, vector<Mat> &template_image);
bool input_assist(string file_name, vector<assist_information> &assist_file, vector<Mat> template_image);
// 处理主函数
void compute_water_area(Mat im, vector<assist_information> assist_files);
// 几何校正原始影像
Mat GeoCorrect2Poly(assist_information assist_file,bool flag);
double compute_rms(vector<vector<double>> point, Mat r);
// 水位线识别
// 结果保存
// 功能函数
vector<vector<double>> swap_point(vector<vector<double>> &data);
Mat vector2Mat(vector<vector<double>> data);
vector<vector<double>> Mat2vector(Mat data);