#pragma once

class water_line
{
public:
	water_line();
	~water_line();
};
void get_line(cv::Mat image, float det_v, float det_h);
void get_line(cv::Mat image);

