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
	vector<Matx<float, 6, 1>> data,result_lines;
	while (lines.size() > 0) {
		Matx<float, 6, 1 > line= *lines.begin();
		lines.erase(lines.begin());
		vector<Matx<float, 6, 1>> new_line = merge_line(line,lines,data, 2.5, 10, 40);
		if (new_line.size()<1) {
			lines = data;
			result_lines.push_back(line);
		}
		else {
			lines = new_line;
			lines.insert(lines.end(), data.begin(), data.end());
		}
	}
	image =draw_line(image, result_lines);
	return vector<Matx<float, 12, 1>>();
}

vector<Matx<float, 6, 1>>  merge_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines, vector<Matx<float, 6, 1>> &data, float angle_t, float d_v_t, float d_p_t)
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
	//  投影距离
	lines = temp1;
	data = temp2;
	temp1.clear(); temp2.clear();
	if (lines.size() < 1) {
		return vector<Matx<float, 6, 1>>();
	}
	vector<Matx<float, 7, 1>> temp_lines = porject_line(line, lines);
	for (int i=0; i != lines.size(); ++i){
		if ((*(temp_lines.begin()+i)).val[6] < d_v_t) {
			temp1.push_back(*(lines.begin() + i));
			temp_lines.push_back(*(temp_lines.begin() + i));
		}
		else {
			temp2.push_back(*(lines.begin() + i));
		}
	}
	temp_lines.erase(temp_lines.begin(), temp_lines.begin() + lines.size());
	lines = temp1;
	data.insert(data.end(), temp2.begin(), temp2.end());
	temp1.clear(); temp2.clear();
	if (lines.size() < 1) {
		return vector<Matx<float, 6, 1>>();
	}
	// 端点距离
	for (int i = 0; i != lines.size(); ++i) {
		float d[4];
		d[0] = sqrt(pow(line.val[0] - (*(lines.begin() + i)).val[0], 2) + pow(line.val[1] - (*(lines.begin() + i)).val[1], 2));
		d[1] = sqrt(pow(line.val[0] - (*(lines.begin() + i)).val[2], 2) + pow(line.val[1] - (*(lines.begin() + i)).val[3], 2));
		d[2] = sqrt(pow(line.val[2] - (*(lines.begin() + i)).val[0], 2) + pow(line.val[2] - (*(lines.begin() + i)).val[1], 2));
		d[3] = sqrt(pow(line.val[2] - (*(lines.begin() + i)).val[2], 2) + pow(line.val[2] - (*(lines.begin() + i)).val[3], 2));
		float mind= d[0];
		for (int j = 1; j < 4; ++j) {
			mind = mind < d[j] ? mind : d[j];
		}
		float k1, k2;
		k1 = (*(temp_lines.begin() + i)).val[4]; k2 = (*(temp_lines.begin() + i)).val[5];
		bool flag = (k1 < 0 && k2 < 0) || (k1 > 1 && k2 > 1);
		flag = flag && (mind > d_p_t);
		if (!flag) {
			temp1.push_back(*(lines.begin() + i));
			temp_lines.push_back(*(temp_lines.begin() + i));
		}
		else {
			temp2.push_back(*(lines.begin() + i));
		}
	}
	temp_lines.erase(temp_lines.begin(), temp_lines.begin() + lines.size());
	lines = temp1;
	data.insert(data.end(), temp2.begin(), temp2.end());
	temp1.clear(); temp2.clear();
	if (lines.size() < 1) {
		return vector<Matx<float, 6, 1>>();
	}
	// 准备合并
	// 按长度进行排序
	new_line = line;
	stable_sort(temp_lines.begin(), temp_lines.end(),
		[](const Matx<float, 7, 1>&a, const Matx<float, 7, 1>&b) {return a.val[4] < b.val[4]; });
	float k1 = (*temp_lines.begin()).val[4];
	if (k1 < 0) {
		new_line.val[0] = (*temp_lines.begin()).val[0];
		new_line.val[1] = (*temp_lines.begin()).val[1];
	}
	stable_sort(temp_lines.begin(), temp_lines.end(),
		[](const Matx<float, 7, 1>&a, const Matx<float, 7, 1>&b) {return a.val[5] > b.val[5]; });
	float k2 = (*temp_lines.begin()).val[5];
	if (k2 > 1) {
		new_line.val[2] = (*temp_lines.begin()).val[2];
		new_line.val[3] = (*temp_lines.begin()).val[3];
	}
	new_line.val[5] = sqrt(pow(new_line.val[2]- new_line.val[0], 2) + pow(line.val[3] - new_line.val[1], 2));
	return vector<Matx<float, 6, 1>>{new_line};
}

vector<Matx<float, 7, 1>> porject_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines)
{
	vector<Matx<float, 7, 1>> result;
	for (auto i = lines.begin(); i != lines.end(); ++i) {
		Matx<float, 7, 1> temp;
		float d = line.val[5];
		float k1 = (((*i).val[0] - line.val[0])*(line.val[2] - line.val[0]) +
			((*i).val[1] - line.val[1])*(line.val[3] - line.val[1])) / pow(d, 2);
		float k2 = (((*i).val[2] - line.val[0])*(line.val[2] - line.val[0]) +
			((*i).val[3] - line.val[1])*(line.val[3] - line.val[1])) / pow(d, 2);
		temp.val[0] = line.val[0] + k1 * (line.val[2] - line.val[0]);
		temp.val[1] = line.val[1] + k1 * (line.val[3] - line.val[1]);
		temp.val[2] = line.val[0] + k2 * (line.val[2] - line.val[0]);
		temp.val[3] = line.val[1] + k2 * (line.val[3] - line.val[1]);
		float d1, d2;
		d1 = sqrt(pow((*i).val[0] - temp.val[0], 2) + pow((*i).val[1] - temp.val[1], 2));
		d2 = sqrt(pow((*i).val[2] - temp.val[2], 2) + pow((*i).val[3] - temp.val[3], 2));
		temp.val[4] = k1;
		temp.val[5] = k2;
		temp.val[6] = d1 < d2 ? d1 : d2;
		result.push_back(temp);
	}
	return result;
}
