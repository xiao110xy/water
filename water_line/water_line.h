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
Mat draw_line(Mat data,vector<Matx<float, 6, 1>> lines);

void get_line(Mat image, float det_v, float det_h, vector<Matx<float, 6, 1>> &lines1, vector<Matx<float, 6, 1>> &lines2);
void get_line(Mat image,vector<Matx<float, 6, 1>> &lines1, vector<Matx<float, 6, 1>> &lines2);

vector<Matx<float, 12, 1>> get_parallel_lines(Mat image, vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2);
Matx<float, 6, 1> merge_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines, float angle_t, float d_v_t, float d_p_t);