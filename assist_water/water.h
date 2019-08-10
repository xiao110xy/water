#pragma once
#define GET_VARIABLE_NAME(Variable) (#Variable)
#include "xy_torch.h"
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
#include "retinex.h"
#include "lsd.h"
#include "GeoMatch.h"
using namespace std;
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"  
#include "opencv2/objdetect/objdetect.hpp"
using namespace cv;


#define match_t 0.5
static 

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
	//
	map<string, vector<double>> xy_param{
		// 摄像头抖动
		// 白天水位获取
		{"score1_t1",vector<double>{0.8}},
		{"score1_t2",vector<double>{0.95}},
		{"score2_t1",vector<double>{0.6}},
		{"score2_t2",vector<double>{0.8}},
		{"score3_t1",vector<double>{0.3}},
		{"score3_t2",vector<double>{0.6}},
		{"roi",vector<double>{}},
		{"left_e",vector<double>{1}},
		{"length_e",vector<double>{5}},
		{"e_line",vector<double>{0}},
		{"gray_value_t1",vector<double>{230}},
		{"gray_value_t2",vector<double>{180}},
		{"gray_value_t",vector<double>{30}},
		{"otsu_high",vector<double>{1}},
		{"water_reflection",vector<double>{0}},
		// 夜晚水位获取
	};

	// 基本信息
	Mat base_image;
	Mat wrap_image;
	int add_row;
	int length;
	int ruler_number=1;
	// 用于配准
	vector<KeyPoint> keypoints;
	Mat descriptors;
	// roi 
	Mat assist_image;
	vector<double> roi;
	vector<double> new_roi;
	vector<double> sub_roi;
	int roi_order = 1;
	// 摄像头抖动
	bool correct_flag = false;
	double correct_score = -1;
	vector<vector<double>> temp_correct_point;
	vector<vector<double>> correct_point;
	//float correct_line;
	// 校正用点
	bool correct2poly = true;
	vector<vector<double>> point;
	Mat r;
	Mat r_inv;
	double rms_1, rms_2;
	Mat mask;
	Vec4f line_para;
	//ref
	int ref_index;
	Mat ref_image;
	// left_right_water
	bool left_right_no_water;
	// 分割水位线
	Mat segment_result;
	Mat segment_wrap_image;
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
void upadate_param(assist_information &assist_files, map<string, string> main_ini);
// 处理主函数

void compute_water_area(Mat image, vector<assist_information> &assist_files, map<string, string> main_ini);
//优化
void opt_assist_files(vector<assist_information> &assist_files);
// 判断是白天还是黑夜
bool isgrayscale(Mat im);
// 判断是否全是水
bool isblank(Mat im, assist_information &assist_file);
// 判断是否已经是底部了，白天；判断左右是否有水，晚上；
bool notall(Mat im, assist_information &assist_file);
// 判断water_line 是否可信
bool water_line_isok(int water_line, int all_length, int n_length);
// 夜晚是否过亮 并优化
bool isTooHighLightInNight(Mat im,int &water_line,int &gray_value, int gray_value_t);
// 对原始影像进行配准
bool correct_control_point(Mat im, assist_information &assist_file);
vector<assist_registration> xy_match(const Mat &image_1, const Mat &image_2, vector<vector<DMatch>> &dmatchs, vector<KeyPoint> keys_1,
	vector<KeyPoint> keys_2, string model,int  ruler_number, vector<double> roi);

void GetImageThreshold(Mat im, int &bestLowThresh, int& bestHighThresh);

// 几何校正原始影像
Mat correct_image(Mat im, assist_information &assist_file);
void map_coord(assist_information &assist_file,Mat &map_x, Mat &map_y,int base_x = 0, int base_y = 0);
Mat GeoCorrect2Poly(assist_information assist_file,bool flag);
Mat compute_point(Mat point, Mat r);
double compute_rms(Mat base_point, Mat wrap_point, Mat r);
// 水位线识别
bool get_label_mask(Mat mask,int &label, Mat &label_mask, double,int y_t);
float match_template_score(Mat temp1, Mat temp2);
vector<float> process_score(vector<float> score, float score_t1, float score_t2, bool water_reflection= false);

// 白天水位线
float get_water_line_day(Mat gc_im, assist_information &assist_file, int water_line, float scale = 0.4);
float get_water_line_day_nowater(Mat gc_im, assist_information &assist_file, int water_line, float scale = 0.4);
int get_mask_line(Mat mask,int n_length, float scale=0.4, int class_n=3);
int get_water_line(assist_information &assist_file );
Mat getBinMaskByMask(Mat mask);
int get_best_line(Mat mask, int x, int y);
// 根据短直线来
vector<vector<float>> get_line(Mat image, vector<vector<float>>& lines1, vector<vector<float>>& lines2, float det_v = 5, float det_h = 5);
Mat draw_line(Mat data, vector<vector<float>> lines, int base_x =0,vector<uchar> rgb = vector < uchar>{ 0, 0, 255 });

// 夜晚水位线
float get_water_line_night(Mat im,assist_information &assist_file);
float get_water_line_night_local(Mat im,assist_information &assist_file);
bool left_right_water(Mat gc_im,int length);
int get_water_line_seg(Mat im, int length, int add_rows = 100, float scale = 0.2);

// 结果保存
void save_file(Mat im, vector<assist_information> assist_files,map<string,string> main_ini);
// 功能函数
vector<vector<double>> part_row_point(vector<vector<double>> point,int r1,int r2);
vector<vector<double>> part_col_point(vector<vector<double>> point,int c1,int c2);
vector<vector<double>> swap_point(vector<vector<double>> &data);
Mat vector2Mat(vector<vector<double>> data);
vector<vector<double>> Mat2vector(Mat data);
int otsu(Mat im,bool flag = true);


