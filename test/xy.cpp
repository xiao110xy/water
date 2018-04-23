#include "stdafx.h"
#include "xy.h"

Mat Edge_Detect(Mat im, int aperture_size)
{
	Mat test_im;
	//
	Mat data, result;
	// 灰度图像
	if (im.channels() > 1)
		cvtColor(im, data, CV_BGR2GRAY);
	else
		data = im.clone();
	// BORDER_REPLICATE 表示当卷积点在图像的边界时，原始图像边缘的像素会被复制，并用复制的像素扩展原始图的尺寸  
	// 计算x方向的sobel方向导数，计算结果存在dx中  
	// 计算y方向的sobel方向导数，计算结果存在dy中  
	Mat dx, dy;
	Sobel(data, dx, CV_32F, 1, 0, aperture_size, 1, 0, cv::BORDER_REPLICATE);
	Sobel(data, dy, CV_32F, 0, 1, aperture_size, 1, 0, cv::BORDER_REPLICATE);
	// dy方向局部极大极小值 找出极小值，并进行处理
	vector<Point3f> points1 = localmax_point_score(dx, dx.cols / 3, dx.cols / 3 * 2,3, 0);
	vector<Point3f> points2 = localmax_point_score(-dx, dx.cols / 3, dx.cols / 3 * 2,3, 0);
	Mat temp_dx = dx.clone();
	temp_dx.colRange(0, dx.cols / 3).setTo(0);
	temp_dx.colRange(dx.cols / 3*2, dx.cols).setTo(0);
	
	//test_im = im.clone();
	//test_im = draw_point(test_im,points1, Scalar(255, 0, 0));
	//test_im = draw_point(test_im,points2, Scalar(0, 0,255));

	//vector<vector<Point3f>> temp_vvp3f;
	//test_im = im.clone();
	//float scale = 0.99;
	//temp_vvp3f = cluster_point(temp_dx, 50, 4);
	//for (auto i:temp_vvp3f)
	//	test_im = draw_point(test_im, i, Scalar(0, 255, 0));
	//temp_vvp3f = cluster_point(-temp_dx, 50, 4);
	//for (auto i : temp_vvp3f)
	//	test_im = draw_point(test_im, i, Scalar(0, 0, 255));
	//
	// 筛选points
	Mat flag_image = Mat::zeros(data.size(), CV_8UC1);
	vector<vector<vector<Point3f>>> E_area1 = select_pointline(dx, dy, points1, true,flag_image);
	vector<vector<vector<Point3f>>> E_area2 = select_pointline(dx, dy, points2, false,flag_image);
	
	Mat temp[3];
	temp[0] = data.clone(); temp[1] = data.clone(); temp[2] = data.clone();
	merge(temp, 3, test_im);
	Scalar rgb[3];
	rgb[0] = Scalar(0, 255, 0);
	rgb[1] = Scalar(0, 0, 255);
	rgb[2] = Scalar(255, 0, 0);
	for (auto i : E_area1) {
		for (int j = 0; j<3; ++j)
			for (auto k : i[j]) {
				test_im.at<Vec3b>(k.y, k.x).val[0] = rgb[j].val[0];
				test_im.at<Vec3b>(k.y, k.x).val[1] = rgb[j].val[1];
				test_im.at<Vec3b>(k.y, k.x).val[2] = rgb[j].val[2];
			}
	}
	rgb[1] = Scalar(0, 255, 0);
	rgb[2] = Scalar(0, 0, 255);
	rgb[0] = Scalar(255, 0, 0);
	for (auto i : E_area2) {
		for (int j = 0; j<3; ++j)
			for (auto k : i[j]) {
				test_im.at<Vec3b>(k.y, k.x).val[0] = rgb[j].val[0];
				test_im.at<Vec3b>(k.y, k.x).val[1] = rgb[j].val[1];
				test_im.at<Vec3b>(k.y, k.x).val[2] = rgb[j].val[2];
			}
	}

	// 去除非中心区域
	get_E_area(im, E_area1, E_area2, dx, dy);
	return Mat();
}

Mat draw_part_E(Mat im, vector<vector<vector<Point3f>>> data)
{
	Mat show_im;
	if (im.channels() == 1) {
		Mat temp[3];
		temp[0] = im.clone(); temp[1] = im.clone(); temp[2] = im.clone();
		merge(temp, 3, show_im);
	}
	else {
		show_im = im.clone();
	}
	Scalar rgb[3];
	rgb[0] = Scalar(0, 255, 0);
	rgb[1] = Scalar(0, 0, 255);
	rgb[2] = Scalar(255, 0, 0);
	for (auto i : data) {
		for (int j = 0; j<3; ++j)
			for (auto k : i[j]) {
				show_im.at<Vec3b>(k.y, k.x).val[0] = rgb[j].val[0];
				show_im.at<Vec3b>(k.y, k.x).val[1] = rgb[j].val[1];
				show_im.at<Vec3b>(k.y, k.x).val[2] = rgb[j].val[2];
			}
	}
	return show_im;
}

vector<vector<vector<Point3f>>> select_pointline(Mat dx, Mat dy, vector<Point3f> points, bool flag,Mat flag_image)
{
	Mat test_im;

	float point_score_t = 50;
	Mat point_scoreimage = flag ? dx.clone() : -dx.clone();
	Mat line_scoreimage = dy.clone();
	vector<vector<vector<Point3f>>> result;
	int dety = line_scoreimage.cols / 6;
	dety = dety < 3 ? 3 : dety;
	int detx = line_scoreimage.cols / 4;
	for (auto &i : points) {
		if (i.y<3 || i.y>point_scoreimage.rows - 4|| point_scoreimage.at<float>(i.y,i.x)<point_score_t)
			continue;
		vector<vector<Point3f>> temp_vvp3f;
		vector<Point3f> temp_point, temp_line1, temp_line2;
		float score_t = i.z*0.5;
		Mat temp_flag_image = flag_image.clone();
		temp_point.push_back(i);
		temp_flag_image.at<uchar>(i.y, i.x) = 255;
		Mat temp_data;
		Point index; double maxval;
		int x1, x2, y1, y2;
		x1 = flag ? i.x - detx : i.x + 1;
		x2 = flag ? i.x : i.x + 1 + detx;
		if (x1<0 || x2>line_scoreimage.cols)
			continue;
		y1 = i.y - dety >= 0 ? i.y - dety : 0;
		y2 = i.y + dety <= line_scoreimage.rows ? i.y + dety : line_scoreimage.rows;
		temp_data = -line_scoreimage(Range(y1, i.y+3), Range(x1, x2)).clone();
		minMaxLoc(temp_data, NULL, NULL, NULL, &index);
		float score_t1 = temp_data.at<float>(index.y, index.x)*0.4;
		temp_vvp3f = cluster_point(temp_data, score_t1, 8, index);
		if (temp_vvp3f.size() < 1)
			continue;
		temp_line1 = cluster_point(-line_scoreimage.clone(), score_t1, Point(index.x + x1, index.y + y1), temp_flag_image.clone(), 8);
		if (temp_line1.size() < 1)
			continue;
		temp_line1 = cluster_point(temp_line1, false, temp_flag_image);
		temp_data = line_scoreimage(Range(i.y -2, y2), Range(x1, x2)).clone();
		minMaxLoc(temp_data, NULL, NULL, NULL, &index);
		float score_t2 = temp_data.at<float>(index.y, index.x)*0.4;
		temp_vvp3f  =cluster_point(temp_data, score_t2, 8, index);
		if (temp_vvp3f.size() < 1)
			continue;
		temp_line2 = cluster_point(line_scoreimage.clone(), score_t2, Point(index.x + x1, index.y + i.y -2), temp_flag_image.clone(), 8);
		if (temp_line2.size() < 1)
			continue;
		temp_line2 = cluster_point(temp_line2, false, temp_flag_image);

		vector<vector<Point3f>> temp_result = new_point_line1_line2(dx, dy, temp_point, temp_line1, temp_line2, flag);
		if (temp_result.size() != 3)
			continue;
		if (temp_result[0].size()>temp_result[1].size() || temp_result[0].size()>temp_result[2].size())
			continue;
		point_scoreimage.rowRange((temp_result[0])[0].y, (temp_result[0])[temp_result[0].size() - 1].y+1).setTo(0);
		line_scoreimage.rowRange((temp_result[0])[0].y, (temp_result[0])[temp_result[0].size() - 1].y+1).setTo(0);
		
		vector<Point> draw_temp;
		draw_temp.push_back(Point((temp_result[0])[0].x, (temp_result[0])[0].y));
		draw_temp.push_back(Point((temp_result[0])[temp_result[0].size()-1].x, (temp_result[0])[temp_result[0].size() - 1].y));
		if (flag) {
			draw_temp.push_back(Point((temp_result[2])[0].x, (temp_result[2])[0].y));
			draw_temp.push_back(Point((temp_result[1])[0].x, (temp_result[1])[0].y));
		}
		else {
			draw_temp.push_back(Point((temp_result[2])[temp_result[2].size() - 1].x, (temp_result[2])[temp_result[2].size() - 1].y));
			draw_temp.push_back(Point((temp_result[1])[temp_result[1].size() - 1].x, (temp_result[1])[temp_result[1].size() - 1].y));
		}
		Mat mask= Mat::zeros(flag_image.size(), CV_8UC1);
		fillPoly(mask, vector<vector<Point>>{draw_temp}, Scalar(255));
		int num1 = 0, num2 = 0;
		for (int i = 0; i < mask.total(); ++i) {
			if (*(mask.ptr(0) + i) != 0) {
				if (*(flag_image.ptr(0) + i) == 0)
					++num1;
				else
					++num2;
			}
		}
		if (num1 < num2)
			continue;
		fillPoly(flag_image, vector<vector<Point>>{draw_temp}, Scalar(result.size() + 1));
		result.push_back(temp_result);
	}
	// 去除掉一些非横线
	stable_sort(result.begin(), result.end(),
		[](vector<vector<Point3f>> a, vector<vector<Point3f>>b) {return (a[0])[0].y < (b[0])[0].y; });
	if (result.size() < 3)
		return result;
	vector<vector<vector<Point3f>>> temp_result;
	vector<Point2f> x_points;
	for (int i = 1; i < result.size()-1; ++i) {
		vector<vector<Point3f>> temp = result[i];
		vector<vector<Point3f>> temp1 = result[i - 1];
		vector<vector<Point3f>> temp2 = result[i + 1];
		// x相差不大
		if (abs((temp[0])[0].x - (temp1[0])[0].x) > (flag_image.cols / 12 < 3 ? flag_image.cols / 12 : 3))
			continue;
		if (abs((temp[0])[0].x - (temp2[0])[0].x) > (flag_image.cols / 12 < 3 ? flag_image.cols / 12 : 3))
			continue;
		// y 差值不大
		if ((temp[0])[0].y - (temp1[0])[temp1[0].size()-1].y > 1.5*temp[0].size())
			continue;
		if ((temp2[0])[0].y - (temp[0])[temp[0].size() - 1].y > 1.5*temp[0].size())
			continue;
		temp_result.push_back(temp);
		x_points.push_back(Point2f((temp[0])[0].x, (temp[0])[0].y));
		x_points.push_back(Point2f((temp[0])[temp[0].size() - 1].x, (temp[0])[temp[0].size() - 1].y));
	}
	test_im = draw_part_E(flag_image, temp_result);
	Vec4f line_para;
	fitLine(x_points, line_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	temp_result.clear();
	for (auto i : result) {
		vector<Point3f> temp = i[0];
		float x1,x2,d;
		d = (temp[0].x - line_para[2])*line_para[0] + (temp[0].y - line_para[3])*line_para[1];
		x1 = d * line_para[0] + line_para[2];
		d = (temp[temp.size()-1].x - line_para[2])*line_para[0] + (temp[temp.size() - 1].y - line_para[3])*line_para[1];
		x2 = d * line_para[0] + line_para[2];
		float x_t = (flag_image.cols / 12) > 3 ? flag_image.cols / 12 : 3;
		if (abs(x1 - temp[0].x) > x_t || abs(x2 - temp[temp.size() - 1].x) > x_t)
			continue;
		temp_result.push_back(i);
	}
	return temp_result;
}

vector<Point3f> cluster_point(Mat score_image, float score_t, Point point, Mat flag_image, int number)
{
	if (score_t<50)
		return vector<Point3f>();
	vector<Point3f> result;
	int x = point.x;
	int y = point.y;
	if (x<0 || y<0 || x>score_image.cols - 1 || y>score_image.rows - 1 || flag_image.at<uchar>(y, x) != 0)
		return vector<Point3f>();
	flag_image.at<uchar>(y, x) = 255;
	Point3f temp_point;
	temp_point.x = (float)point.x;
	temp_point.y = (float)point.y;
	temp_point.z = score_image.at<float>(y, x);
	if (temp_point.z < score_t)
		return vector<Point3f>();
	// 四邻域
	result.push_back(temp_point);
	vector<Point3f> temp;
	temp = cluster_point(score_image, score_t, Point(point.x, point.y - 1), flag_image, number);
	result.insert(result.end(), temp.begin(), temp.end());
	temp = cluster_point(score_image, score_t, Point(point.x, point.y + 1), flag_image, number);
	result.insert(result.end(), temp.begin(), temp.end());
	temp = cluster_point(score_image, score_t, Point(point.x + 1, point.y), flag_image, number);
	result.insert(result.end(), temp.begin(), temp.end());
	temp = cluster_point(score_image, score_t, Point(point.x - 1, point.y), flag_image, number);
	result.insert(result.end(), temp.begin(), temp.end());
	if (number > 4) {
		temp = cluster_point(score_image, score_t, Point(point.x - 1, point.y - 1), flag_image, number);
		result.insert(result.end(), temp.begin(), temp.end());
		temp = cluster_point(score_image, score_t, Point(point.x + 1, point.y - 1), flag_image, number);
		result.insert(result.end(), temp.begin(), temp.end());
		temp = cluster_point(score_image, score_t, Point(point.x + 1, point.y + 1), flag_image, number);
		result.insert(result.end(), temp.begin(), temp.end());
		temp = cluster_point(score_image, score_t, Point(point.x - 1, point.y + 1), flag_image, number);
		result.insert(result.end(), temp.begin(), temp.end());
	}
	return result;
}
vector<vector<Point3f>> cluster_point(Mat score_image, float score_t, int number, Point &index)
{
	vector<Point> points;
	for (int i = 0; i < score_image.total(); ++i) {
		if (*(score_image.ptr<float>(0)+i) > score_t) {
			points.push_back(Point(i % score_image.cols, i/score_image.cols));
		}
	}
	Mat flag = Mat::zeros(score_image.size(),CV_8U);
	vector<vector<Point3f>> result;
	for (auto i : points) {
		vector<Point3f> temp=cluster_point(score_image, score_t, i, flag, number);
		if (temp.size()>0)
			result.push_back(temp);
	}
	stable_sort(result.begin(), result.end(),
		[](vector<Point3f> a, vector<Point3f> b) {return a.size() > b.size(); });
	if (result.size()<1)
		return result;
	vector<Point3f> temp_result(result[0]);
	stable_sort(temp_result.begin(), temp_result.end(),
		[](Point3f a, Point3f b) {return a.z > b.z; });
	index.x = temp_result[0].x;
	index.y = temp_result[0].y;
	return result;
}
vector<vector<Point3f>> cluster_point(Mat score_image, float score_t, int number)
{
	vector<vector<Point3f>> result;
	Point index;
	result = cluster_point(score_image, score_t, number, index);
	return result;
}
vector<Point3f> cluster_point(vector<Point3f> data, bool flag, Mat &flag_image)
{
	Mat test_im = flag_image.clone();
	for (auto i:data)
		test_im.at<uchar>(i.y, i.x) = flag ? 255 : 125;
	if (flag) {
		for (auto &i : data)
			std::swap(i.x, i.y);
	}
	stable_sort(data.begin(), data.end(),
		[](const Point3f&a, const Point3f&b) {return a.x < b.x; });
	for (int i = 0; i < data.size(); ++i) {
		if (data[i].x >= 1) {
			data.erase(data.begin(), data.begin() + i);
			break;
		}
	}
	Point3f maxz_point;
	for (auto i : data) {
		if (i.z > maxz_point.z)
			maxz_point = i;
	}
	vector<Point3f> result{ maxz_point };
	int x, y;
	x = maxz_point.x;
	y = maxz_point.y;
	while (x > data[0].x) {
		int index = -1;
		float score = 0;
		for (int i = 0; i < data.size(); ++i) {
			if (data[i].x == x - 1 && abs(data[i].y - y) < 2 && data[i].z > score) {
				score = data[i].z;
				index = i;
			}
		}
		if (index == -1)
			break;
		x = data[index].x;
		y = data[index].y;
		result.push_back(data[index]);
	}
	stable_sort(result.begin(), result.end(),
		[](Point3f a, Point3f b) {return a.x < b.x; });
	x = maxz_point.x;
	y = maxz_point.y;
	while (x < data[data.size()-1].x) {
		int index = -1;
		float score = 0;
		for (int i = 0; i < data.size(); ++i) {
			if (data[i].x == x + 1 && abs(data[i].y - y) < 2 && data[i].z > score) {
				score = data[i].z;
				index = i;
			}
		}
		if (index == -1)
			break;
		x = data[index].x;
		y = data[index].y;
		result.push_back(data[index]);
	}
	if (flag) {
		for (auto &i : result)
			std::swap(i.x, i.y);
	}
	for (auto i : result)
		flag_image.at<uchar>(i.y, i.x) = flag ? 255 : 125;
	return result;
}



vector<vector<Point3f>> new_point_line1_line2(Mat dx, Mat dy, vector<Point3f> point, vector<Point3f> line1_point, vector<Point3f> line2_point, bool flag)
{
	// 竖直直线的x坐标
	int x = point[0].x, y = point[0].y;
	//int x=0,y=0;
	//map<int,int> temp;
	//int max = 0;
	//for (auto i : point) {
	//	++temp[(int)i.x];
	//}
	//for (auto i : temp) {
	//	if (i.second > max) {
	//		x = i.first;
	//		max = i.second;
	//	}
	//	if (i.second == max && flag&&i.first < x)
	//		x = i.first;
	//	if (i.second == max && !flag&&i.first > x)
	//		x = i.first;
	//}
	//float score_z=-1;
	//for (auto i : point)
	//	if (i.x == x && score_z < i.z)
	//		y = i.y;
	// 直线拟合
	vector<Point2f> temp_f_point;
	Point2f point_first(9999, 0), point_end(-9999, 0);
	vector<Point> temp_point, temp_line1_point, temp_line2_point;
	Vec4f point_para, line1_para, line2_para;
	float mindx1 = 999, mindy1 = 999;
	for (auto i : line1_point) {
		if (i.x <= x && flag) {
			temp_line1_point.push_back(Point((int)i.x, (int)i.y));
			mindx1 = mindx1 < abs(i.x - x) ? mindx1 : abs(i.x - x);
			//mindy1 = mindy1 <y - i.y? mindy1 : y - i.y;
		}
		if (i.x >= x && !flag) {
			temp_line1_point.push_back(Point((int)i.x, (int)i.y));
			mindx1 = mindx1 <abs(i.x - x) ? mindx1 : abs(i.x - x);
			//mindy1 = mindy1 < y - i.y ? mindy1 : y - i.y;
		}
	}
	if (temp_line1_point.size()<2 || mindx1>3)
		return vector<vector<Point3f>>();
	//
	float mindx2 = 999, mindy2 = 999;
	for (auto i : line2_point) {
		if (i.x <= x && flag) {
			temp_line2_point.push_back(Point((int)i.x, (int)i.y));
			mindx2 = mindx2 < abs(i.x - x) ? mindx2 : abs(i.x - x);
			//mindy2 = mindy2 < i.y - y ? mindy2 : i.y - y;

		}

		if (i.x >= x && !flag) {
			temp_line2_point.push_back(Point((int)i.x, (int)i.y));
			mindx2 = mindx2 < abs(i.x - x) ? mindx2 : abs(i.x - x);
			//mindy2 = mindy2 < i.y - y ? mindy2 : i.y - y;
		}
	}
	if (temp_line2_point.size()<2 || mindx2>3)
		return vector<vector<Point3f>>();
	//
	fitLine(temp_line1_point, line1_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	temp_f_point.push_back(Point2f((float)x,
		(x - line1_para[2]) * line1_para[1] / line1_para[0] + line1_para[3]));
	for (auto i : temp_line1_point) {
		float d = (i.x - line1_para[2])*line1_para[0] + (i.y - line1_para[3])*line1_para[1];
		float x = d * line1_para[0] + line1_para[2];
		float y = d * line1_para[1] + line1_para[3];
		temp_f_point.push_back(Point2f(x, y));
	}
	for (auto i : temp_f_point) {
		if (i.x < point_first.x)
			point_first = i;
		if (i.x > point_end.x)
			point_end = i;
	}
	temp_line1_point = get_line_point(point_first, point_end);
	fitLine(temp_line2_point, line2_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	temp_f_point.clear();
	temp_f_point.push_back(Point2f((float)x,
		(x - line2_para[2]) * line2_para[1] / line2_para[0] + line2_para[3]));
	for (auto i : temp_line2_point) {
		float d = (i.x - line2_para[2])*line2_para[0] + (i.y - line2_para[3])*line2_para[1];
		float x = d * line2_para[0] + line2_para[2];
		float y = d * line2_para[1] + line2_para[3];
		temp_f_point.push_back(Point2f(x, y));
	}
	point_first.x = 9999;
	point_end.x = -9999;
	for (auto i : temp_f_point) {
		if (i.x < point_first.x)
			point_first = i;
		if (i.x > point_end.x)
			point_end = i;
	}
	temp_line2_point = get_line_point(point_first, point_end);
	point_first.y = 9999;
	point_end.y = -9999;
	if (flag) {
		point_first = *(temp_line1_point.end() - 1);
		point_end = *(temp_line2_point.end() - 1);
	}
	else {
		point_first = temp_line1_point[0];
		point_end = temp_line2_point[0];
	}
	if (y - point_first.y < 0 || y - point_end.y>0|| point_end.y- point_first.y<3)
		return vector<vector<Point3f>>();
	temp_point = get_line_point(point_first, point_end);
	vector<vector<Point3f>> temp_result;
	point.clear();
	line1_point.clear();
	line2_point.clear();
	for (auto i : temp_point)
		point.push_back(Point3f((float)i.x, (float)i.y, dx.at<float>(i.y, i.x)));
	for (auto i : temp_line1_point)
		line1_point.push_back(Point3f((float)i.x, (float)i.y, dy.at<float>(i.y, i.x)));
	for (auto i : temp_line2_point)
		line2_point.push_back(Point3f((float)i.x, (float)i.y, dy.at<float>(i.y, i.x)));
	temp_result.push_back(point);
	temp_result.push_back(line1_point);
	temp_result.push_back(line2_point);
	return temp_result;
}

void get_E_area(Mat im, vector<vector<vector<Point3f>>> left_e, vector<vector<vector<Point3f>>> right_e, Mat dx, Mat dy)
{
	vector<Point2f> x_points;
	for (auto i : left_e) {
		for (auto j : i[0]) {
			x_points.push_back(Point2f(j.x, j.y));
		}
	}
	for (auto i : right_e) {
		for (auto j : i[0]) {
			x_points.push_back(Point2f(j.x, j.y));
		}
	}
	Vec4f line_para;
	fitLine(x_points, line_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	// 寻找确定的E区域
	// 根据确定的E区域进行推断
	// 
	// 寻找左右E靠近相交部分
	vector<vector<vector<Point3f>>> temp_result, result;
	vector<vector<float>> flag_v;
	for (auto i:left_e)
		for (auto j : right_e) {
			vector<Point3f> y1 =i[0], y2=j[0];
			Point3f y1_1= y1[0], y1_2 = *(y1.end()-1), y2_1= y2[0], y2_2= *(y2.end() - 1);
			bool flag1 = abs(y1_2.y - y2_1.y) > ((y1.size() > y2.size() ? y1.size() : y2.size())*0.5);
			bool flag2 = abs(y1_1.x - y2_1.x) > ((im.cols / 12)>3?(im.cols/12):3);
			if (flag1||flag2)
				continue;
			vector<float> temp(2);
			temp[0] = y1_2.y; temp[1] = y2_1.y;
			flag_v.push_back(temp);
		}


}
