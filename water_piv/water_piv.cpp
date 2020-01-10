// water_piv.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <fstream>
#include<iomanip>
#include <map>
using namespace cv;
using namespace std;
map<string, vector<double>> xy_param{
	{"ref_frame",vector<double>{0.5}},
	{"step_frame",vector<double>{0.01}},
	{"compute_frame",vector<double>{0.1}},
	{"rotate_flag",vector<double>{0}},
	{"dist_marker-1-2",vector<double>{1}},
	{"dist_marker-3-4",vector<double>{1}},
	{"dist_marker-1-4",vector<double>{10}},
	{"dist_marker-2-3",vector<double>{10}},
	{"dist_marker-1-shore",vector<double>{2}},
	{"dist_marker-2-shore",vector<double>{2}},
	{"dist_marker-3-shore",vector<double>{2}},
	{"dist_marker-4-shore",vector<double>{2}},
	{"marker_1",vector<double>{}},
	{"marker_2",vector<double>{}},
	{"marker_3",vector<double>{}},
	{"marker_4",vector<double>{}},
	{"box_size",vector<double>{5}}
};

void split_path_name(string name,string& base_name, string& base_path) {
	for (int i = name.size(); i >= 0; --i) {
		if (name[i] == 47 || name[i] == 92) {
			base_name.append(name.begin() + i + 1, name.end() - 4);
			base_path.append(name.begin(), name.begin() + i + 1);
			break;
		}
	}
	if (base_path == "") {
		base_name.append(name.begin(), name.end() - 4);
	}
}

bool get_number(string line_string, vector<double>& temp)
{
	temp.clear();
	for (int i = 0; i < line_string.size(); ++i) {
		if ((line_string[i] >= 48 && line_string[i] <= 57) || line_string[i] == 45) {
			double temp_value = 0;
			int j = i;
			int n = -1;
			bool flag = true;
			for (; j < line_string.size(); ++j) {
				if (line_string[j] == 45) {
					flag = false;
					continue;
				}
				if (line_string[j] == 44 || line_string[j] == 59) {
					break;
				}
				if (line_string[j] >= 48 && line_string[j] <= 57) {
					temp_value = temp_value * 10 + line_string[j] - 48;
				}
				if (line_string[j] == 46)
					n = j;
			}
			temp_value = n == -1 ? temp_value : temp_value / pow(10, j - n - 1);
			if (!flag)
				temp_value = temp_value * -1;
			temp.push_back(temp_value);
			i = j;

		}
	}
	return true;
}

void upadate_param(string assist_file, map<string, vector<double>>& main_ini)
{
	fstream assist_file_name(assist_file);
	if (!assist_file_name)
		return;
	string temp;
	while (!assist_file_name.eof()) {
		temp.clear();
		getline(assist_file_name, temp);
		if (temp.size() < 4)
			break;
		for (int i = 1; i < temp.size() - 1; ++i) {
			if (temp[i] == 61) {
				string temp1(temp.begin(), temp.begin() + i);
				string temp2(temp.begin() + i + 1, temp.end());
				vector<double> temp_value;
				get_number(temp2, temp_value);
				if (temp_value.size() > 0)
					main_ini[temp1] = temp_value;
			}
		}
	}
}
Mat rotate_image_by_angle(Mat frame, int rotate_flag)
{
	if (rotate_flag == 90) {
		transpose(frame, frame);
		flip(frame, frame, 1);
	}
	else if (rotate_flag == 180) {
		flip(frame, frame, -1);
	}
	else if (rotate_flag == 270) {
		transpose(frame, frame);
		flip(frame, frame, 0);
	}
	return frame;
}

bool save_to_txt(string file_name, Mat im) {
	if (!im.data)
		return false;
	ofstream file(file_name);
	file.setf(ios::left);
	file.width(10);
	file.fill('0');
	file.precision(6);
	for (int i = 0; i < im.rows; ++i) {
		for (int j = 0; j < im.cols; ++j) {
			file << im.at<float>(i, j); 
			if (j == im.cols - 1)
				file <<";"<< endl;
			else
				file << ',';
		}
	}

	file.close();
	return true;
}

int main(int argc, char **argv)
{
	// 判断输入
	if (argc < 2)
	{
		cout << "please input video!";
		return -1;
	}
	if (argc > 4) {
		cout << "please input correct param!";
		return -2;
	}
	string file_name;
	string assit_name;
	string result_image;
	if (argc == 2) {
		file_name = string(argv[1]);
		string base_name, base_path;
		split_path_name(file_name, base_name, base_path);
		assit_name = base_path + "assist_" + base_name + ".txt";
		result_image = base_path + "result_" + base_name;
	}
	if (argc == 3) {
		file_name = string(argv[1]);
		assit_name = string(argv[2]);
		string base_name, base_path;
		split_path_name(file_name, base_name, base_path);
		result_image = base_path + "result_" + base_name;
	}
	if (argc == 4) {
		file_name = string(argv[1]);
		assit_name = string(argv[2]);
		result_image = string(argv[3]);
	}
	// 参数修改
	upadate_param(assit_name, xy_param); 
	// 读取视频及信息
	VideoCapture cap;
	cap.open(file_name);
	if (!cap.isOpened())
	{
		cout << "video file ie error!";
		return -3;
	}
	long nFrame = static_cast<long>(cap.get(CAP_PROP_FRAME_COUNT));
	int ref_frame = xy_param["ref_frame"][0] * nFrame;
	int step_frame = xy_param["step_frame"][0] * nFrame; 
	step_frame = step_frame > 1 ? step_frame : 1;
	int compute_frame = xy_param["compute_frame"][0] * nFrame/2;
	int fps = cap.get(CAP_PROP_FPS);
	double t = 1.0 / fps;
	int rotate_flag = int(xy_param["rotate_flag"][0]);

	int start_frame = ref_frame - (compute_frame / step_frame)*step_frame;
	int end_frame = ref_frame + (compute_frame / step_frame)*step_frame;
	// 计算光流
	Ptr<DenseOpticalFlow> algorithm = DISOpticalFlow::create(DISOpticalFlow::PRESET_FAST);
	Mat frame,back_image,forward_image, flow, flow_uv[2],gray1,gray2;
	for(int i = 1; i <= start_frame; ++i) {
		cap >> frame;// 读取无用帧
	}
	back_image = rotate_image_by_angle(frame, rotate_flag);
	Mat sum_flow1 = Mat::zeros(back_image.size(), CV_32FC2);// 前后光流结果
	Mat sum_flow2 = Mat::zeros(back_image.size(), CV_32FC2);
	Mat map_x = Mat::zeros(back_image.size(), CV_32FC1);
	for (int i = 0; i < map_x.cols; ++i) {
		map_x.col(i).setTo(i);
	}
	Mat map_y = Mat::zeros(back_image.size(), CV_32FC1);
	for (int i = 0; i < map_y.rows; ++i) {
		map_y.row(i).setTo(i);
	}
	bool reverse_flag = true;// 计算前还是后
	int n = start_frame + step_frame;
	for (int i = start_frame+1; i <= nFrame; ++i) {
		cap >> frame;
		if (n == i) {
			forward_image = rotate_image_by_angle(frame, rotate_flag);
			// 计算
			if (reverse_flag) {
				cv::cvtColor(back_image, gray2, COLOR_BGR2GRAY);
				cv::cvtColor(forward_image, gray1, COLOR_BGR2GRAY);
			}
			else {
				cv::cvtColor(back_image, gray1, COLOR_BGR2GRAY);
				cv::cvtColor(forward_image, gray2, COLOR_BGR2GRAY);
			}
			algorithm->calc(gray1, gray2, flow);
			// 累加
			if (reverse_flag) {
				split(sum_flow1, flow_uv);
				Mat temp;
				remap(flow, temp, map_x + flow_uv[0], map_y + flow_uv[1], INTER_LINEAR);
				sum_flow1 += temp;
			}
			else {
				split(sum_flow2, flow_uv);
				Mat temp;
				remap(flow, temp, map_x + flow_uv[0], map_y + flow_uv[1], INTER_LINEAR);
				sum_flow2 += temp;
			}
			// 
			if (n == ref_frame)
				reverse_flag = false;

			back_image = forward_image.clone();
			n += step_frame;
		}
	}
	// 光流合成
	Mat result_flow = sum_flow2 - sum_flow1;
	float all_time = (end_frame - start_frame)*t;
	result_flow = result_flow / all_time;
	Mat param_image = back_image;
	// 光流仿射
	if (xy_param["marker_1"].size() == 2&& xy_param["marker_2"].size() == 2
		&& xy_param["marker_3"].size() == 2 && xy_param["marker_4"].size() == 2) {
		double x1, y1, x2, y2, x3, y3, x4, y4;
		double dist12, dist34, dist23, dist14;
		double dist1shore, dist2shore, dist3shore, dist4shore;
		x1 = xy_param["marker_1"][0], y1 = xy_param["marker_1"][1];
		x2 = xy_param["marker_2"][0], y2 = xy_param["marker_2"][1];
		x3 = xy_param["marker_3"][0], y3 = xy_param["marker_3"][1];
		x4 = xy_param["marker_4"][0], y4 = xy_param["marker_4"][1];

		dist12 = xy_param["dist_marker-1-2"][0];
		dist34 = xy_param["dist_marker-3-4"][0];
		dist14 = xy_param["dist_marker-1-4"][0];
		dist23 = xy_param["dist_marker-2-3"][0];
		dist1shore = xy_param["dist_marker-1-shore"][0];
		dist2shore = xy_param["dist_marker-2-shore"][0];
		dist3shore = xy_param["dist_marker-3-shore"][0];
		dist4shore= xy_param["dist_marker-4-shore"][0];
		//
		double a1,b1,a2,b2;
		// 
		Mat map_x_real = Mat::zeros(result_flow.size(), CV_32FC1);
		a1 = dist34 / (x3 - x4);
		a2 = dist12 / (x2 - x1);
		b1 = 0.5*(y3 + y4);
		b2 = 0.5*(y1 + y2);
		for (int i = 0; i < map_x_real.rows; ++i) {
			double temp = (i - b2)/(b1-b2)*(a1-a2)+a2 ;
			map_x_real.row(i).setTo(temp);
		}
		//
		Mat map_y_real = Mat::zeros(result_flow.size(), CV_32FC1);
		a1 = dist23 / (y2 - y3);
		a2 = dist14 / (y1 - y4);
		b1 = 0.5*(x2 + x3);
		b2 = 0.5*(x1 + x4);
		for (int i = 0; i < map_y_real.cols; ++i) {
			double temp = (i - b2) / (b1 - b2)*(a1 - a2) + a2;
			map_y_real.col(i).setTo(temp);
		}

		// 结果保存

		Mat temp1, temp2;
		split(result_flow, flow_uv);
		multiply(flow_uv[0], map_x_real, flow_uv[0]);
		multiply(flow_uv[1], map_y_real, flow_uv[1]);


		int box_size = xy_param["box_size"][0];
		for (int i = 0; i < 2; ++i) {
			int xo, yo;
			Scalar temp;
			xo = 0.5*(x1 + x2);
			yo = 0.5*(y1 + y2);
			temp1 = flow_uv[i](Range(yo - box_size / 2, yo + box_size / 2), Range(xo - box_size / 2, xo + box_size / 2));
			xo = 0.5*(x3 + x4);
			yo = 0.5*(y3 + y4);
			temp2 = flow_uv[i](Range(yo - box_size / 2, yo + box_size / 2), Range(xo - box_size / 2, xo + box_size / 2));
			temp = mean(temp1);
			flow_uv[i] -= temp*0.5;

			temp = mean(temp2);
			flow_uv[i] -= temp * 0.5;
		}
		// 保存flow
		save_to_txt(result_image + "_x.txt", flow_uv[0]);
		save_to_txt(result_image + "_y.txt", flow_uv[1]);
		imwrite(result_image + ".jpg", param_image);
		// 画出点
		circle(param_image, Point(x1, y1), 3, Scalar(0, 0, 255),3);
		circle(param_image, Point(x2, y2), 3, Scalar(0, 0, 255), 3);
		circle(param_image, Point(x3, y3), 3, Scalar(0, 0, 255), 3);
		circle(param_image ,Point(x4,y4), 3, Scalar(0, 0, 255), 3);
		//获取文本框的长宽 
		int font_face = cv::FONT_HERSHEY_COMPLEX;
		double font_scale = 1;
		int thickness = 2;
		int baseline;
		putText(param_image, "1", Point2f(x1+ 20,y1- 20), font_face, font_scale, cv::Scalar(255, 0, 0), thickness, 12, 0);
		putText(param_image, "2", Point2f(x2 - 20, y2 - 20), font_face, font_scale, cv::Scalar(255, 0, 0), thickness, 12, 0);
		putText(param_image, "3", Point2f(x3 - 20, y3 + 20), font_face, font_scale, cv::Scalar(255, 0, 0), thickness, 12, 0);
		putText(param_image, "4", Point2f(x4 + 20, y4 + 20), font_face, font_scale, cv::Scalar(255, 0, 0), thickness, 12, 0);
		imwrite(result_image + ".jpg", param_image);
	}
	else {
		cout << "the label is error!";
		imwrite(result_image + ".jpg", param_image);
		return -4;
	}

	

	return 1;
}
