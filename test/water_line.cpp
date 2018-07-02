#include "stdafx.h"
#include "water_line.h"


water_line::water_line()
{
}


water_line::~water_line()
{
}

Mat draw_line(Mat data, vector<Matx<float, 6, 1>> lines)
{
	Mat result = draw_line(data, lines, Scalar(0, 0, 255));
	return result;
}
Mat draw_line(Mat data, vector<Matx<float, 6, 1>> lines, Scalar rgb)
{
	Mat result;
	if (data.channels() == 1) {
		Mat temp[3];
		temp[0] = data.clone(); temp[1] = data.clone(); temp[2] = data.clone();
		merge(temp, 3, result);
	}
	else {
		result = data.clone();
	}
	auto a = lines.begin(); auto b = lines.end();
	for (auto i = a; i != b; ++i) {
		Point2f temp1 = Point2f((*i).val[0], (*i).val[1]);
		Point2f temp2 = Point2f((*i).val[2], (*i).val[3]);
		line(result, temp1, temp2, rgb, 1, 8);
	}
	return result;
}
Mat draw_line(Mat data, vector<Matx<float, 12, 1>> lines)
{
	vector<Scalar> rgb;
	rgb.push_back(Scalar(0, 0, 255));
	rgb.push_back(Scalar(255, 0, 0));
	Mat result = draw_line(data, lines, rgb);
	return result;
}
Mat draw_line(Mat data, vector<Matx<float, 12, 1>> lines, vector<Scalar> rgb)
{
	if (rgb.size() < 2) {
		rgb.push_back(Scalar(0, 0, 255));
		rgb.push_back(Scalar(255, 0, 0));
	}
	Mat result;
	if (data.channels() == 1) {
		Mat temp[3];
		temp[0] = data.clone(); temp[1] = data.clone(); temp[2] = data.clone();
		merge(temp, 3, result);
	}
	else {
		result = data.clone();
	}
	auto a = lines.begin(); auto b = lines.end();
	for (auto i = a; i != b; ++i) {
		Point2f temp1 = Point2f((*i).val[0], (*i).val[1]);
		Point2f temp2 = Point2f((*i).val[2], (*i).val[3]);
		line(result, temp1, temp2, rgb[0], 1, 8);
		temp1 = Point2f((*i).val[6], (*i).val[7]);
		temp2 = Point2f((*i).val[8], (*i).val[9]);
		line(result, temp1, temp2, rgb[1], 1, 8);
	}
	return result;
}
Mat draw_point(Mat data, vector<Point3f> points, Scalar rgb) {
	Mat result;
	if (data.channels() == 1) {
		Mat temp[3];
		temp[0] = data.clone(); temp[1] = data.clone(); temp[2] = data.clone();
		merge(temp, 3, result);
	}
	else {
		result = data.clone();
	}
	Point2i temp;
	for (auto &i : points) {
		temp.x = (int)round(i.x);
		temp.y = (int)round(i.y);
		circle(result, temp, 2, rgb, -1);
	}
	return result;
}
Mat draw_point(Mat data, vector<Point2i> points, Scalar rgb)
{
	Mat result;
	if (data.channels() == 1) {
		Mat temp[3];
		temp[0] = data.clone(); temp[1] = data.clone(); temp[2] = data.clone();
		merge(temp, 3, result);
	}
	else {
		result = data.clone();
	}
	for (auto &i : points) {
		circle(result, i, 2, rgb, -1);
	}
	return result;
}

Mat draw_point(Mat data, vector<Mat> points)
{
	Mat result;
	if (data.channels() == 1) {
		Mat temp[3];
		temp[0] = data.clone(); temp[1] = data.clone(); temp[2] = data.clone();
		merge(temp, 3, result);
	}
	else {
		result = data.clone();
	}
	vector<Scalar> rgb;
	rgb.push_back(Scalar(0, 255, 0));
	rgb.push_back(Scalar(0, 0, 255));
	rgb.push_back(Scalar(255, 0, 0));
	int x = 0, y = 0;
	for (auto &i : points) {
		for (int j = 0; j < 3; ++j) {
			x = i.at<int>(j, 0);
			y = i.at<int>(j, 1);
			circle(result, Point2i(x, y), 2, rgb[j], -1);
		}
	}
	return result;
}
Mat draw_point(Mat data, vector<vector<float>> points)
{
	Mat result;
	if (data.channels() == 1) {
		Mat temp[3];
		temp[0] = data.clone(); temp[1] = data.clone(); temp[2] = data.clone();
		merge(temp, 3, result);
	}
	else {
		result = data.clone();
	}
	vector<Scalar> rgb;
	rgb.push_back(Scalar(0, 255, 0));
	rgb.push_back(Scalar(0, 0, 255));
	rgb.push_back(Scalar(255, 0, 0));
	int x = 0, y = 0, flag = 0;
	for (auto &i : points) {
		x = (int)i[0];
		y = (int)i[1];
		flag = (int)i[2];
		flag = flag == -1 ? 0 : 1;
		circle(result, Point2i(x, y), 2, rgb[flag], -1);
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
			Matx<float, 6, 1> temp{ x1,y1,x2,y2,angle,distance };
			lines1.push_back(temp);
		}
		// 水平直线      
		if (angle > 180 - det_h || angle < det_h) {
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

void get_line(Mat image, vector<Matx<float, 6, 1>> &lines1, vector<Matx<float, 6, 1>> &lines2)
{
	float det_v = 22.5; float det_h = 22.5;
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
			Matx<float, 6, 1> temp{ x1,y1,x2,y2,angle,distance };
			lines1.push_back(temp);
		}
		// 水平直线      
		if (angle > 180 - det_h || angle < det_h) {
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
	Mat test_im;
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
	//
	test_im = draw_line(image, lines);
	// 合并直线
	vector<Matx<float, 6, 1>> data, result_lines;
	while (lines.size() > 0) {
		Matx<float, 6, 1 > line;
		vector<Matx<float, 6, 1>> temp_lines;
		temp_lines = lines;
		while (1) {
			vector<Matx<float, 6, 1>> temp_line{temp_lines[0]};
			temp_lines.erase(temp_lines.begin());
			temp_line = merge_line(temp_line[0], temp_lines, data, 2.5, 20, 40);
			temp_lines = temp_line;
			temp_lines.insert(temp_lines.end(), data.begin(), data.end());
			if (temp_line.size() == 0)
				break;
		}
		vector<Matx<float, 6, 1>> points_lines = intersect_line(lines, temp_lines);
		Matx<float, 6, 1> new_line = sub_merge_line(points_lines);
		test_im = image.clone();
		test_im = draw_line(test_im, lines,Scalar(0,0,255));
		test_im = draw_line(test_im, points_lines, Scalar(0, 255, 0));
		test_im = draw_line(test_im, vector<Matx<float, 6, 1>>{new_line}, Scalar(255, 0, 0));

		result_lines.push_back(new_line);
		lines = data;


	}
	lines = result_lines;
	std::stable_sort(lines.begin(), lines.end(),
		[](const Matx<float, 6, 1>&a, const Matx<float, 6, 1>&b) {return a.val[5] > b.val[5]; });
	//
	test_im = draw_line(image, lines);
	//
	if (lines.size() < 1) {
		return vector<Matx<float, 12, 1>>();
	}
	// 平行线获取
	vector<Matx<float, 12, 1>> parallel_lines;
	while (lines.size() > 0) {
		Matx<float, 6, 1> line = *lines.begin();
		lines.erase(lines.begin());
		vector<Matx<float, 6, 1>> temp_lines;
		for (int i = 5; i < 20; ++i) {
			float angle_t = (float)0.5*i;
			for (auto &j : lines) {
				if ((j.val[4] < line.val[4] + angle_t) && (j.val[4] > line.val[4] - angle_t)) {
					temp_lines.push_back(j);
				}
			}
			if (temp_lines.size() > 0) {
				break;
			}
		}
		test_im = draw_line(image, temp_lines,Scalar(0, 255, 0));
		test_im = draw_line(test_im, lines2, Scalar(0, 255, 0));
		test_im = draw_line(test_im, vector<Matx<float, 6, 1>>{line}, Scalar(0, 0, 255));

		vector<Matx<float, 6, 1>> result_line = judge_line(line, temp_lines, lines2);
		test_im = draw_line(test_im, result_line, Scalar(255, 0, 0));

		if (result_line.size() < 1) {
			continue;
		}
		else {
			lines = intersect_line(lines, result_line);
			Matx<float, 12, 1> temp;
			for (int i = 0; i < 6; ++i) {
				if (line.val[0] < (*(result_line.begin())).val[0]) {
					temp.val[i] = line.val[i];
					temp.val[i + 6] = (*(result_line.begin())).val[i];
				}
				else {
					temp.val[i + 6] = line.val[i];
					temp.val[i] = (*(result_line.begin())).val[i];
				}
			}
			parallel_lines.push_back(temp);
		}
	}

	//
	test_im = draw_line(image, parallel_lines);
	//
	return parallel_lines;
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
	lines = temp1;
	data = temp2;
	temp1.clear(); temp2.clear();
	if (lines.size() < 1) {
		return vector<Matx<float, 6, 1>>();
	}
	//  投影距离
	vector<Matx<float, 7, 1>> temp_lines = project_line(line, lines);
	for (int i = 0; i != lines.size(); ++i) {
		if ((*(temp_lines.begin() + i)).val[6] < d_v_t) {
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
		d[2] = sqrt(pow(line.val[2] - (*(lines.begin() + i)).val[0], 2) + pow(line.val[3] - (*(lines.begin() + i)).val[1], 2));
		d[3] = sqrt(pow(line.val[2] - (*(lines.begin() + i)).val[2], 2) + pow(line.val[3] - (*(lines.begin() + i)).val[3], 2));
		float mind = d[0];
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
	// data 排序
	stable_sort(data.begin(), data.end(),
		[](const Matx<float, 6, 1>&a, const Matx<float, 6, 1>&b) {return a.val[5] < b.val[5]; });
	temp1.clear(); temp2.clear();
	if (lines.size() < 1) {
		return vector<Matx<float, 6, 1>>();
	}
	// 准备合并
	// 按长度进行排序
	new_line = line;
	vector<Point> points;
	points = get_line_point(Point((int)line.val[0], (int)line.val[1]), Point((int)line.val[2], (int)line.val[3]));
	Vec4f line_para;
	fitLine(points, line_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	vector<Matx<float, 6, 1>> temp(lines.begin(), lines.end());
	for (auto &i : temp) {
		Point2f point1, point2;
		point1.x = i.val[0];
		point1.y = i.val[1];
		point2.x = i.val[2];
		point2.y = i.val[3];
		vector<Point> temp_point = get_line_point(point1, point2);
		points.insert(points.end(),temp_point.begin(),temp_point.end());
	}
	new_line.val[0] = line.val[0];new_line.val[1] = line.val[1];
	new_line.val[2] = line.val[2];new_line.val[3] = line.val[3];
	for (auto &i : points) {
		float x = 0, y = 0;
		float d = (i.x - line_para[2])*line_para[0]+ (i.y - line_para[3])*line_para[1];
		x = d * line_para[0] + line_para[2];
		y = d * line_para[1] + line_para[3];
		if (new_line.val[1] > y) {
			new_line.val[0] = x;
			new_line.val[1] = y;
		}
		else if (new_line.val[3] < y) {
			new_line.val[2] = x;
			new_line.val[3] = y;
		}
	}
	new_line.val[4] = float(atan(-(new_line.val[3] - new_line.val[1]) / (new_line.val[2] - new_line.val[0])) * 180 / atan(1) / 4);
	if (new_line.val[4] < 0) {
		new_line.val[4] = new_line.val[4] + 180;
	}
	new_line.val[5] = sqrt(pow(new_line.val[2] - new_line.val[0], 2) + pow(new_line.val[3] - new_line.val[1], 2));

	return vector<Matx<float, 6, 1>>{new_line};
}

Matx<float, 6, 1> sub_merge_line(vector<Matx<float, 6, 1>> lines)
{
	int index1 = 0, index2 = 0;
	float y1=9999, y2=-9999;
	for (int i = 0; i < lines.size(); ++i) {
		if (lines[i].val[1] < y1) {
			y1 = lines[i].val[1];
			index1 = i;
		}
		if (lines[i].val[3] > y2) {
			y2 = lines[i].val[3];
			index2 = i;
		}
	}
	if (index1 == 0 || index2 == 0) {
		y1 = -9999;
		y2 = 9999;
	}
	vector<Point> points,temp_points;
	for (auto &i : lines) {
		Point2f point1, point2;
		point1.x = i.val[0]; point1.y = i.val[1];
		point2.x = i.val[2]; point2.y = i.val[3];
		vector<Point> temp = get_line_point(point1, point2);
		if (i.val[1]<=y1||i.val[3]>=y2)
			temp_points.insert(temp_points.end(), temp.begin(), temp.end());
		else
			points.insert(points.end(), temp.begin(), temp.end());
	}
	Vec4f line_para;
	fitLine(points, line_para, CV_DIST_L12, 0, 1e-2, 1e-2);
	points.insert(points.end(), temp_points.begin(), temp_points.end());
	Matx<float, 6, 1> new_line;
	new_line.val[1] = 99999;new_line.val[3] = -99999;
	for (auto &i : points) {
		float x = 0, y = 0;
		float d = (i.x - line_para[2])*line_para[0] + (i.y - line_para[3])*line_para[1];
		x = d * line_para[0] + line_para[2];
		y = d * line_para[1] + line_para[3];
		if (new_line.val[1] > y) {
			new_line.val[0] = x;
			new_line.val[1] = y;
		}
		else if (new_line.val[3] < y) {
			new_line.val[2] = x;
			new_line.val[3] = y;
		}
	}
	new_line.val[4] = float(atan(-(new_line.val[3] - new_line.val[1]) / (new_line.val[2] - new_line.val[0])) * 180 / atan(1) / 4);
	if (new_line.val[4] < 0) {
		new_line.val[4] = new_line.val[4] + 180;
	}
	new_line.val[5] = sqrt(pow(new_line.val[2] - new_line.val[0], 2) + pow(new_line.val[3] - new_line.val[1], 2));
	return new_line;
}

vector<Point> get_line_point(Point2f point1, Point2f point2)
{
	vector<Point> result;
	int x1, x2, y1, y2;
	x1 = (int)round(point1.x);
	y1 = (int)round(point1.y);
	x2 = (int)round(point2.x);
	y2 = (int)round(point2.y);
		
	int dx = x2 - x1;
	int dy = y2 - y1;
	int ux = ((dx > 0) << 1) - 1;//x的增量方向，取或-1
	int uy = ((dy > 0) << 1) - 1;//y的增量方向，取或-1
	int x = x1, y = y1, eps;//eps为累加误差

	eps = 0; dx = abs(dx); dy = abs(dy);
	if (dx > dy)
	{
		for (x = x1; x != x2; x += ux)
		{
			result.push_back(Point(x, y));
			eps += dy;
			if ((eps << 1) >= dx)
			{
				y += uy; eps -= dx;
			}
		}
	}
	else
	{
		for (y = y1; y != y2; y += uy)
		{
			result.push_back(Point(x, y));
			eps += dx;
			if ((eps << 1) >= dy)
			{
				x += ux; eps -= dy;
			}
		}
	}
	result.push_back(Point(x2, y2));
	return result;
}

vector<Matx<float, 7, 1>> project_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines)
{
	//Mat test_im = Mat::zeros(Size(2000, 2000), CV_8UC3);
	
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
		//cv::line(test_im, Point2f(temp.val[0], temp.val[1]), Point2f(temp.val[2], temp.val[3]),Scalar(255,0,0));
		//cv::line(test_im, Point2f((*i).val[0], (*i).val[1]), Point2f((*i).val[2], (*i).val[3]), Scalar(255, 0, 0));
		//cv::line(test_im, Point2f(temp.val[0], temp.val[1]), Point2f((*i).val[0], (*i).val[1]),Scalar(0,255,0));
		//cv::line(test_im, Point2f(temp.val[2], temp.val[3]), Point2f((*i).val[2], (*i).val[3]), Scalar(0, 255, 0));
		float d1, d2;
		d1 = sqrt(pow((*i).val[0] - temp.val[0], 2) + pow((*i).val[1] - temp.val[1], 2));
		d2 = sqrt(pow((*i).val[2] - temp.val[2], 2) + pow((*i).val[3] - temp.val[3], 2));
		temp.val[4] = k1;
		temp.val[5] = k2;
		temp.val[6] = d1 < d2 ? d1 : d2;
		result.push_back(temp);
	}
	//cv::line(test_im, Point2f(line.val[0], line.val[1]), Point2f(line.val[2], line.val[3]), Scalar(0, 0, 255));
	return result;
}

vector<Matx<float, 6, 1>> judge_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines, vector<Matx<float, 6, 1>> lines2)
{
	// 计算lines在line上的端点投影位置
	vector<Matx<float, 6, 1>> temp1;
	vector<Matx<float, 7, 1>> temp2;
	vector<Matx<float, 7, 1>> temp_lines = project_line(line, lines);
	for (int i = 0; i < lines.size(); ++i) {
		float d_t = (*(temp_lines.begin() + i)).val[6];
		if ((d_t < 100) && (d_t > 20)) {
			temp1.push_back(*(lines.begin() + i));
			temp2.push_back(*(temp_lines.begin() + i));
		}
	}
	if (temp1.size() < 1) {
		return vector<Matx<float, 6, 1>>();
	}
	lines = temp1;
	temp_lines = temp2;
	temp1.clear(); temp2.clear();
	// 长线应该完全包含短线，但容许短线中存在一点在长线内
	for (int i = 0; i < lines.size(); ++i) {
		float k1 = (*(temp_lines.begin() + i)).val[4];
		float k2 = (*(temp_lines.begin() + i)).val[5];
		bool flag = ((k1 > 0) && (k1 < 1)) || ((k2 > 0) && (k2 < 1));
		if (flag) {
			temp1.push_back(*(lines.begin() + i));
			temp2.push_back(*(temp_lines.begin() + i));
		}
	}
	lines = temp1;
	temp_lines = temp2;
	temp1.clear(); temp2.clear();
	// 水尺区域包含的线段应该有大量水平线段
	vector<Matx<float, 6, 1>> score;
	for (int i = 0; i < lines.size(); ++i) {
		vector<Point2f> temp = judge_parrallel_line(line, lines[i]);
		float x[4], y[4];
		for (int j = 0; j < 4; ++j) {
			x[j] = temp[j].x;
			y[j] = temp[j].y;
		}
		Matx<float,6, 1> temp_score;
		float d = 0;
		for (auto &j : lines2) {
			float angle = j.val[4] < 90 ? j.val[4] + 90 : j.val[4] - 90;
			bool flag1 = pnpoly(4, x, y, j.val[0], j.val[1]);
			bool flag2 = pnpoly(4, x, y, j.val[2], j.val[3]);
			if (flag1 || flag2) {
				for (int k = 1; k < 6; ++k) {
					bool flag = (line.val[4] < angle + k * 5) && (line.val[4] > angle - k * 5);
					if (flag) {
						temp_score.val[k - 1] += j.val[5];
					}
				}
				d += j.val[5];
			}
		}
		if (d == 0) {
			temp_score.val[5] = (float)9999;
			score.push_back(temp_score);
			continue;
		}
		temp_score.val[5] = d;
		//for (int j = 0; j < 5; ++j) {
		//	temp_score.val[j] = temp_score.val[j] / d;
		//}
		score.push_back(temp_score);
	}
	if (lines.size() < 1) {
		return vector<Matx<float, 6, 1>>();
	}
	Matx<float, 6, 1> result_line;
	int  max_index = 0;
	result_line = lines[0];
	for (int i = 1; i < lines.size(); ++i) {
		if (score[i].val[4] > score[max_index].val[4]) {
			max_index = i;
			result_line = lines[i] ;
		}
	}
	float max_score = (float)score[max_index].val[4] / (float)score[max_index].val[5];
	if (max_score  < 0.5) {
		return vector<Matx<float, 6, 1>>();
	}
	else {
		return vector<Matx<float, 6, 1>>{result_line};
	}
}

vector<Point2f> judge_parrallel_line(Matx<float, 6, 1> line1, Matx<float, 6, 1> line2)
{
	vector<Matx<float, 7, 1>> temp1 = project_line(line1, vector<Matx<float, 6, 1>>{line2});
	vector<Point2f> result;
	float k1, k2, x[4], y[4];
	k1 = temp1[0].val[4]; k2 = temp1[0].val[5];
	if ((k1 < 0) || (k1 > 1)) {
		x[0] = line1.val[0];
		y[0] = line1.val[1];
	}
	else {
		x[0] = temp1[0].val[0];
		y[0] = temp1[0].val[1];
	}
	if ((k2 < 0) || (k2 > 1)) {
		x[1] = line1.val[2];
		y[1] = line1.val[3];
	}
	else {
		x[1] = temp1[0].val[2];
		y[1] = temp1[0].val[3];
	}
	vector<Matx<float, 7, 1>> temp2 = project_line(line2, vector<Matx<float, 6, 1>>{line1});
	k1 = temp2[0].val[4]; k2 = temp2[0].val[5];
	if ((k1 < 0) || (k1 > 1)) {
		x[3] = line2.val[0];
		y[3] = line2.val[1];
	}
	else {
		x[3] = temp2[0].val[0];
		y[3] = temp2[0].val[1];
	}
	if ((k2 < 0) || (k2 > 1)) {
		x[2] = line2.val[2];
		y[2] = line2.val[3];
	}
	else {
		x[2] = temp2[0].val[2];
		y[2] = temp2[0].val[3];
	}
	for (int i = 0; i < 4; ++i) {
		Point2f temp_point(x[i],y[i]);
		result.push_back(temp_point);
	}

	Mat test_im = Mat::zeros(Size(4000, 4000), CV_8UC3);
	test_im = draw_line(test_im, vector<Matx<float, 6, 1>>{line1}, Scalar(255, 0, 0));
	test_im = draw_line(test_im, vector<Matx<float, 6, 1>>{line2}, Scalar(0, 0, 255));
	line(test_im, result[0], result[1], Scalar(0, 255, 0), 3);
	line(test_im, result[2], result[3], Scalar(0, 255, 0), 3);
	return result;
}

vector<Matx<float, 6, 1>> intersect_line(vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2)
{
	vector<Matx<float, 6, 1>> result;
	float error_t = (float)0.001;
	for (auto &i : lines1) {
		bool flag = true;
		for (auto &j : lines2) {
			if ((i.val[0] < j.val[0] + error_t) && (i.val[0] > j.val[0] - error_t) &&
				(i.val[1] < j.val[1] + error_t) && (i.val[1] > j.val[1] - error_t) &&
				(i.val[2] < j.val[2] + error_t) && (i.val[2] > j.val[2] - error_t) &&
				(i.val[3] < j.val[3] + error_t) && (i.val[3] > j.val[3] - error_t) &&
				(i.val[4] < j.val[4] + error_t) && (i.val[4] > j.val[4] - error_t) &&
				(i.val[5] < j.val[5] + error_t) && (i.val[5] > j.val[5] - error_t)) {
				flag = false;
			}
		}
		if (flag) {
			result.push_back(i);
		}
	}

	return result;
}

bool pnpoly(int nvert, float * vertx, float * verty, float testx, float testy)
{
	int i, j;
	bool c = false;
	for (i = 0, j = nvert - 1; i < nvert; j = i++) {
		if (((verty[i] > testy) != (verty[j] > testy)) &&
			(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
			c = !c;
	}
	return c;
}

vector<water_result> segement_area(Mat I, vector<vector<Mat>> &model)
{
	Mat test_im;
	vector<Matx<float, 6, 1>> lines1, lines2;
	get_line(I, lines1, lines2);
	auto parallel_lines = get_parallel_lines(I, lines1, lines2);

	// 延伸平行线中的端点，直到图像边界
	test_im = draw_line(I, parallel_lines);
	parallel_lines = extend_line(I, parallel_lines);
	// 去除重叠区域
	parallel_lines = subtract_iou(I, parallel_lines);

	test_im = draw_line(I, parallel_lines);
	if ((lines1.size() == 0) || (lines2.size() == 0) || (parallel_lines.size() == 0))
		return vector<water_result>();
	vector<water_result> result;
	Mat line1(2, 4, CV_32F), line2(2, 4, CV_32F);
	for (auto &i : parallel_lines) {
		Mat temp(2, 4, CV_32F);
		temp.setTo(0);
		temp.at<float>(0, 0) = i.val[0]; temp.at<float>(1, 0) = i.val[1];
		temp.at<float>(0, 1) = i.val[2]; temp.at<float>(1, 1) = i.val[3];
		line1 = temp.clone();
		temp.setTo(0);
		temp.at<float>(0, 0) = i.val[6]; temp.at<float>(1, 0) = i.val[7];
		temp.at<float>(0, 1) = i.val[8]; temp.at<float>(1, 1) = i.val[9];
		line2 = temp.clone();
		// 旋转原始影像
		float angle = (i.val[4] + i.val[10]) / 2;
		Point2d center(I.cols / 2.0, I.rows / 2.0);
		Mat r = getRotationMatrix2D(center, 90 - angle, 1.0);
		Rect bbox = RotatedRect(center, I.size(), 90 - angle).boundingRect();
		r.at<double>(0, 2) += bbox.width / 2.0 - center.x;
		r.at<double>(1, 2) += bbox.height / 2.0 - center.y;
		Mat image_rotate;
		cv::warpAffine(I, image_rotate, r, bbox.size());
		// 截取大区域
		r.convertTo(r, CV_32F);
		for (int j = 0; j < 2; ++j) {
			Mat temp;
			line1.col(j).copyTo(temp);
			temp = r.colRange(0, 2) * temp + r.colRange(2, 3);
			temp.copyTo(line1.col(j + 2));
			line2.col(j).copyTo(temp);
			temp = r.colRange(0, 2) * temp + r.colRange(2, 3);
			temp.copyTo(line2.col(j + 2));
		}
		int r1, r2, c1, c2;
		double temp1, temp2;
		line1.colRange(2, 4).copyTo(temp.colRange(0, 2));
		line2.colRange(2, 4).copyTo(temp.colRange(2, 4));
		cv::minMaxIdx(temp.row(0), &temp1, &temp2);
		c1 = (int)round(temp1) >= 0 ? (int)round(temp1) : 0;
		c2 = (int)round(temp2) <= image_rotate.cols ? (int)round(temp2) : image_rotate.cols;
		temp1 = (double)temp.at<float>(1, 0);
		temp2 = (double)temp.at<float>(1, 2);
		r1 = temp1 >= temp2 ? (int)ceil(temp1) : (int)ceil(temp2);
		temp1 = (double)temp.at<float>(1, 1);
		temp2 = (double)temp.at<float>(1, 3);
		r2 = temp1 <= temp2 ? (int)floor(temp1) : (int)floor(temp2);
		r2 = r2 <= image_rotate.rows ? r2 : image_rotate.rows;
		Mat data = image_rotate(Range(r1, r2), Range(c1, c2));
		// 精细化小区域
		Mat location = sub_water_area(data, line1, line2);
		if (location.total() == 0)
			continue;
		// 计算e点确切位置
		vector<vector<float>> points = compute_e_point(data, location);
		data = data(Range(0, data.rows), Range(location.at<int>(0, 0), location.at<int>(0, 1) + 1));

		test_im = draw_point(data, points);

		if (points.size() == 0)
			continue;
		vector<vector<float>> number = number_area_recognition(data, points, model);
		if (number.size() != 0) {
			vector<float> temp;
			for (int i = 0; i < number.size(); ++i)
				if ((number[i])[0] > 9.5&&i!=0)
					temp.push_back((number[i])[0]);
			int n = temp.size();
			(number[0])[0] += n * 10;
			for (int i = 0; i < number.size(); ++i) {
				(number[i])[0] = (float)((number[0])[0] - i * 0.5);
			}
				
		}
		// 得出水线
		Mat mask;
		float water_line = 0;

		// 得到数值
		vector<float> temp_water;
		vector<float> temp_p;
		float p = 0, water_number = 0;
		for (int i = 1; i < points.size(); ++i) {
			if ((*(points.begin() + i - 1))[1] > water_line)
				break;
			float x1, x2, y1, y2;
			x1 = (*(points.begin() + i - 1))[1]; x2 = (*(points.begin() + i))[1];
			y1 = (*(number.begin() + i - 1))[0]; y2 = (*(number.begin() + i))[0];
			float temp = 0;
			temp = 1 / (water_line - x1);
			temp_p.push_back(temp);
			p += temp;
			temp = y1 + (y2 - y1) / (x2 - x1)*(water_line - x1);
			temp_water.push_back(temp);
			water_number += temp / (water_line - x1);
		}
		water_number = water_number / p;

		water_result water_area;
		//
		float x, y;
		//
		temp_water.clear();
		x = c1 + location.at<int>(0,0);y = r1;
		temp_p = rotated2unrotated_point(Point2f(x, y), r);
		temp_water.insert(temp_water.end(), temp_p.begin(), temp_p.end());
		x = c1 + location.at<int>(0,0);y = r2;
		temp_p = rotated2unrotated_point(Point2f(x, y), r);
		temp_water.insert(temp_water.end(), temp_p.begin(), temp_p.end());
		x = c1 + location.at<int>(0,1);y = r1;
		temp_p = rotated2unrotated_point(Point2f(x, y), r);
		temp_water.insert(temp_water.end(), temp_p.begin(), temp_p.end());
		x = c1 + location.at<int>(0,1);y = r2;
		temp_p = rotated2unrotated_point(Point2f(x, y), r);
		temp_water.insert(temp_water.end(), temp_p.begin(), temp_p.end());
		water_area.parrallel_lines = temp_water;
		// 水面线
		temp_water.clear();
		x = c1 + location.at<int>(0, 0) + 0.5; y = water_line + r1;
		temp_p = rotated2unrotated_point(Point2f(x, y), r);
		temp_water.insert(temp_water.end(), temp_p.begin(), temp_p.end());
		x = c1 + location.at<int>(0, 0)+data.cols-0.5; y = water_line + r1;
		temp_p = rotated2unrotated_point(Point2f(x, y), r);
		temp_water.insert(temp_water.end(), temp_p.begin(), temp_p.end());
		water_area.water_lines = temp_water;
		//
		water_area.data = data.clone();
		water_area.number = number;
		for (int i = 0; i < points.size(); ++i) {
			vector<float> temp = rotated2unrotated_point(Point2f(c1 + location.at<int>(0, 0)+(points[i])[0], (points[i])[1]+r1), r);
			temp.push_back((points[i])[2]);
			temp.push_back((points[i])[3]);
			water_area.points.push_back(temp);
		}
	
		water_area.water_number = water_number;
		water_area.water_line = (int)water_line;
		result.push_back(water_area);
	}
	return result;
}

vector<Matx<float, 12, 1>> extend_line(Mat I, vector<Matx<float, 12, 1>> parallel_lines)
{
	for (auto &i : parallel_lines) {
		for (int j = 0; j < 2; ++j) {
			Mat temp(2, 2, CV_32F);
			float k = (i.val[6 * j + 3] - i.val[6 * j + 1]) / (i.val[6 * j + 2] - i.val[6 * j + 0]);
			temp.at<float>(0, 0) = (float)(0.5 - i.val[6 * j + 1]) / k + i.val[6 * j + 0];
			temp.at<float>(1, 0) = (float)0.5;
			temp.at<float>(0, 1) = (float)(I.rows - 0.5 - i.val[6 * j + 1]) / k + i.val[6 * j + 0];
			temp.at<float>(1, 1) = (float)(I.rows - 0.5);
			if (temp.at<float>(0, 0) < 0.5) {
				temp.at<float>(0, 0) = (float)0.5;
				temp.at<float>(1, 0) = (float)(0.5 - i.val[6 * j + 0]) * k + i.val[6 * j + 1];
			}
			else if (temp.at<float>(0, 0) > I.cols - 0.5) {
				temp.at<float>(0, 0) = (float)(I.cols - 0.5);
				temp.at<float>(0, 0) = (float)(I.cols - 0.5 - i.val[6 * j + 0]) * k + i.val[6 * j + 1];
			}
			if (temp.at<float>(0, 1) < 0.5) {
				temp.at<float>(0, 1) = (float)0.5;
				temp.at<float>(1, 1) = (float)(0.5 - i.val[6 * j + 0]) * k + i.val[6 * j + 1];
			}
			else if (temp.at<float>(0, 1) > I.cols - 0.5) {
				temp.at<float>(0, 1) = (float)(I.cols - 0.5);
				temp.at<float>(1, 1) = (float)(I.cols - 0.5 - i.val[6 * j + 0]) * k + i.val[6 * j + 1];
			}
			i.val[6 * j + 0] = temp.at<float>(0, 0);
			i.val[6 * j + 1] = temp.at<float>(1, 0);
			i.val[6 * j + 2] = temp.at<float>(0, 1);
			i.val[6 * j + 3] = temp.at<float>(1, 1);
			i.val[6 * j + 5] = sqrt(pow(i.val[6 * j + 0] - i.val[6 * j + 2], 2) + pow(i.val[6 * j + 1] - i.val[6 * j + 3], 2));
		}
	}

	return parallel_lines;
}

vector<Matx<float, 12, 1>> subtract_iou(Mat I, vector<Matx<float, 12, 1>> parallel_lines)
{
	vector<bool> flag(parallel_lines.size(), true);
	Mat iou = Mat::zeros(Size((int)parallel_lines.size(), (int)parallel_lines.size()), CV_32F);

	for (int i = 0; i < parallel_lines.size(); ++i) {
		int n1 = 0, n2 = 0, n3 = 0;
		Mat ShapeAImage = Mat::zeros(Size(I.cols + 10, I.rows + 10), CV_8U);
		vector<Point> temp;
		temp.push_back(Point((int)parallel_lines[i].val[0], (int)parallel_lines[i].val[1]));
		temp.push_back(Point((int)parallel_lines[i].val[2], (int)parallel_lines[i].val[3]));
		temp.push_back(Point((int)parallel_lines[i].val[8], (int)parallel_lines[i].val[9]));
		temp.push_back(Point((int)parallel_lines[i].val[6], (int)parallel_lines[i].val[7]));
		vector<vector<Point>> temp_points{ temp };
		fillPoly(ShapeAImage, temp_points, Scalar(128));
		n1 = countNonZero(ShapeAImage);
		for (int j = i + 1; j < parallel_lines.size(); ++j) {
			Mat ShapeBImage = Mat::zeros(Size(I.cols + 10, I.rows + 10), CV_8U);
			vector<Point> temp;
			temp.push_back(Point((int)parallel_lines[j].val[0], (int)parallel_lines[j].val[1]));
			temp.push_back(Point((int)parallel_lines[j].val[2], (int)parallel_lines[j].val[3]));
			temp.push_back(Point((int)parallel_lines[j].val[8], (int)parallel_lines[j].val[9]));
			temp.push_back(Point((int)parallel_lines[j].val[6], (int)parallel_lines[j].val[7]));
			vector<vector<Point>> temp_points{ temp };
			fillPoly(ShapeBImage, temp_points, Scalar(128));
			Mat temp_image;
			n2 = countNonZero(ShapeBImage);
			bitwise_and(ShapeAImage, ShapeBImage, temp_image);
			n3 = countNonZero(temp_image);
			iou.at<float>(i, j) = (float)n3 / (float)n1;
			if (iou.at<float>(i, j) > 0.5) {
				flag[j] = false;
			}
		}
	}
	vector<Matx<float, 12, 1>> result;
	for (int i = 0; i < parallel_lines.size(); ++i) {
		if (flag[i])
			result.push_back(parallel_lines[i]);
	}
	return result;
}

Mat sub_water_area(Mat I, Mat &line1, Mat &line2)
{
	Mat test_im;
	// 检测直线
	vector<Matx<float, 6, 1>> lines1, lines2;
	get_line(I, 5, 22.5, lines1, lines2);
	// 利用规则进行过滤
	// 不能过短
	float length_t = (float)(I.cols / 5.0);
	for (auto i = lines1.begin(); i != lines1.end(); ++i) {
		if ((*i).val[5] < length_t) {
			lines1.erase(i, lines1.end());
			break;
		}
	}
	for (auto i = lines2.begin(); i != lines2.end(); ++i) {
		if ((*i).val[5] < length_t) {
			lines2.erase(i, lines2.end());
			break;
		}
	}
	// 水平线不能过长
	length_t = (float)(2.0*I.cols / 3.0);
	for (auto i = lines2.begin(); i != lines2.end(); ++i) {
		if ((*i).val[5] < length_t) {
			lines2.erase(lines2.begin(), i);
			break;
		}
	}
	if ((lines1.size() == 0) || lines2.size() == 0)
		return Mat();
	// 竖直直线应该在左右两侧才有意义
	vector<Matx<float, 6, 1>> temp;
	length_t = (float)(2.0*I.cols / 3.0);
	for (auto &i : lines1) {
		float p_location = (float)((i.val[0] + i.val[2]) / 2.0);
		if ((p_location < (float)I.cols - length_t) || (p_location > length_t)) {
			temp.push_back(i);
		}
	}
	lines1 = temp;
	if ((lines1.size() == 0) || lines2.size() == 0)
		return Mat();
	// 竖直直线投影到竖直方向上
	lines2 = select_h_lines(I, lines1, lines2);
	// 求解出边界
	if (lines2.size() == 0)
		return Mat();
	Mat location = get_e_boundary(I, lines2);
	return location;
}

vector<Matx<float, 6, 1>> select_h_lines(Mat I, vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2)
{
	Mat data(I.rows, 1, CV_32F, Scalar(0));
	for (auto &i : lines1) {
		int y1, y2;
		y1 = round(i.val[1]) >= 0 ? (int)round(i.val[1]) : 0;
		y2 = round(i.val[3]) <= I.rows ? (int)round(i.val[3]) : (int)I.rows;
		data.rowRange(y1, y2).setTo(1);
	}
	// 
	Mat Kernel = (cv::Mat_<float>(2, 1) << -1, 1);
	Mat dy;
	filter2D(data, dy, -1, Kernel, cv::Point(-1, -1), 0.0, cv::BORDER_CONSTANT);
	//
	vector<Matx<int, 3, 1>> index;
	int y1 = 0, y2 = 0;
	for (int i = 0; i < dy.rows; ++i) {
		if (dy.at<float>(i, 0) > 0.5) {
			y1 = i;
		}
		if (dy.at<float>(i, 0) < -0.5) {
			y2 = i;
			index.push_back(Matx<int, 3, 1>(y1, y2, y2 - y1 + 1));
		}
	}
	if (y1 > (*(index.end() - 1)).val[1]) {
		y2 = I.rows - 1;
		index.push_back(Matx<int, 3, 1>(y1, y2, y2 - y1 + 1));
	}
	stable_sort(index.begin(), index.end(),
		[](const Matx<float, 3, 1>&a, const Matx<float, 3, 1>&b) {return a.val[2] > b.val[2]; });
	if (index.size() < 1) {
		return vector<Matx<float, 6, 1>>();
	}
	y1 = (*index.begin()).val[0];	y2 = (*index.begin()).val[1];
	//
	bool flag = false;
	vector<Matx<float, 6, 1>> result;
	for (auto &i : lines2)
	{
		flag = (i.val[1] >= y1) && (i.val[1] <= y2) && (i.val[3] >= y1) && (i.val[3] <= y2);
		if (flag) {
			result.push_back(i);
			flag = false;
		}
	}
	return result;
}

Mat get_e_boundary(Mat I, vector<Matx<float, 6, 1>> lines)
{
	Mat temp(1, I.cols, CV_32F, Scalar(0));
	int x1, x2;
	for (auto &i : lines) {
		x1 = (int)round(i.val[0]) >= 0 ? (int)round(i.val[0]) : 0;
		x2 = (int)round(i.val[2]) <= I.cols ? (int)round(i.val[2]) : I.cols;
		temp.colRange(x1, x2) += 1.0;
	}
	vector<int> index;
	for (int i = 0; i < I.cols; ++i) {
		if (temp.at<float>(0, i) > 3) {
			index.push_back(i);
		}
	}
	if (index.size() == 0)
		return Mat();
	Mat data(3, 3, CV_32S, Scalar(0));
	// 中
	data.at<int>(0, 0) = *(index.begin());
	data.at<int>(0, 1) = *(index.end() - 1);
	data.at<int>(0, 2) = data.at<int>(0, 1) - data.at<int>(0, 0);
	// 左边
	data.at<int>(1, 0) = *(index.begin());
	data.at<int>(1, 1) = (int)round(2 * (*(index.end() - 1)) - *(index.begin()));
	data.at<int>(1, 2) = data.at<int>(1, 1) - data.at<int>(1, 0);
	// 右边
	data.at<int>(2, 0) = (int)round(2 * (*index.begin()) - *(index.end() - 1));
	data.at<int>(2, 1) = (*(index.end() - 1));
	data.at<int>(2, 2) = data.at<int>(2, 1) - data.at<int>(2, 0);
	for (int i = 0; i < 3; ++i) {
		if (data.at<int>(i, 2) < 15) {
			data.at<int>(i, 2) = -1;
			continue;
		}
		if ((data.at<int>(i, 0) < -5) || (data.at<int>(i, 1) > 9.0*I.cols / 8.0)) {
			data.at<int>(i, 2) = -1;
			continue;
		}
	}
	Mat location(1, 3, CV_32S, Scalar(0));
	for (int i = 0; i < 3; ++i) {
		if (data.at<int>(i, 2) > location.at<int>(0, 2)) {
			location.at<int>(0, 0) = data.at<int>(i, 0);
			location.at<int>(0, 1) = data.at<int>(i, 1);
			location.at<int>(0, 2) = data.at<int>(i, 2);
		}
	}
	location.at<int>(0, 0) = location.at<int>(0, 0) < 0 ? 0 : location.at<int>(0, 0);
	location.at<int>(0, 1) = location.at<int>(0, 1) >= I.cols ? I.cols - 1 : location.at<int>(0, 1);
	location.at<int>(0, 2) = location.at<int>(0, 1) - location.at<int>(0, 0);
	if (location.at<int>(0, 2) < 10)
		return  Mat();
	return location;
}

vector<float> rotated2unrotated_point(Point2f point, Mat r)
{
	// 平行线
	Mat invert_r = Mat::zeros(Size(3,3),CV_32F);
	r.copyTo(invert_r.rowRange(0, 2));
	invert_r.at<float>(2, 2) = 1;
	invert(invert_r, invert_r);
	float x, y;
	x = invert_r.at<float>(0, 0)*point.x + invert_r.at<float>(0, 1)*point.y + invert_r.at<float>(0, 2);
	y = invert_r.at<float>(1, 0)*point.x + invert_r.at<float>(1, 1)*point.y + invert_r.at<float>(1, 2);
	vector<float> temp;
	temp.push_back(x);
	temp.push_back(y);
	return temp;
}

vector<vector<float>> compute_e_point(Mat I, Mat location)
{
	Mat test_im;
	I = I(Range(0, I.rows), Range(location.at<int>(0, 0), location.at<int>(0, 1) + 1));
	// 对原始影像进行缩放
	Mat im = I;
	float scale = (float)(I.cols / 75.0);
	resize(im, im, Size(75, I.rows), INTER_LINEAR);
	//// 模板制作
	vector<Mat> xy_filter, score_image;
	Mat filter;
	int h_w_size, w_size;
	//中心点模板
	h_w_size = (int)round(I.cols / 6.0);
	w_size = 2 * h_w_size + 1;
	filter = Mat::zeros(w_size, 31, CV_32F);
	filter(Range(h_w_size - 1, h_w_size + 2), Range(0, 16)).setTo(-1.0 / 48.0);
	filter(Range(0, filter.rows), Range(16, 31)).setTo(1.0 / (15.0*w_size));
	xy_filter.push_back(filter);
	// 图示上点模板
	h_w_size = (int)round(I.cols / 2.0);
	w_size = 2 * h_w_size + 1;
	filter = Mat::zeros(w_size, 31, CV_32F);
	filter(Range(h_w_size - 1, h_w_size), Range(0, 16)).setTo(1.0 / (15.0*(h_w_size + 1) + 16));
	filter(Range(h_w_size, h_w_size + 2), Range(0, 16)).setTo(-1.0 / 32);
	filter(Range(h_w_size, filter.rows), Range(16, 31)).setTo(1.0 / (15.0*(h_w_size + 1) + 16));
	xy_filter.push_back(filter.clone());
	// 图示下点模板
	flip(filter, filter, 0);
	xy_filter.push_back(filter.clone());
	// 得到响应值图
	Mat im_gray;
	cvtColor(im, im_gray, CV_BGR2GRAY);
	im_gray.convertTo(im_gray, CV_32F);
	for (auto &i : xy_filter) {
		Mat temp;
		filter2D(im_gray, temp, -1, i, cv::Point(-1, -1), 0.0, cv::BORDER_CONSTANT);
		score_image.push_back(temp);
	}
	for (auto &i : score_image)
		for (auto &j : score_image)
			i.setTo(0, i < j);
	// 非极大值抑制
	vector<vector<Point2i>> points;
	float d_t = (float)(0.8*I.cols);
	for (auto &i : score_image) {
		vector<Point2i> temp = localmax_point(i, d_t, (float)0.2);
		points.push_back(temp);
	}
	// 三点建立联系
	if (points.size() == 0)
		return vector<vector<float>>();
	vector<Mat> e_proposal_points = get_e_proposal_points(I, points);
	test_im = draw_point(im, e_proposal_points);
	// 对候选区域进行处理，优化e点,以确定E左右中点的位置，从而在某种程度上精确定位
	if (e_proposal_points.size() == 0)
		return vector<vector<float>>();
	vector<vector<float>> result = better_e_points(im, e_proposal_points);
	if (result.size() == 0)
		return vector<vector<float>>();
	// 同比缩放
	int x = (int)round((*result.begin())[0] * scale);
	for (auto &i : result) {
		i[0] = (float)x;
	}
	return result;
}

vector<Point2i> localmax_point(Mat score_image, float d_t, float scale)
{
	// 获取所有可能的坐标
	vector<Point3f> data;
	vector<Point2i> point;
	vector<float> score;
	//去除非中心位置
	int x1 = (int)round(score_image.cols / 3.0);
	int x2 = (int)round(2.0*score_image.cols / 3.0);
	for (int i = x1; i < x2; ++i)
		for (int j = 0; j < score_image.rows; ++j) {
			if (score_image.at<float>(j, i) > 0) {
				data.push_back(Point3f((float)i, (float)j, score_image.at<float>(j, i)));
			}
		}
	int x, y;
	while (data.size() > 0) {
		vector<Point3f> temp;
		auto index = max_element(data.begin(), data.end(),
			[](const Point3f&a, const Point3f&b) {return a.z < b.z; });
		temp.push_back(*index);
		x = (int)temp[0].x; y = (int)temp[0].y;
		point.push_back(Point2i(x, y));
		score.push_back(temp[0].z);
		data.erase(index);
		temp.clear();

		for (auto &i : data) {
			if (abs(y - i.y) > d_t) {
				temp.push_back(i);
			}
		}
		data = temp;
		temp.clear();
	}
	float score_t = score[0] * scale;
	for (int i = 0; i < point.size(); ++i) {
		if (score[i] < score_t) {
			point.erase(point.begin() + i, point.end());
			break;
		}
	}
	return point;
}
vector<Point3f> localmax_point_score(Mat score_image, float d_t, float scale)
{
	vector<Point3f> result;
	//去除非中心位置
	int x1 = (int)round(score_image.cols / 3.0);
	int x2 = (int)round(2.0*score_image.cols / 3.0);
	result = localmax_point_score(score_image, x1, x2, d_t, scale);
	return result;
}
vector<Point3f> localmax_point_score(Mat score_image, int x1, int x2, float d_t, float scale)
{
	// 获取所有可能的坐标
	vector<Point3f> data, result;
	for (int i = x1; i < x2; ++i)
		for (int j = 0; j < score_image.rows; ++j) {
			if (score_image.at<float>(j, i) > 0) {
				data.push_back(Point3f((float)i, (float)j, score_image.at<float>(j, i)));
			}
		}
	int x, y;
	while (data.size() > 0) {
		vector<Point3f> temp;
		auto index = max_element(data.begin(), data.end(),
			[](const Point3f&a, const Point3f&b) {return a.z < b.z; });
		temp.push_back(*index);
		result.push_back(*index);
		x = (int)temp[0].x; y = (int)temp[0].y;
		data.erase(index);
		temp.clear();

		for (auto &i : data) {
			if (abs(y - i.y) > d_t) {
				temp.push_back(i);
			}
		}
		data = temp;
		temp.clear();
	}
	stable_sort(result.begin(), result.end(),
		[](const Point3f&a, const Point3f&b) {return a.z > b.z; });
	float score_t = result[0].y * scale;
	for (int i = 0; i < result.size(); ++i) {
		if (result[i].y < score_t) {
			result.erase(result.begin() + i, result.end());
			break;
		}
	}
	return result;
}
vector<Mat> get_e_proposal_points(Mat im, vector<vector<Point2i>> points)
{

	vector<Mat> result;
	vector<Point2i> point_0 = points[0], point_1 = points[1], point_2 = points[2];
	vector<Point2i> temp;

	// 根据规则，与另外两点建立联系，中点
	for (int i = 0; i < point_0.size(); ++i) {
		int xo = point_0[i].x, yo = point_0[i].y;
		int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
		int temp1 = -1, temp2 = -1;
		for (int j = 0; j < point_1.size(); ++j) {
			x1 = point_1[j].x;
			y1 = point_1[j].y;
			bool flag = abs(x1 - xo) < 5 && y1 - yo > -im.cols / 2.0&&y1 - yo < -2;
			if (flag) {
				temp1 = j;
				break;
			}
		}
		for (int j = 0; j < point_2.size(); ++j) {
			x2 = point_2[j].x;
			y2 = point_2[j].y;
			bool flag = abs(x2 - xo) < 5 && y2 - yo < im.cols / 2.0&&y2 - yo >2;
			if (flag) {
				temp2 = j;
				break;
			}
		}
		if ((abs(y2 + y1 - 2 * yo) > 5) || (abs(x2 + x1 - 2 * xo) > 5) || temp1 == -1 || temp2 == -1||y2-y1<im.cols/3) {
			temp.push_back(point_0[i]);
			continue;
		}
		
		point_1.erase(point_1.begin() + temp1);
		point_2.erase(point_2.begin() + temp2);
		Mat temp = (Mat_<int>(3, 2) << xo, yo, x1, y1, x2, y2);
		result.push_back(temp);
	}
	point_0 = temp;
	temp.clear();

	// 根据规则，另两点建立联系
	for (int i = 0; i < point_1.size(); ++i) {
		int x1 = point_1[i].x, y1 = point_1[i].y;
		int xo = 0, yo = 0, x2 = 0, y2 = 0;
		int temp2 = -1;
		for (int j = 0; j < point_2.size(); ++j) {
			x2 = point_2[j].x;
			y2 = point_2[j].y;
			bool flag = abs(x2 - x1) < 5 && y2 - y1 < im.cols &&y2 - y1 > im.cols/3;
			if (flag) {
				temp2 = j;
				break;
			}
		}
		if (temp2 == -1) {
			temp.push_back(point_1[i]);
			continue;
		}
		xo = (x1 + x2) / 2;
		yo = (y1 + y2) / 2;
		point_2.erase(point_2.begin() + temp2);
		Mat temp = (Mat_<int>(3, 2) << xo, yo, x1, y1, x2, y2);
		result.push_back(temp);
	}
	point_1 = temp;
	temp.clear();

	// 根据规则，余下中点分别与另两点建立联系
	for (int i = 0; i < point_0.size(); ++i) {
		int xo = point_0[i].x, yo = point_0[i].y;
		int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
		int temp1 = -1;
		for (int j = 0; j < point_1.size(); ++j) {
			x1 = point_1[j].x;
			y1 = point_1[j].y;
			bool flag = abs(x1 - xo) < 5 && y1 - yo > - im.cols / 2.0&&y1 - yo <-2;
			if (flag) {
				temp1 = j;
				break;
			}
		}
		if (temp1 == -1 || yo - y1<im.cols / 6) {
			temp.push_back(point_0[i]);
			continue;
		}
		x2 = (3*xo-x1) / 2;
		y2 = (3 * yo - y1 ) / 2;
		point_1.erase(point_1.begin() + temp1);
		Mat temp = (Mat_<int>(3, 2) << xo, yo, x1, y1, x2, y2);
		result.push_back(temp);
	}
	point_0 = temp;
	temp.clear();
	for (int i = 0; i < point_0.size(); ++i) {
		int xo = point_0[i].x, yo = point_0[i].y;
		int x1 = 0, y1 = 0, x2 = 0, y2 = 0;
		int temp2 = -1;
		for (int j = 0; j < point_2.size(); ++j) {
			x2 = point_2[j].x;
			y2 = point_2[j].y;
			bool flag = abs(x2 - xo) < 5 && y2 - yo < im.cols / 2.0&&y2 - yo >2;
			if (flag) {
				temp2 = j;
				break;
			}
		}
		if (temp2 == -1||y2-yo<im.cols/6) {
			temp.push_back(point_0[i]);
			continue;
		}
		x1 = (3 * xo - x2) / 2;
		y1 = (3 * yo - y2) / 2;
		point_2.erase(point_2.begin() + temp2);
		Mat temp = (Mat_<int>(3, 2) << xo, yo, x1, y1, x2, y2);
		result.push_back(temp);
	}
	point_1 = temp;
	temp.clear();

	return result;
}

vector<vector<float>> better_e_points(Mat im, vector<Mat> points)
{
	vector<Mat> data1, data2;
	vector<int> point_x;
	// 裁取区域
	for (int i = 0; i < points.size(); ++i) {
		Mat temp_point = *(points.begin() + i);
		int xo = 0;
		int y1 = 9999;
		int y2 = -9999;
		for (int j = 0; j < 3; ++j) {
			xo = xo > temp_point.at<int>(j, 0) ? xo : temp_point.at<int>(j, 0);
			y1 = y1 < temp_point.at<int>(j, 1) ? y1 : temp_point.at<int>(j, 1);
			y2 = y2 > temp_point.at<int>(j, 1) ? y2 : temp_point.at<int>(j, 1);
		}
		Mat temp;
		temp = im(Range(y1, y2 + 1), Range(0, xo + 1));
		data1.push_back(temp.clone());
		temp = im(Range(y1, y2 + 1), Range(xo + 1, im.cols));
		data2.push_back(temp.clone());
		point_x.push_back(xo);
	}
	// 计算互相的相互关系
	Mat corr_matrix1 = Mat::zeros((int)points.size(), (int)points.size(), CV_32F);
	Mat	corr_matrix2 = Mat::zeros((int)points.size(), (int)points.size(), CV_32F);
	for (int i = 0; i < points.size(); ++i)
		for (int j = 0; j < points.size(); ++j) {
			if (i == j) {
				continue;
			}
			Mat temp1, temp2;
			Mat result;
			(*(data1.begin() + i)).copyTo(temp1);
			(*(data1.begin() + j)).copyTo(temp2);
			resize(temp2, temp2, temp1.size(), cv::INTER_LINEAR);
			matchTemplate(temp1, temp2, result, CV_TM_CCOEFF_NORMED);
			corr_matrix1.at<float>(i, j) = result.at<float>(0, 0);
			(*(data2.begin() + i)).copyTo(temp1);
			(*(data2.begin() + j)).copyTo(temp2);
			resize(temp2, temp2, temp1.size(), cv::INTER_LINEAR);
			matchTemplate(temp1, temp2, result, CV_TM_CCOEFF_NORMED);
			corr_matrix2.at<float>(i, j) = result.at<float>(0, 0);
		}
	//
	vector<int> index1 = class_score(corr_matrix1.clone(),1,(float)(0.4));
	vector<int> index2 = class_score(corr_matrix2.clone(), 1, (float)(0.2));
	vector<int> index;
	for (auto &i : index1) {
		if (find(index2.begin(), index2.end(), i) != index2.end())
			index.push_back(i);
	}
	stable_sort(index.begin(), index.end());
	if (index.size() == 0) {
		if (index1.size() != 0)
			index = index1;
		else
			return vector<vector<float>>();
	}
	//
	vector<Mat> temp1, temp2;
	int r1 = 0, r2 = 0, c1 = 0, c2 = 0;
	for (auto i : index) {
		temp1.push_back(*(data1.begin() + i));
		temp2.push_back(*(data2.begin() + i));
		r1 += (*(data1.begin() + i)).rows;
		c1 += (*(data1.begin() + i)).cols;
		r2 += (*(data2.begin() + i)).rows;
		c2 += (*(data2.begin() + i)).cols;
	}
	data1 = temp1;
	data2 = temp2;
	r1 = (int)round((float)r1 / (float)index.size());
	c1 = (int)round((float)c1 / (float)index.size());
	r2 = (int)round((float)r2 / (float)index.size());
	c2 = (int)round((float)c2 / (float)index.size());
	for (auto &i : data1) {
		resize(i, i, Size(r1, c1), INTER_LINEAR);
	}
	//for (auto &i : data2) {
	//	resize(i, i, Size(r1, c1), INTER_LINEAR);
	//}
	int x = 0;
	for (auto &i : index) {
		x += *(point_x.begin() + i);
	}
	x = (int)round((float)x / (float)index.size());
	vector<float> score1, score2;
	for (int i = 0; i < im.rows; ++i)
		if (i > round(r1 / 2) + 2 && i < im.rows - round(r1 / 2) - 2) {
			Mat temp1, temp2;
			temp1 = im(Range(i - (int)round(r1 / 2.0), i + (int)round(r1 / 2.0) + 1), Range(0, x)).clone();
			temp2 = im(Range(i - (int)round(r1 / 2.0), i + (int)round(r1 / 2.0) + 1), Range(x, im.cols)).clone();
			flip(temp2, temp2, -1);
			float temp;
			temp = corr_data(temp1, data1);
			score1.push_back(temp);
			temp = corr_data(temp2, data1);
			score2.push_back(temp);
		}
		else {
			score1.push_back((float)-3);
			score2.push_back((float)-3);
		}
		vector<vector<float>> result = get_e_location(im, score1, score2, (float)r1, (float)0.4, x);
		return result;
}

vector<vector<float>> get_e_location(Mat im, vector<float> score1, vector<float> score2, float distance_t, float score_t, int x)
{
	vector<vector<float>>  result;
	int y = -1;
	float score = -4.0;
	for (int i = 0; i < im.rows; ++i) {
		if (score < score1[i]) {
			score = score1[i];
			y = i;
		}
	}
	if (score < score_t) {
		return vector<vector<float>>();
	}
	vector<float> temp_result;
	temp_result.push_back((float)x);
	temp_result.push_back((float)y);
	temp_result.push_back((float)1);
	temp_result.push_back((float)score);
	result.push_back(temp_result);
	// 寻找上方的E
	while (1) {
		int temp_yo = 0, temp_y1 = 0, temp_y2 = 0;
		temp_yo = (int)(*result.begin())[1];
		float temp_score1 = -3, temp_score2 = -3;
		int y1, y2;
		y1 = (int)floor(temp_yo - 2.5*distance_t);
		y2 = (int)ceil(temp_yo - 1.5*distance_t);
		if ((y1 < 0) && (y2 < 0))
			break;
		else if (y1 < 0)
			y1 = 0;
		vector<float> temp1(score1.begin() + y1, score1.begin() + y2 + 1);
		for (int i = 0; i < temp1.size(); ++i) {
			if (temp_score1 < temp1[i]) {
				temp_score1 = temp1[i];
				temp_y1 = i;
			}
		}
		temp_y1 = temp_y1 + y1;

		vector<float> temp2(score2.begin() + temp_y1, score2.begin() + temp_yo + 1);
		for (int i = 0; i < temp2.size(); ++i) {
			if (temp_score2 < temp2[i]) {
				temp_score2 = temp2[i];
				temp_y2 = i;
			}
		}
		temp_y2 = temp_y1 + temp_y2;
		if ((temp_score1 < score_t) && (temp_score2 < score_t))
			break;
		if ((temp_score1 > score_t) && (temp_score2 > score_t) && abs(temp_y1 + temp_y2 - 2 * temp_yo) < 10) {
			temp_result[0] = (float)x; temp_result[1] = (float)temp_y2;
			temp_result[2] = (float)-1; temp_result[3] = (float)temp_score2;
			result.insert(result.begin(), temp_result);
			temp_result[0] = (float)x; temp_result[1] = (float)temp_y1;
			temp_result[2] = (float)1; temp_result[3] = (float)temp_score1;
			result.insert(result.begin(), temp_result);
			continue;
		}
		if (temp_score1 > score_t) {
			int temp_y = int(round((temp_y1 + temp_yo) / 2.0));
			float temp_score = score2[temp_y];
			if ((temp_score > score_t) && abs(temp_y - temp_yo) > 0.8*distance_t&& abs(temp_y - temp_yo) <1.5*distance_t) {
				temp_y2 = temp_y;
				temp_score2 = temp_score;
				temp_result[0] = (float)x; temp_result[1] = (float)temp_y2;
				temp_result[2] = (float)-1; temp_result[3] = (float)temp_score2;
				result.insert(result.begin(), temp_result);
				temp_result[0] = (float)x; temp_result[1] = (float)temp_y1;
				temp_result[2] = (float)1; temp_result[3] = (float)temp_score1;
				result.insert(result.begin(), temp_result);
				continue;
			}
		}
		if (temp_score2 > score_t) {
			int temp_y = 2 * temp_y2 - temp_yo;
			if (temp_y < 0)
				temp_y = 0;
			float temp_score = score1[temp_y];
			if ((temp_score > score_t) && abs(temp_y - temp_yo) < 1.5*distance_t&& abs(temp_y - temp_yo) > 0.8*distance_t) {
				temp_y1 = temp_y;
				temp_score1 = temp_score;
				temp_result[0] = (float)x; temp_result[1] = (float)temp_y2;
				temp_result[2] = (float)-1; temp_result[3] = (float)temp_score2;
				result.insert(result.begin(), temp_result);
				temp_result[0] = (float)x; temp_result[1] = (float)temp_y1;
				temp_result[2] = (float)1; temp_result[3] = (float)temp_score1;
				result.insert(result.begin(), temp_result);
				continue;
			}
		}
		break;
	}
	// 寻找下方的E
	while (1) {
		int temp_yo = 0, temp_y1 = 0, temp_y2 = 0;
		temp_yo = (int)(*(result.end() - 1))[1];
		float temp_score1 = -3, temp_score2 = -3;
		int y1, y2;
		y1 = (int)floor(temp_yo + 1.5*distance_t);
		y2 = (int)ceil(temp_yo + 2.5*distance_t);
		if ((y1 >= im.rows) && (y2 >= im.rows))
			break;
		else if (y2 >= im.rows)
			y2 = im.rows - 1;
		vector<float> temp1(score1.begin() + y1, score1.begin() + y2 + 1);
		for (int i = 0; i < temp1.size(); ++i) {
			if (temp_score1 < temp1[i]) {
				temp_score1 = temp1[i];
				temp_y1 = i;
			}
		}
		temp_y1 = temp_y1 + y1;

		vector<float> temp2(score2.begin() + temp_yo, score2.begin() + temp_y1 + 1);
		for (int i = 0; i < temp2.size(); ++i) {
			if (temp_score2 < temp2[i]) {
				temp_score2 = temp2[i];
				temp_y2 = i;
			}
		}
		temp_y2 = temp_yo + temp_y2;
		if ((temp_score1 < score_t) && (temp_score2 < score_t))
			break;
		if ((temp_score1 > score_t) && (temp_score2 > score_t) && abs(temp_y1 + temp_yo - 2 * temp_y2) < 10) {
			temp_result[0] = (float)x; temp_result[1] = (float)temp_y2;
			temp_result[2] = (float)-1; temp_result[3] = (float)temp_score2;
			result.insert(result.end(), temp_result);
			temp_result[0] = (float)x; temp_result[1] = (float)temp_y1;
			temp_result[2] = (float)1; temp_result[3] = (float)temp_score1;
			result.insert(result.end(), temp_result);
			continue;
		}
		if (temp_score1 > score_t) {
			int temp_y = int(round((temp_y1 + temp_yo) / 2));
			float temp_score = score2[temp_y];
			if ((temp_score > score_t) && abs(temp_y - temp_yo) > 0.8*distance_t&&abs(temp_y - temp_yo) <1.5*distance_t) {
				temp_y2 = temp_y;
				temp_score2 = temp_score;
				temp_result[0] = (float)x; temp_result[1] = (float)temp_y2;
				temp_result[2] = (float)-1; temp_result[3] = (float)temp_score2;
				result.insert(result.end(), temp_result);
				temp_result[0] = (float)x; temp_result[1] = (float)temp_y1;
				temp_result[2] = (float)1; temp_result[3] = (float)temp_score1;
				result.insert(result.end(), temp_result);
				continue;
			}
		}
		if (temp_score2 > score_t && abs(temp_y2 - temp_yo) > 0.8*distance_t&&abs(temp_y2 - temp_yo) <1.5*distance_t) {
			temp_result[0] = (float)x; temp_result[1] = (float)temp_y2;
			temp_result[2] = (float)-1; temp_result[3] = (float)temp_score2;
			result.insert(result.end(), temp_result);
			break;
		}
		break;
	}
	return result;
}

float corr_data(Mat im, vector<Mat> data)
{
	float score = -2;
	Mat temp = im.clone(), result;
	resize(temp, temp, data[0].size(), CV_INTER_LINEAR);
	for (auto &i : data) {
		matchTemplate(temp, i, result, CV_TM_CCOEFF_NORMED);
		score = score > result.at<float>(0, 0) ? score : result.at<float>(0, 0);
	}
	return score;
}
vector<int> class_score(Mat corr_matrix,int n, float value)
{
	vector<int> index;
	Mat temp_corr_matrix = corr_matrix.clone();
	Point maxloc;
	double maxval = 0;
	minMaxLoc(temp_corr_matrix, NULL, &maxval, NULL, &maxloc);
	Mat temp;
	int temp_index = maxloc.x;
	float score = (float)maxval;
	float score_t = 0;
	if (n == 1) {
		score_t = value *(float)maxval > (float)0.1 ? value * (float)maxval : (float)0.1;
		score_t = score_t > (float)0.4 ? score_t : (float)0.4;
	}
	else {
		score_t = value;
	}
	while (1) {
		if (score < score_t) {
			break;
		}
		else {
			index.push_back(temp_index);
			temp = temp_corr_matrix.row(temp_index).clone();
			temp.at<float>(0, temp_index) = -9;
			temp_corr_matrix.row(temp_index).setTo(-9);
			temp_corr_matrix.col(temp_index).setTo(-9);
			score = -1;
			for (int i = 0; i < temp.cols; ++i) {
				if (score < temp.at<float>(0, i)) {
					score = temp.at<float>(0, i);
					temp_index = i;
				}
			}
		}
	}
	return index;
}

vector<vector<float>> number_area_recognition(Mat data, vector<vector<float>> points, vector<vector<Mat>>& model)
{
	vector<vector<float>> number;
	for (auto i = points.begin(); i != points.end(); ++i) {
		vector<float> temp_result{ -1,-1 };
		if ((*i)[2] == 1) {
			number.push_back(temp_result);
			continue;
		}
		int x, y1, y2;
		x = (int)((*i)[0]);
		y1 = (int)round(((*(i - 1))[1] + (*i)[1]) / 2.0);
		y2 = (int)round((3 * (*i)[1] - (*(i - 1))[1]) / 2.0);
		Mat temp_im = data(Range(y1, y2 + 1), Range(0, x + 1));
		temp_result = number_recognition(temp_im, model);
		if (temp_result[0] < 0.5) {
			number.push_back(vector<float>{-1, -1});
			continue;
		}
		number.push_back(temp_result);
	}

	bool flag = true;
	for (auto &i : number)
		if (i[0] > -0.5)
			flag = false;
	if (flag)
		return vector<vector<float>>();
	vector<vector<float>> new_number = better_number_rec(number, points);

	return new_number;
}

vector<float> number_recognition(Mat data, vector<vector<Mat>> model)
{
	//
	Mat gray_image, logitic_image;
	cvtColor(data, gray_image, CV_BGR2GRAY);
	gray_image = 255 - gray_image;
	Mat im;
	im = gray_image.clone();
	resize(im, im, Size(24, 48), INTER_LINEAR);
	im.convertTo(im, CV_32F);
	normalize(im, im, 1.0, 0.0, NORM_MINMAX);
	//
	threshold(gray_image, logitic_image, 0, 255, THRESH_BINARY | THRESH_OTSU);
	int n_label = connectedComponents(logitic_image, logitic_image, 4);
	logitic_image.convertTo(logitic_image, CV_8U);
	int n_pixel_t = -99;
	Mat temp_im;
	for (int i = 1; i < n_label; ++i) {
		Mat temp = logitic_image.clone();
		temp.convertTo(temp, CV_8U);
		uchar* data1 = temp.ptr<uchar>(0);
		uchar* data2 = logitic_image.ptr<uchar>(0);
		int n_pixel = 0;
		for (int j = 0; j < temp.total(); ++j) {
			*(data1 + j) = *(data2 + j) == i ? 1 : 0;
			n_pixel += *(data1 + j);
		}
		if (n_pixel > n_pixel_t) {
			temp_im = temp.clone();
			n_pixel_t = n_pixel;
		}
	}
	Mat colSumVec, rowSumVec;
	reduce(temp_im, rowSumVec, 1, CV_REDUCE_SUM, CV_32S); // sum(img, 2)
	reduce(temp_im, colSumVec, 0, CV_REDUCE_SUM, CV_32S); // sum(img, 1)
	int r1 = 0, r2 = data.rows - 1, c1 = 0, c2 = data.cols - 1;
	// 行
	for (int j = 0; j < data.rows - 1; ++j) {
		if ((rowSumVec.at<int>(j, 0) == 0) && (rowSumVec.at<int>(j + 1, 0) != 0)) {
			r1 = j;
			break;
		}
	}
	for (int j = data.rows - 1; j > 0; --j) {
		if ((rowSumVec.at<int>(j, 0) == 0) && (rowSumVec.at<int>(j - 1, 0) != 0)) {
			r2 = j;
			break;
		}
	}
	// 列
	for (int j = 0; j < data.cols - 1; ++j) {
		if ((colSumVec.at<int>(0, j) == 0) && (colSumVec.at<int>(0, j + 1) != 0)) {
			c1 = j;
			break;
		}
	}
	for (int j = data.cols - 1; j > 0; --j) {
		if ((colSumVec.at<int>(0, j) == 0) && (colSumVec.at<int>(0, j - 1) != 0)) {
			c2 = j;
			break;
		}
	}
	if ((r2 - r1 + 1)*(c2 - c1 + 1) > temp_im.total() / 3) {
		im = gray_image(Range(r1, r2 + 1), Range(c1, c2 + 1)).clone();
		resize(im, im, Size(24, 48), INTER_LINEAR);
		im.convertTo(im, CV_32F);
		normalize(im, im, 1.0, 0.0, NORM_MINMAX);
	}

	vector<vector<float>> scores;
	vector<float> score(2, -1);
	for (int i = 0; i < model.size(); ++i) {
		float temp_score = -1;
		for (auto &j : model[i]) {
			Mat temp;
			matchTemplate(im, j, temp, CV_TM_CCOEFF_NORMED);
			temp_score = temp.at<float>(0, 0) > temp_score ? temp.at<float>(0, 0) : temp_score;
		}
		vector<float> temp(2, -1);
		temp[0] = (float)i; temp[1] = temp_score;
		scores.push_back(temp);
		if (temp_score > score[1]) {
			score = temp;
		}
	}
	return score;
}

vector<vector<float>> better_number_rec(vector<vector<float>> number, vector<vector<float>> points)
{
	int n = 0;
	vector<vector<float>> temp1, temp2;
	for (int i = 0; i < points.size(); ++i) {
		if ((*(points.begin() + i))[2] > 0)
			continue;
		else {
			temp1.push_back(*(number.begin() + i));
			temp2.push_back(*(points.begin() + i));
			n += 1;
		}
	}
	Mat scores = Mat::zeros(Size(10, n), CV_32F);
	for (int i = 0; i < n; ++i) {
		if ((temp1[i])[0] < 0)
			continue;
		for (int j = 0; j < n; ++j) {
			int index = (int)(temp1[i])[0] + i - j;
			index = index % 10;
			if (index < 0) {
				index += 10;
			}
			scores.at<float>(j, index) += (temp1[i])[1];
		}
	}
	for (int i = 0; i < n; ++i) {
		int index = -1; float score = -1;
		for (int j = 0; j < 10; ++j) {
			float temp = scores.at<float>(i, j);
			if (temp > score) {
				index = j;
				score = temp;
			}
		}
		(*(temp1.begin() + i))[0] = (float)index + (float)0.25;
		(*(temp1.begin() + i))[1] = score;
		if ((index == 9) && (temp1.size() > 15)) {
			for (int j = 0; j < i; ++j) {
				(*(temp1.begin() + i))[0] += 10;
			}
		}
	}
	vector<vector<float>> new_number = number;
	int index = 0;;
	for (int i = 0; i < new_number.size(); ++i) {
		if ((*(points.begin() + i))[2] > 0)
			continue;
		if (index == n)
			break;
		(*(new_number.begin() + i))[0] = (*(temp1.begin() + index))[0];
		(*(new_number.begin() + i))[1] = (*(temp1.begin() + index))[1];
		index += 1;
	}
	for (int i = 0; i < new_number.size(); ++i) {
		if ((*(points.begin() + i))[2] < 0)
			continue;
		if (i < new_number.size() - 1)
			(*(new_number.begin() + i))[0] = (*(new_number.begin() + i + 1))[0] + (float)0.5;
		else
			(*(new_number.begin() + i))[0] = (*(new_number.begin() + i - 1))[0] - (float)0.5;
	}
	return new_number;
}

vector<string> getFiles(string folder, string firstname, string lastname)
{
	vector<string> files;
	//文件句柄  
	long long hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;   //大家可以去查看一下_finddata结构组成                            
								   //以及_findfirst和_findnext的用法，了解后妈妈就再也不用担心我以后不会编了  
	string p(folder);
	p.append("\\");
	p.append(firstname);
	p.append("*");
	p.append(lastname);
	if ((hFile = _findfirst(p.c_str(), &fileinfo)) != -1) {
		do {
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib & _A_SUBDIR)) {
				continue;
				/*if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(folder).append("\\").append(fileinfo.name), files);*/
			}
			else {
				string temp = p.assign(folder).append("\\").append(fileinfo.name);
				files.push_back(fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return files;
}
bool get_label_mask(Mat mask, int y, int & label, Mat &label_mask)
{
	Mat data = mask.clone();
	if (y < 0 || y>mask.rows - 1)
		y = 0;
	int n = 0;
	int *mask_ptr = mask.ptr<int>(0);
	for (int i = 0; i < mask.total(); ++i) {
		if (mask_ptr[i] > n)
			n = mask_ptr[i];
	}
	vector<vector<int>> temp_label(n+1);
	for (int i = y; i < mask.rows; ++i) {
		int j = 0;
		for (j = 0; j < mask.cols; ++j) {
			if (mask.at<int>(i, j) != mask.at<int>(i, 0))
				break;
		}
		if (j == mask.cols)
			temp_label[mask.at<int>(i, 0)].push_back(i);
	}
	for (int i = 0; i <= y; ++i) {
		temp_label[mask.at<int>(i, 0)].clear();
	}
	label = -1;
	for (int i = 0; i < n; ++i) {
		if (temp_label[i].size() > 10) {
			label = i;
			break;
		}
	}
	Mat temp_mask = Mat::zeros(mask.size(), CV_8UC1);
	label_mask = Mat::zeros(mask.size(), CV_8UC1);
	for (int i = 0; i < mask.total(); ++i) {
		*(label_mask.ptr<uchar>(0) + i) = *(mask.ptr<int>(0) + i) == label ? 1 : 0;
		*(temp_mask.ptr<uchar>(0) + i) = *(mask.ptr<int>(0) + i) != label ? -1 : 0;
	}
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	Mat temp = Mat::ones(Size(label_mask.cols + 2, label_mask.rows), CV_8UC1);
	label_mask.copyTo(temp.colRange(Range(1, temp.cols - 1)));
	//进行膨胀操作  
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	dilate(temp, temp, element);
	findContours(temp,contours,hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE,Point());
	if (label ==-1||contours.size()>3)
		return false;
	int y1_min= label_mask.rows, y1_max=0, y2_min= label_mask.rows, y2_max = 0;
	for (int i = 0; i < label_mask.cols; ++i) {
		int y1 = -1; int y2=-1;
		for (int j = 0; j < label_mask.rows; ++j) {
			if (label_mask.at<uchar>(j, i) == 1) {
				y1 = j;
				break;
			}
		}
		for (int j = label_mask.rows-1; j >=0; --j) {
			if (label_mask.at<uchar>(j, i) == 1) {
				y2 = j;
				break;
			}
		}
		if (y1 < 0 || y2 < 0)
			continue;
		y1_min = y1_min < y1 ? y1_min : y1;
		y2_min = y2_min < y2 ? y2_min : y2;
		y1_max = y1_max > y1 ? y1_max : y1;
		y2_max = y2_max > y2 ? y2_max : y2;
	}
	float per_t = (float)(y2_min - y1_max) / (float)(y2_max - y1_min);
	if (per_t < 0.6)
		return false;
	return true;
}
float  get_water_line(Mat data, vector<vector<float>> points)
{
	Mat test_im;
	//
	int index = data.rows - 1;
	if (points.size() < 3)
		index = (int)(*(points.end() - 1))[1];
	else
		index = (int)(*(points.end() - 2))[1];
	//
	Mat im = data.clone();
	float scale = (float)100.0 / (float)data.cols;
	resize(im, im, Size(100, data.rows));
	vector<Matx<float, 6, 1>> lines,lines1, lines2;
	get_line(im, 5,45,lines1, lines2);
	// 过滤一些直线
	for (int i = 0; i < lines2.size(); ++i) {
		if (lines2[i].val[5] < 15) {
			lines2.erase(lines2.begin() + i, lines2.end());
			break;
		}
	}
	lines = lines2;
	lines1.clear(); lines2.clear();
	vector<Point> temp_points;
	for (auto &i : lines) {
		if (i.val[0] + i.val[2] < im.cols) {
			lines1.push_back(i);
			temp_points.push_back(Point((int)i.val[2], (int)i.val[3]));
		}
		else {
			lines2.push_back(i);
			temp_points.push_back(Point((int)i.val[0], (int)i.val[1]));
		}
	}
	Vec4f line_para;
	fitLine(temp_points, line_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	vector<vector<float>> e_points = select_e_area_by_line(im, lines1, lines2, (float)data.cols);
	float y1 = 0, y2 = 0;
	if (e_points.size() > 0) {
		vector<float> temp = e_points[0];
		if (temp.size() > 0) {
			y1 = temp[temp.size() - 1];
			y2 = 1.7*(temp[temp.size() - 1] - temp[temp.size() - 2]) + temp[temp.size() - 1];
			y1 = y1 < data.rows - 1 ? y1 : data.rows - 1;
			y2 = y2 < data.rows - 1 ? y2 : data.rows - 1;
		}
	}
	else{

	}
	im = data.clone();
	im(Range(floor(y1), ceil(y2)), Range(0, data.cols)).copyTo(im);

	float water_line = 0;
	
	//
	return water_line;
}
vector<vector<float>> select_e_area_by_line(Mat im, vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2, float distance)
{
	Mat test_im;

	//
	Mat xy_filter, im_gray, score_image;
	vector<Point3f> points1, points2;
	cvtColor(im, im_gray, CV_BGR2GRAY);
	im_gray.convertTo(im_gray, CV_32F);
	float d_t = (float)(distance/10.0);
	xy_filter = Mat::zeros(Size(7, 2), CV_32F);
	xy_filter.colRange(0, 4).setTo(-1.0 / 8.0);xy_filter.colRange(4, 7).setTo(1.0 / 6.0);
	filter2D(im_gray, score_image, -1, xy_filter, Point(-1, -1), 0.0, BORDER_CONSTANT);
	points1 = localmax_point_score(score_image, d_t, (float)0);// 非极大值抑制
	flip(xy_filter, xy_filter, 1);
	filter2D(im_gray, score_image, -1, xy_filter, Point(-1, -1), 0.0, BORDER_CONSTANT);
	points2 = localmax_point_score(score_image, d_t, (float)0);// 非极大值抑制



	vector<Matx<float, 6, 1>> lines1_1, lines1_2, lines2_1, lines2_2;
	vector<vector<float>> e_points,e_points1,e_points2;
	vector<Point3f> temp_points;
	vector<Mat> data1, data2,score1,score2,score;
	test_im = im.clone();
	test_im = draw_line(test_im, lines1, Scalar(255, 0, 0));
	test_im = draw_line(test_im, lines2, Scalar(0, 0, 255));
	e_points1 = claaify_h_lines(im_gray,lines1,points1,lines1_1, lines1_2,data1, score1,true);
	e_points2 = claaify_h_lines(im_gray,lines2,points2,lines2_1, lines2_2,data2, score2,false);
	vector<float> temp,y,index;
	if (e_points1.size()>0)
		temp.insert(temp.end(), e_points1[0].begin(), e_points1[0].end());
	if (e_points2.size()>0)
		temp.insert(temp.end(), e_points2[0].begin(), e_points2[0].end());
	y = cluster_value(temp, index);
	temp.clear();
	if (y.size() < 2)
		return vector<vector<float>>{y};
	for (int i = 0; i < y.size() - 1; ++i)
		temp.push_back(y[i + 1] - y[i]);
	temp = cluster_value(temp, index);
	for (int i = 0; i < temp.size(); ++i) {
		if (index[i] > index[0])
			temp[0] = temp[i];
	}
	if (abs(y[y.size() - 1] - y[y.size() - 2]) > temp[0]&&y.size()>2)
		y.erase(y.end() - 1);
	return vector<vector<float>>{y};
}
vector<vector<float>> cluster_v_line(vector<Matx<float, 6, 1>>& lines, vector<vector<Matx<float, 6, 1>>>& result, float distance)
{
	// 计算关键角点


	float distance_t = 0;
	distance_t = distance / 3;
	Matx<float, 6, 1> line;
	vector<vector<float>> result_points;
	while (lines.size()>0) {
		line = *lines.begin();//从最长开始
		lines.erase(lines.begin());
		vector<Matx<float, 6, 1>> temp_lines, temp;// 与它差不多的直线
		temp_lines.push_back(line);
		for (auto &i : lines) {
			float angle = (line.val[4] < 90 ? line.val[4] + 90 : line.val[4] - 90) - (i.val[4] < 90 ? i.val[4] + 90 : i.val[4] - 90);
			if (abs(i.val[1] - (*(temp_lines.end() - 1)).val[1]) < distance_t&&abs(i.val[3] - (*(temp_lines.end() - 1)).val[3]) < distance_t&&abs(angle)<5)
				temp_lines.push_back(i);
			else
				temp.push_back(i);
		}
		if (temp_lines.size() > 7) {
			lines.insert(lines.begin(), line);
			distance_t = distance_t - 2;
		}
		else {
			result.push_back(temp_lines);
			lines = temp;
			distance_t = distance / 3;
		}
	}
	vector<vector<Matx<float, 6, 1>>> temp;
	lines.clear();
	for (auto &i : result) {
		if (i.size() < 3)
			lines.insert(lines.end(), i.begin(), i.end());
		else
			temp.push_back(i);
	}
	result = temp;
	return vector<vector<float>>();
}
vector<vector<float>> claaify_h_lines(Mat im, vector<Matx<float, 6, 1>> lines, vector<Point3f> points, vector<Matx<float, 6, 1>>& lines1, vector<Matx<float, 6, 1>>& lines2, vector<Mat> &data, vector<Mat> &scores, bool left_or_right)
{
	int x1=0, x2=0;
	lines1.clear();
	lines2.clear();
	Mat im_gray;
	if (im.dims == 1)
		im_gray = im.clone();
	else
		im.convertTo(im_gray, CV_32F);
	for (auto &i : lines) {
		auto temp = get_line_point(Point2f(i.val[0], i.val[1]), Point2f(i.val[2], i.val[3]));
		int n1 = 0,n2=0;
		for (auto &j : temp) {
			if (j.y < 2) {
				n1 += 1;
				continue;
			}
			if (j.y>im.rows-3) {
				n2 += 1;
				continue;
			}
			if (j.x < 0)
				j.x = 0;
			if (j.x >im.cols-1)
				j.x = im.cols - 1;
			if ((im_gray.at<float>(j.y - 2, j.x) + im_gray.at<float>(j.y - 1, j.x) - im_gray.at<float>(j.y +1, j.x) - im_gray.at<float>(j.y + 2, j.x)) > 0)
				n1 += 1;
			else
				n2 += 1;
		}
		if ((float)n1 / float(temp.size()) > 0.5)
			lines1.push_back(i);
		if (((float)n2 / float(temp.size()) > 0.5))
			lines2.push_back(i);
	}
	//
	vector<vector<float>> result;
	for (auto i : points) {
		float score1=999, score2 = 999;
		int index_1=-1, index_2=-1;
		for (int j = 0; j < lines1.size(); ++j) {
			if (abs(2 * i.y - lines1[j].val[1] - lines1[j].val[3]) < score1) {
				score1 = abs(2 * i.y - lines1[j].val[1] - lines1[j].val[3]);
				index_1 = j;
			}
		}
		for (int j = 0; j < lines2.size(); ++j) {
			if (abs(2 * i.y - lines2[j].val[1] - lines2[j].val[3]) < score2) {
				score2 = abs(2 * i.y - lines2[j].val[1] - lines2[j].val[3]);
				index_2 = j;
			}
		}
		if (index_1 <0 || index_2 < 0)
			continue;
		if (lines1[index_1].val[1] > lines2[index_2].val[1] || lines1[index_1].val[3] > lines2[index_2].val[3])
			continue;
		float angle_t = (lines1[index_1].val[4] < 90 ? lines1[index_1].val[4] + 90 : lines1[index_1].val[4] - 90)
						- (lines2[index_2].val[4] < 90 ? lines2[index_2].val[4] + 90 : lines2[index_2].val[4] - 90);
		bool flag = (lines1[index_1].val[1] + lines1[index_1].val[3] > im.cols) == (lines2[index_2].val[1] + lines2[index_2].val[3] > im.cols);
		if (abs(angle_t)<10&&flag) {
			vector<float> temp(15, -1);
			temp[0] = i.x; temp[1] = i.y; temp[2] = i.z;
			temp[3] = lines1[index_1].val[0] < lines2[index_2].val[0]? lines1[index_1].val[0]: lines2[index_2].val[0];
			temp[4] = lines1[index_1].val[1] < lines1[index_1].val[3]? lines1[index_1].val[1]: lines1[index_1].val[3];
			temp[5] = lines1[index_1].val[2] > lines2[index_2].val[2] ? lines1[index_1].val[2] : lines2[index_2].val[2];
			temp[6] = lines2[index_2].val[1] > lines2[index_2].val[3] ? lines2[index_2].val[1] : lines2[index_2].val[3];
			if (temp[4] > temp[6])
				continue;
			for (int j = 0; j < 4; ++j) {
				temp[7+j] = lines1[index_1].val[j];
				temp[11 +j] = lines2[index_2].val[j];
			}
			lines1.erase(lines1.begin() + index_1);
			lines2.erase(lines2.begin() + index_2);
			result.push_back(temp);
		}
	}
	stable_sort(result.begin(),result.end(),
				[](const vector<float>&a, const vector<float>&b) {return a[1] < b[1]; });
	
	Mat test_im = im.clone();
	for (auto &i : result)
		circle(test_im, Point((int)i[0], (int)i[1]), 1,Scalar(255));
	// 找到E区域
	vector<vector<float>> result_e;
	for (int i = 1; i < result.size()-1; ++i) {
		if (abs((result[i - 1])[1] + (result[i + 1])[1] - 2 * (result[i])[1]) > 5)
			continue;
		vector<float> temp1((result[i-1]).begin() + 7, (result[i-1]).begin() + 11);
		vector<float> temp2((result[i]).begin() + 11, (result[i]).end());
		vector<float> temp3((result[i]).begin() + 7, (result[i]).begin() + 11);
		vector<float> temp4((result[i+1]).begin() + 11, (result[i+1]).end());
		Mat temp_im1 = get_area_by_lines(im, temp1, temp2);
		Mat temp_im2 = get_area_by_lines(im, temp3, temp4);
		if (temp_im1.total() == 0|| temp_im2.total() == 0)
			continue;
		temp_im1.convertTo(temp_im1, CV_32F);
		temp_im2.convertTo(temp_im2, CV_32F);
		resize(temp_im1, temp_im1, temp_im2.size(), cv::INTER_LINEAR);
		Mat scores;
		matchTemplate(temp_im1, temp_im2, scores, CV_TM_CCOEFF_NORMED);
		if (scores.at<float>(0, 0) < 0.7)
			continue;
		vector<float> temp5((result[i - 1]).begin() + 7, (result[i - 1]).begin() + 11);
		vector<float> temp6((result[i+1]).begin() + 11, (result[i+1]).end());
		Mat temp_im = get_area_by_lines(im, temp5, temp6);
		data.push_back(temp_im.clone());

		vector<float> temp;
		float x = 0;
		for (int j = i - 1; j < i + 2; ++j) {
			x+=(result[j])[0];
			temp.push_back((result[j])[1]);
		}
		x = x / (float)3;
		temp.insert(temp.begin(),x);
		temp.insert(temp.end(), temp5.begin(), temp5.end());
		temp.insert(temp.end(), temp6.begin(), temp6.end());
		result_e.push_back(temp);
	}
	// 抑制不对劲的E区域
	// 计算互相的相互关系
	Mat corr_matrix = Mat::zeros((int)data.size(), (int)data.size(), CV_32F);
	for (int i = 0; i < data.size(); ++i)
		for (int j = 0; j < data.size(); ++j) {
			if (i == j) {
				continue;
			}
			Mat temp1, temp2;
			Mat result;
			data[i].copyTo(temp1);
			data[j].copyTo(temp2);
			resize(temp2, temp2, temp1.size(), cv::INTER_LINEAR);
			matchTemplate(temp1, temp2, result, CV_TM_CCOEFF_NORMED);
			corr_matrix.at<float>(i, j) = result.at<float>(0, 0);
		}
	vector<int> index = class_score(corr_matrix.clone(), 2, (float)(0.7));
	for (int i = 0; i < index.size(); ++i) {
		data.push_back(data[index[i]]);
		result_e.push_back(result_e[index[i]]);
	}
	data.erase(data.begin(), data.begin() + corr_matrix.rows);
	result_e.erase(result_e.begin(), result_e.begin() + corr_matrix.rows);
	// 排除异常的data
	Point2f row_t(0,0), col_t(0, 0);
	vector<float> temp1,temp2,temp3;
	if (data.size() == 0)
		return vector<vector<float>>();
	//row
	for (int i = 0; i < data.size(); ++i) {
		temp1.push_back((float)data[i].rows);
		temp2.push_back((float)data[i].cols);
	}
	temp1 = cluster_value(temp1, temp3);
	if (temp3.size() == 1) {
		row_t.x = temp1[0];
		row_t.y = temp3[0];
	}
	for (int i = 0; i < temp3.size(); ++i) {
		if (row_t.x < temp1[i] && temp3[i] >= row_t.y) {
			row_t.x = temp1[i];
			row_t.y = temp3[i];
		}
	}
	temp2 = cluster_value(temp2, temp3);
	if (temp3.size() == 1) {
		col_t.x = temp2[0];
		col_t.y = temp3[0];
	}
	for (int i = 0; i < temp3.size(); ++i) {
		if (col_t.x < temp2[i] && temp3[i] >= col_t.y) {
			col_t.x = temp2[i];
			col_t.y = temp3[i];
		}
	}
	int num = (int)data.size();
	for (int i = 0; i < num; ++i) {
		if (abs(data[i].rows - row_t.x) < 5 && abs(data[i].cols - col_t.x) < 5) {
			data.insert(data.end(), data[i]);
			result_e.insert(result_e.end(), result_e[i]);
		}
	}
	data.erase(data.begin(), data.begin() + num);
	result_e.erase(result_e.begin(), result_e.begin() + num);
	//
	vector<vector<Point3f>> result_points;
	for (int i = 0; i < data.size(); ++i) {
		Mat score;
		matchTemplate(im, data[i], score, CV_TM_CCOEFF_NORMED);
		scores.push_back(score);
	}

	vector<float> y;
	for (int i = 0; i < scores.size(); ++i) {
		//x1 = left_or_right ? 0 : 50 - data[i].cols / 2;
		//x2 = left_or_right ? 50 - data[i].cols / 2 : scores[i].cols - 1;
		x1 = 0;
		x2 = scores[i].cols - 1;
		vector<Point3f> temp_points = localmax_point_score(scores[i], x1, x2, data[i].rows/2, 0);
		for (int j = 0; j < temp_points.size(); ++j) {
			if (temp_points[j].z < 0.7) {
				temp_points.erase(temp_points.begin() + j, temp_points.end());
				break;
			}
		}
		vector<float> temp;
		for (int j = 0; j < temp_points.size(); ++j) {
			temp.push_back(temp_points[j].y+(float)data[i].rows/(float)2.0);
		}
		y.insert(y.end(), temp.begin(), temp.end());
	}
	stable_sort(y.begin(), y.end());
	return vector<vector<float>>{y};
}
vector<float> cluster_value(vector<float> data, vector<float>& index)
{
	stable_sort(data.begin(), data.end());
	vector<float> result;
	if (data.size() <= 0)
		return vector<float>();
	index.clear();
	while (1) {
		float sum_y = data[0];
		int i = 0;
		for (; i < data.size(); ++i) {
			if (abs(sum_y - data[i]) > 5) {
				result.push_back(sum_y);
				index.push_back((float)(i));
				break;
			}
			else {
				sum_y = (sum_y*i + data[i]) / (float)(i + 1);
			}
		}
		data.erase(data.begin(), data.begin() + i);
		if (data.size() == 0) {
			result.push_back(sum_y);
			index.push_back((float)(i));
			break;
		}
	}
	return result;
}
Mat get_area_by_lines(Mat im, vector<float> lines1, vector<float> lines2)
{
	float angle, angle1, angle2;
	angle1 = float(atan(-(lines1[3] - lines1[1]) / (lines1[2] - lines1[0])) * 180 / atan(1) / 4);
	angle2 = float(atan(-(lines2[3] - lines2[1]) / (lines2[2] - lines2[0])) * 180 / atan(1) / 4);
	angle = (angle1 + angle2) / (float)2.0;
	// 旋转原始影像
	Point2d center(im.cols / 2.0, im.rows / 2.0);
	Mat r = getRotationMatrix2D(center, -angle, 1.0);
	Rect bbox = RotatedRect(center, im.size(),-angle).boundingRect();
	r.at<double>(0, 2) += bbox.width / 2.0 - center.x;
	r.at<double>(1, 2) += bbox.height / 2.0 - center.y;
	Mat image_rotate;
	cv::warpAffine(im, image_rotate, r, bbox.size());
	r.convertTo(r, CV_32F);
	// 截取大区域
	Mat line1 = Mat::zeros(Size(4,2),CV_32F), line2 = Mat::zeros(Size(4, 2), CV_32F);
	line1.at<float>(0, 0) = lines1[0];	line1.at<float>(1, 0) = lines1[1];	line1.at<float>(0, 1) = lines1[2];	line1.at<float>(1, 1) = lines1[3];
	line2.at<float>(0, 0) = lines2[0];	line2.at<float>(1, 0) = lines2[1];	line2.at<float>(0, 1) = lines2[2];	line2.at<float>(1, 1) = lines2[3];
	for (int j = 0; j < 2; ++j) {
		Mat temp;
		line1.col(j).copyTo(temp);
		temp = r.colRange(0, 2) * temp + r.colRange(2, 3);
		temp.copyTo(line1.col(j + 2));
		line2.col(j).copyTo(temp);
		temp = r.colRange(0, 2) * temp + r.colRange(2, 3);
		temp.copyTo(line2.col(j + 2));
	}
	int r1, r2, c1, c2;
	double temp1, temp2;
	Mat temp(2, 4, CV_32F);
	line1.colRange(2, 4).copyTo(temp.colRange(0, 2));
	line2.colRange(2, 4).copyTo(temp.colRange(2, 4));
	cv::minMaxIdx(temp.row(0), &temp1, &temp2);
	c1 = (int)round(temp1) >= 0 ? (int)round(temp1) : 0;
	c2 = (int)round(temp2) <= image_rotate.cols ? (int)round(temp2) : image_rotate.cols;
	temp1 = (double)temp.at<float>(1, 0);
	temp2 = (double)temp.at<float>(1, 1);
	r1 = temp1 >= temp2 ? (int)ceil(temp1) : (int)ceil(temp2);
	temp1 = (double)temp.at<float>(1, 2);
	temp2 = (double)temp.at<float>(1, 3);
	r2 = temp1 <= temp2 ? (int)floor(temp1) : (int)floor(temp2);
	r2 = r2 <= image_rotate.rows ? r2 : image_rotate.rows;
	if (r1 > r2)
		return Mat();
	Mat data = image_rotate(Range(r1, r2), Range(c1, c2));
	return data;
}
vector<int> sub2ind(Mat m, vector<Point2i> point)
{
	int temp;
	vector<int> result;
	for (auto &i : point) {
		temp = i.x + i.y*m.cols;
		result.push_back(temp);
	}
	return result;
}

vector<Point2i> ind2sub(Mat m, vector<int> ind)
{
	int row, col;
	vector<Point2i> result;
	for (auto &i : ind) {
		row = i / m.cols;
		col = i % m.cols;
		result.push_back(Point2i(col, row));
	}
	return result;
}

void svaefile(Mat im, string image_name, vector<water_result> water)
{
	string temp(image_name), name;
	for (int i = 0; i < temp.size() - 4; ++i) {
		name += temp[i];
	}
	string result_image = "result_" + name + ".jpg", result_txt="result_" + name + ".txt";
	svaefile(im, image_name, water, result_image, result_txt);
}

void svaefile(Mat im, string image_name, vector<water_result> water, string result_image, string result_txt)
{
	//结果保存
	Mat result = im.clone();
	vector<vector<float>> temp_water_number;
	for (int i = 0; i < water.size(); ++i) {

		water_result temp = *(water.begin() + i);
		// 画出点
		result = draw_point(result, temp.points);
		// 画出水尺区域线
		line(result, Point2f(temp.parrallel_lines[0], temp.parrallel_lines[1]), Point2f(temp.parrallel_lines[2], temp.parrallel_lines[3]), Scalar(255, 0, 0), 2);
		line(result, Point2f(temp.parrallel_lines[4], temp.parrallel_lines[5]), Point2f(temp.parrallel_lines[6], temp.parrallel_lines[7]), Scalar(255, 0, 0), 2);
		// 画出水线
		line(result, Point2f(temp.water_lines[0], temp.water_lines[1]), Point2f(temp.water_lines[2], temp.water_lines[3]), Scalar(0, 255, 0), 2);
		// 画出数值
		ostringstream ss;
		ss << round(temp.water_number * 100) / 10;
		string text = ss.str() + "cm";
		int font_face = cv::FONT_HERSHEY_COMPLEX;
		double font_scale = 1;
		int thickness = 2;
		int baseline;
		//获取文本框的长宽  
		Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
		Point2f temp_point((temp.points[temp.points.size() - 1])[0], (temp.points[temp.points.size() - 1])[1]);
		if (temp_point.x < result.cols / 2)
			temp_point.x = temp_point.x + 30;
		else
			temp_point.x = temp_point.x - 30 - text_size.width;
		putText(result, text, temp_point, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);
		vector<float> temp1;
		temp1.push_back(temp_point.x);
		temp1.push_back(round(temp.water_number * 100) / 10);
		temp_water_number.push_back(temp1);
	}
	// 读写图像
	imwrite(result_image, result);
	// 读写文件
	stable_sort(temp_water_number.begin(), temp_water_number.end(),
		[](vector<float>a, vector<float>b) {return a[0] < b[0]; });
	ofstream file(result_txt);
	//file.width(16);
	//file.setf(ios::left);
	for (int i = 0; i < water.size(); ++i) {
		water_result temp = *(water.begin() + i);
		file << "No=";
		file << (i + 1) <<";"<< endl;
		file << "WaterLevel=";
		file << fixed << setprecision(1)<<round(temp.water_number * 100) / 10 << ";" << endl;
		file << "GuageArea=";
		for (int j = 0; j < temp.parrallel_lines.size(); ++j) {
			file << fixed << setprecision(2) << temp.parrallel_lines[j];
			if ((j % 4) == 3)
				file << ";";
			else
				file << ",";
		}
		file << endl;
		file << "WaterLine=";
		for (int j = 0; j < temp.water_lines.size(); ++j) {
			file << fixed << setprecision(2) << temp.water_lines[j] ;
			if ((j % 4) == 3)
				file << ";";
			else
				file << ",";
		}
		file << endl;
	}
	file.close();
}
