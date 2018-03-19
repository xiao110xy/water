#include "stdafx.h"
#include "water_line.h"


water_line::water_line()
{
}


water_line::~water_line()
{
}

Mat draw_line(Mat data,std::vector<Matx<float, 6, 1>> lines)
{
	cvtColor(data, data, CV_BGR2GRAY);
	if (data.channels() == 1) {
		Mat temp[3];
		temp[0] = data.clone(); temp[1] = data.clone(); temp[2] = data.clone();
		merge(temp,3,data);
	}
	return Mat();
}

void get_line(cv::Mat image, float det_v, float det_h, std::vector<Matx<float, 6, 1>>& lines1, std::vector<Matx<float, 6, 1>>& lines2)
{
	Mat data;
	cvtColor(image, data, CV_BGR2GRAY);
	// lsd计算直线
	Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_STD);
	std::vector<Matx<float, 4, 1>> temp_lines;
	ls->detect(data, temp_lines);
	//计算直线长度、角度
	auto b = temp_lines.begin(); auto e = temp_lines.end();
	// 存贮检测到的直线
	for (auto i = b; i != e; ++i) {
		float x1, x2, y1, y2, distance, angle;
		x1 = (*i).val[0]; y1 = (*i).val[1];
		x2 = (*i).val[2]; y2 = (*i).val[3];
		// 长度
		distance = std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
		// 角度
		angle = float(std::atan(-(y2 - y1) / (x2 - x1)) * 180 / std::atan(1) / 4);
		if (angle < 0) {
			angle = angle + 180;
		}

		// 水平直线      
		if (angle > 180 - det_h || angle < det_h) {
			if (x1 > x2) {
				float temp;
				temp = x1; x1 = x2; x2 = temp;
				temp = y1; y1 = y2; y2 = temp;
			}
			Matx<float, 6, 1> temp{ x1,y1,x2,y2,distance,angle };
			lines1.push_back(temp);
		}

		// 竖直直线
		if (angle > 90 - det_v && angle < 90 + det_v) {
			if (y1 > y2) {
				float temp;
				temp = x1; x1 = x2; x2 = temp;
				temp = y1; y1 = y2; y2 = temp;
			}
			Matx<float, 6, 1> temp{ x1,y1,x2,y2,distance,angle };
			lines2.push_back(temp);
		}
	}
	// 按长度进行排序
	std::stable_sort(lines1.begin(), lines1.end(),
		[](const Matx<float, 6, 1>&a, const Matx<float, 6, 1>&b) {return a.val[4] > b.val[4]; });
	std::stable_sort(lines2.begin(), lines2.end(),
		[](const Matx<float, 6, 1>&a, const Matx<float, 6, 1>&b) {return a.val[4] > b.val[4]; });
}

void get_line(Mat image, std::vector<Matx<float, 6, 1>> &lines1, std::vector<Matx<float, 6, 1>> &lines2)
{
	float det_v = 22.5; float det_h = 22.5;
	Mat data;
	cvtColor(image, data, CV_BGR2GRAY);
	// lsd计算直线
	Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_STD);
	std::vector<Matx<float,4,1>> temp_lines;
	ls->detect(data, temp_lines);
	//计算直线长度、角度
	auto b = temp_lines.begin(); auto e = temp_lines.end();
	// 存贮检测到的直线
	for (auto i = b; i != e; ++i) {
		float x1, x2, y1, y2, distance, angle;
		x1 = (*i).val[0]; y1 = (*i).val[1];
		x2 = (*i).val[2]; y2 = (*i).val[3];
		// 长度
		distance = std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
		// 角度
		angle = float(std::atan(-(y2 - y1) / (x2 - x1)) * 180 / std::atan(1) / 4);
		if (angle < 0) {
			angle = angle + 180;
		}

		// 水平直线      
		if (angle > 180 - det_h||angle < det_h) {
			if (x1 > x2) {
				float temp;
				temp = x1; x1 = x2; x2 = temp;
				temp = y1; y1 = y2; y2 = temp;
			}
			Matx<float, 6, 1> temp{ x1,y1,x2,y2,distance,angle };
			lines1.push_back(temp);
		}

		// 竖直直线
		if (angle > 90 - det_v && angle < 90 + det_v) {
			if (y1 > y2) {
				float temp;
				temp = x1; x1 = x2; x2 = temp;
				temp = y1; y1 = y2; y2 = temp;
			}
			Matx<float, 6, 1> temp{ x1,y1,x2,y2,distance,angle };
			lines2.push_back(temp);
		}
	}
	// 按长度进行排序
	std::stable_sort(lines1.begin(), lines1.end(),
		[](const Matx<float, 6, 1>&a, const Matx<float, 6, 1>&b) {return a.val[4] > b.val[4]; });
	std::stable_sort(lines2.begin(), lines2.end(),
		[](const Matx<float, 6, 1>&a, const Matx<float, 6, 1>&b) {return a.val[4] > b.val[4]; });

}
