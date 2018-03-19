#pragma once
#include <iostream>
#include <algorithm>
#include <vector> 
#include <string>

class water_line
{
public:
	water_line();
	~water_line();
};
Mat draw_line(Mat data,std::vector<Matx<float, 6, 1>> lines);

void get_line(cv::Mat image, float det_v, float det_h, std::vector<Matx<float, 6, 1>> &lines1, std::vector<Matx<float, 6, 1>> &lines2);
void get_line(cv::Mat image,std::vector<Matx<float, 6, 1>> &lines1, std::vector<Matx<float, 6, 1>> &lines2);

