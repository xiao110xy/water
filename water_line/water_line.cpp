#include "stdafx.h"
#include "water_line.h"


water_line::water_line()
{
}


water_line::~water_line()
{
}

Mat draw_line(Mat data,vector<Matx<float, 6, 1>> lines)
{
	Mat result;
	if (data.channels() == 1) {
		Mat temp[3];
		temp[0] = data.clone(); temp[1] = data.clone(); temp[2] = data.clone();
		merge(temp,3,result);
	}
	else {
		result = data.clone();
	}
	auto a = lines.begin(); auto b = lines.end();
	for (auto i = a; i != b; ++i) {
		Point2f temp1 = Point2f((*i).val[0], (*i).val[1]);
		Point2f temp2 = Point2f((*i).val[2], (*i).val[3]);
		line(result, temp1, temp2, Scalar(0, 0, 255), 1, 8);
		if ((*i).rows>10) {
			Point2f temp1 = Point2f((*i).val[6], (*i).val[7]);
			Point2f temp2 = Point2f((*i).val[8], (*i).val[9]);
			line(result, temp1, temp2, Scalar(255, 0, 0), 1, 8);
		}
	}
	return result;
}

void get_line(cv::Mat image, float det_v, float det_h, vector<Matx<float, 6, 1>>& lines1, vector<Matx<float, 6, 1>>& lines2)
{
	Mat data;
	cvtColor(image, data, CV_BGR2GRAY);
	// lsd计算直线
	Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_STD);
	vector<Matx<float, 4, 1>> temp_lines;
	ls->detect(data, temp_lines);
	//计算直线长度、角度
	auto b = temp_lines.begin(); auto e = temp_lines.end();
	// 存贮检测到的直线
	for (auto i = b; i != e; ++i) {
		float x1, x2, y1, y2, distance, angle;
		x1 = (*i).val[0]; y1 = (*i).val[1];
		x2 = (*i).val[2]; y2 = (*i).val[3];
		// 长度
		distance = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
		// 角度
		angle = float(atan(-(y2 - y1) / (x2 - x1)) * 180 / atan(1) / 4);
		if (angle < 0) {
			angle = angle + 180;
		}
		// 竖直直线
		if (angle > 90 - det_v && angle < 90 + det_v) {
			if (y1 > y2) {
				float temp;
				temp = x1; x1 = x2; x2 = temp;
				temp = y1; y1 = y2; y2 = temp;
			}
			Matx<float, 6, 1> temp{ x1,y1,x2,y2,distance,angle };
			lines1.push_back(temp);
		}
		// 水平直线      
		if (angle > 180 - det_h || angle < det_h) {
			if (x1 > x2) {
				float temp;
				temp = x1; x1 = x2; x2 = temp;
				temp = y1; y1 = y2; y2 = temp;
			}
			Matx<float, 6, 1> temp{ x1,y1,x2,y2,distance,angle };
			lines2.push_back(temp);
		}
	}
	// 按长度进行排序
	stable_sort(lines1.begin(), lines1.end(),
		[](const Matx<float, 6, 1>&a, const Matx<float, 6, 1>&b) {return a.val[4] > b.val[4]; });
	stable_sort(lines2.begin(), lines2.end(),
		[](const Matx<float, 6, 1>&a, const Matx<float, 6, 1>&b) {return a.val[4] > b.val[4]; });
}

void get_line(Mat image, vector<Matx<float, 6, 1>> &lines1, vector<Matx<float, 6, 1>> &lines2)
{
	float det_v = 22.5; float det_h = 22.5;
	Mat data;
	cvtColor(image, data, CV_BGR2GRAY);
	// lsd计算直线
	Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_STD);
	vector<Matx<float,4,1>> temp_lines;
	ls->detect(data, temp_lines);
	//计算直线长度、角度
	auto b = temp_lines.begin(); auto e = temp_lines.end();
	// 存贮检测到的直线
	for (auto i = b; i != e; ++i) {
		float x1, x2, y1, y2, distance, angle;
		x1 = (*i).val[0]; y1 = (*i).val[1];
		x2 = (*i).val[2]; y2 = (*i).val[3];
		// 长度
		distance = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
		// 角度
		angle = float(atan(-(y2 - y1) / (x2 - x1)) * 180 / atan(1) / 4);
		if (angle < 0) {
			angle = angle + 180;
		}
		// 竖直直线
		if (angle > 90 - det_v && angle < 90 + det_v) {
			if (y1 > y2) {
				float temp;
				temp = x1; x1 = x2; x2 = temp;
				temp = y1; y1 = y2; y2 = temp;
			}
			Matx<float, 6, 1> temp{ x1,y1,x2,y2,angle,distance };
			lines1.push_back(temp);
		}
		// 水平直线      
		if (angle > 180 - det_h||angle < det_h) {
			if (x1 > x2) {
				float temp;
				temp = x1; x1 = x2; x2 = temp;
				temp = y1; y1 = y2; y2 = temp;
			}
			Matx<float, 6, 1> temp{ x1,y1,x2,y2,angle,distance };
			lines2.push_back(temp);
		}
	}
	// 按长度进行排序
	stable_sort(lines1.begin(), lines1.end(),
		[](const Matx<float, 6, 1>&a, const Matx<float, 6, 1>&b) {return a.val[5] > b.val[5]; });
	stable_sort(lines2.begin(), lines2.end(),
		[](const Matx<float, 6, 1>&a, const Matx<float, 6, 1>&b) {return a.val[5] > b.val[5]; });

}

vector<Matx<float, 12, 1>> get_parallel_lines(Mat image, vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2)
{
	vector<Matx<float, 6, 1>> lines(lines1);
	// 去除短线
	auto b = lines.begin(); auto e = lines.end();
	for (auto i = b; i != e; ++i) {
		if ((*i).val[5] < 25) {
			lines.erase(i, e);
			break;
		}
	}
	if (lines.size() < 1) {
		return vector<Matx<float, 12, 1>>();
	}
	// 合并直线
	vector<Matx<float, 6, 1>> temp(lines);
	while (temp.size() > 0) {
		Matx<float, 6, 1 > line= *temp.begin();
		temp.erase(temp.begin());
		Matx<float, 6, 1 > new_line = merge_line(line, temp, 2.5, 10, 40);
		break;
	}

}

Matx<float, 6, 1> merge_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines, float angle_t, float d_v_t, float d_p_t)
{
	Matx<float, 6, 1> new_line;
	// 角度差异较大 排除
	vector<Matx<float, 6, 1>> temp1, temp2;
	for (auto i = lines.begin(); i != lines.end(); ++i) {
		if ((*i).val[4]<line.val[4] + angle_t && (*i).val[4]>line.val[4] - angle_t) {
			temp1.push_back(*i);
		}
		else {
			temp2.push_back(*i);
		}
	}

	return new_line;
}
