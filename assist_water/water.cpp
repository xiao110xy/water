#include "stdafx.h"
#include "water.h"

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
				files.push_back(p.assign(folder).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return files;
}

bool input_template(string file_name, vector<Mat> &template_image)
{
	FileStorage fs(file_name, FileStorage::READ);
	fs["model"] >> template_image;
	if (template_image.size() == 0)
	{
		return false;
	}
	for (auto &i : template_image) {
		if (i.dims != 1) {
			i.convertTo(i, CV_BGR2GRAY);
		}
	}
	return true;
}

bool input_assist(string file_name, vector<assist_information> & assist_files, vector<Mat> template_image)
{
	fstream assist_file_name(file_name);
	string temp_name;
	while (!assist_file_name.eof())
	{
		vector<int> temp;
		assist_information temp_assist_file;
		getline(assist_file_name, temp_name);
		temp.clear();
		for (int i = 0; i < temp_name.size(); ++i) {
			int temp_value = 0;
			int j = 0;
			for (j = i; j < temp_name.size(); ++j) {
				if (temp_name[j] >= 48 && temp_name[j] <= 57) {
					temp_value = temp_value * 10 + temp_name[j] - 48;
				}
				else {
					break;
				}
			}
			i = j;
			temp.push_back(temp_value);
		}
		if (temp.size() == 0|| assist_files.size() != temp[0]-1)
			break;
		temp_assist_file.base_image = template_image[temp[1] - 1];
		temp_assist_file.length = temp[2];
		// roi
		vector<double> roi;
		getline(assist_file_name, temp_name);
		for (int i = 0; i < temp_name.size(); ++i) {
			if (temp_name[i] >= 48 && temp_name[i] <= 57) {
				double temp_value = 0;
				int j = i;
				int n = -1;
				for (; j < temp_name.size(); ++j) {
					if (temp_name[j]==44||temp_name[j] == 59) {
						temp_value = n == -1 ? temp_value : temp_value / pow(10, j - n - 1);
						roi.push_back(temp_value);
						break;
					}
					if (temp_name[j] >=48 && temp_name[j] <= 57) {
						temp_value = temp_value * 10 + temp_name[j] - 48;
					}
					if (temp_name[j] == 46)
						n = j;
				}
				i = j;
			}
		}
		if (roi.size() != 4)
			break;
		temp_assist_file.roi = roi;
		// point
		vector<vector<double>> temp_point;
		for (int n = 0; n < temp[3]; ++n) {
			getline(assist_file_name, temp_name);
			vector<double> temp;
			for (int i = 0; i < temp_name.size(); ++i) {
				if (temp_name[i] >= 48 && temp_name[i] <= 57) {
					double temp_value = 0;
					int j = i;
					int n = -1;
					for (; j < temp_name.size(); ++j) {
						if (temp_name[j] == 44 || temp_name[j] == 59) {
							temp_value = n == -1 ? temp_value : temp_value / pow(10, j - n - 1);
							temp.push_back(temp_value);
							break;
						}
						if (temp_name[j] >= 48 && temp_name[j] <= 57) {
							temp_value = temp_value * 10 + temp_name[j] - 48;
						}
						if (temp_name[j] == 46)
							n = j;
					}
					i = j;
				}
			}
			temp_point.push_back(temp);
		}
		temp_assist_file.point = temp_point;
		assist_files.push_back(temp_assist_file);
	}
	if (assist_files.size() == 0)
		return false;
	else
		return true;
}

void compute_water_area(Mat im, vector<assist_information> &assist_files, string base_name)
{
	int n = 0;
	for (auto &assist_file:assist_files) {
		// 延伸一部分水尺区域
		assist_file.add_row = 25;
		Mat base_image = assist_file.base_image;
		Mat add_image = Mat::zeros(Size(base_image.cols, assist_file.add_row), CV_64F);
		vconcat(base_image, add_image, assist_file.wrap_image);
		// 旋转校正
		Mat image_rotate = correct_image(im, assist_file);
		assist_file.base_image = image_rotate.rowRange(0, base_image.rows).clone();
		// 获得水位线,两种方式选择
		string ref_image_name = "ref_" + base_name +"_"+to_string(n+1) +".bmp";
		fstream _file;
		_file.open(ref_image_name, ios::in);
		if (!_file) {
			get_water_line(assist_file);// 不需要历史数据参考
		}
		else {
			Mat ref_image = imread(ref_image_name);// 历史参考数据
			get_water_line(assist_file, ref_image);
		}
		_file.close();
		++n;
	}
}

Mat correct_image(Mat im, assist_information &assist_file)
{
	Mat result,map_x,map_y;
	map_coord(assist_file, map_x, map_y);// map_x,map_y float型数据
	remap(im, result, map_x, map_y,CV_INTER_CUBIC);
	assist_file.wrap_image = result;
	// 将roi 即水尺区域的平行线保存
	// 存在一些问题 需要使用拟合的方式进行一定的处理
	Vec4f line_para;
	vector<Point2d> points;
	Point2d point1, point2;
	// 左侧线段
	Mat test_im = Mat::zeros(Size(2000, 2000), CV_8UC3);
	for (int i = 0; i < assist_file.wrap_image.rows; ++i) {
		points.push_back(Point2d(map_x.at<float>(i, 0), map_y.at<float>(i, 0)));
		circle(test_im, points[i], 1, Scalar(255, 0, 0));
		assist_file.parrallel_left.push_back(points[i]);
	}
	fitLine(points, line_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	point1.y = 9999; point2.y = -9999;
	for (auto i : points) {
		float d = (i.x - line_para[2])*line_para[0] + (i.y - line_para[3])*line_para[1];
		float x = d * line_para[0] + line_para[2];
		float y = d * line_para[1] + line_para[3];
		if (point1.y > y) {
			point1.x = x;
			point1.y = y;
		}
		if (point2.y < y) {
			point2.x = x;
			point2.y = y;
		}
	}
	line(test_im, point1, point2, Scalar(0, 255, 0));
	assist_file.parrallel_lines.push_back(point1);
	assist_file.parrallel_lines.push_back(point2);
	// 右侧线段
	points.clear();
	for (int i = 0; i < assist_file.wrap_image.rows; ++i) {
		points.push_back(Point2d(map_x.at<float>(i, assist_file.wrap_image.cols-1), map_y.at<float>(i, assist_file.wrap_image.cols - 1)));
		circle(test_im, points[i], 1, Scalar(255, 0, 0));
		assist_file.parrallel_right.push_back(points[i]);
	}
	fitLine(points, line_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	point1.y = 9999; point2.y = -9999;
	for (auto i : points) {
		float d = (i.x - line_para[2])*line_para[0] + (i.y - line_para[3])*line_para[1];
		float x = d * line_para[0] + line_para[2];
		float y = d * line_para[1] + line_para[3];
		if (point1.y > y) {
			point1.x = x;
			point1.y = y;
		}
		if (point2.y < y) {
			point2.x = x;
			point2.y = y;
		}
	}
	line(test_im, point1, point2, Scalar(0, 255, 0));
	assist_file.parrallel_lines.push_back(point1);
	assist_file.parrallel_lines.push_back(point2);
	return result;
}

void map_coord(assist_information & assist_file, Mat &map_x, Mat &map_y)
{
	// 获取变换的矩阵
	Mat r = GeoCorrect2Poly(assist_file, true);
	Mat r_inv = GeoCorrect2Poly(assist_file, false);
	Mat point = vector2Mat(assist_file.point);
	double rms_1 = compute_rms(point.colRange(0, 2), point.colRange(2, 4), r);
	double rms_2 = compute_rms(point.colRange(2, 4), point.colRange(0, 2), r_inv);
	assist_file.r = r;
	assist_file.r_inv = r_inv;
	assist_file.rms_1 = rms_1;
	assist_file.rms_2 = rms_2;
	//
	Mat temp1 = Mat::zeros(assist_file.wrap_image.size(), CV_64F);
	for (int i = 0; i < temp1.cols; ++i) {
		temp1.col(i).setTo(i);
	}
	temp1 = temp1.reshape(0,(int)temp1.total());
	Mat temp2 = Mat::zeros(assist_file.wrap_image.size(), CV_64F);
	for (int i = 0; i < temp2.rows; ++i) {
		temp2.row(i).setTo(i);
	}
	temp2 = temp2.reshape(0, (int)temp2.total());
	Mat temp;
	hconcat(temp1, temp2, temp);
	temp = compute_point(temp, r);
	map_x = temp.col(0).clone();
	map_x = map_x.reshape(0, assist_file.wrap_image.rows);
	map_x.convertTo(map_x, CV_32F);
	map_y = temp.col(1).clone();
	map_y = map_y.reshape(0, assist_file.wrap_image.rows);
	map_y.convertTo(map_y, CV_32F);
}

Mat GeoCorrect2Poly(assist_information assist_file, bool flag)
{
	int n = (int)assist_file.point.size();
	Mat point;
	if (flag) 
		point = vector2Mat(assist_file.point);
	else{
		vector<vector<double>> temp= assist_file.point;
		swap_point(temp);
		point = vector2Mat(temp);
	}
	Mat X = point.col(0).clone();
	Mat Y = point.col(1).clone();
	Mat XY = X.mul(Y), XX = X.mul(X), YY = Y.mul(Y);
	Mat M = Mat::zeros(Size(6, n), CV_64F);
	M.col(0).setTo(1);
	X.copyTo(M.col(1));
	Y.copyTo(M.col(2));
	XY.copyTo(M.col(3));
	XX.copyTo(M.col(4));
	YY.copyTo(M.col(5));
	Mat A, B;
	A = (M.t()*M).inv()*M.t()*point.col(2);
	B = (M.t()*M).inv()*M.t()*point.col(3);
	Mat result = Mat::zeros(Size(2, 6), CV_64F);
	A.copyTo(result.col(0));
	B.copyTo(result.col(1));
	return result;
}

Mat compute_point(Mat point, Mat r)
{
	Mat M = Mat::ones(Size(1, point.rows), CV_64F);
	Mat X = point.col(0).clone() , Y = point.col(1).clone();
	hconcat(M, X, M);
	hconcat(M,Y, M);
	hconcat(M, X.mul(Y), M);
	hconcat(M, X.mul(X), M);
	hconcat(M, Y.mul(Y), M);
	return M * r;
}

double compute_rms(Mat base_point, Mat wrap_point, Mat r)
{
	double rms = 0;
	Mat temp_point = compute_point(base_point, r) - wrap_point;
	pow(temp_point , 2, temp_point);
	for (int i = 0; i < temp_point.total(); ++i)
		rms += *(temp_point.ptr<double>(0) + i);
	rms = sqrt(rms / temp_point.rows);
	return rms;
}

void get_water_line(assist_information & assist_file)
{
	Mat im = assist_file.wrap_image.clone();
	float water_line = 9999;
	int sigmarS = 30, sigmarR = 40;
	int label;
	int y1 = (int)floor(Edge_Detect(assist_file.wrap_image,3))+5;
	Mat mask, label_mask;
	while (1) {
		// 处理分割
		MeanShiftSegmentor s;
		s.m_SigmaS = sigmarS;
		s.m_SigmaR = sigmarR;
		if (sigmarR <20) {
			water_line = assist_file.base_image.rows - 1;
			break;
		}
		s.SetImage(im);
		s.Run();
		s.ShowResult();
		assist_file.segment_result = s.showImg.clone();
		mask = s.m_Result.clone();
		// 超出部分，插值为0
		// 将超出部分的label设置为-1;
		for (int i = 0; i < mask.rows; ++i) {
			int n = 0;
			for (int j = 0; j < mask.cols; ++j) {
				Vec3b  temp = im.at<Vec3b>(i, j);
				if (temp[0] == 0 && temp[1] == 0 && temp[2] == 0) {
					mask.at<int>(i, j) = -1;
				}
			}
		}
		bool flag = get_label_mask(mask,label, label_mask, assist_file,y1);
		if (flag) {
			Mat element1 = getStructuringElement(MORPH_RECT, Size(3, 5));
			Mat element2 = getStructuringElement(MORPH_RECT, Size(5, 3));
			Mat temp = mask.clone();
			temp.convertTo(temp, CV_8U);
			erode(temp, temp, element1);
			erode(temp, temp, element2);
			dilate(temp, temp, element2);
			dilate(temp, temp, element1);
			vector<float> y_num;
			for (int i = 0; i < temp.cols; ++i) {
				for (int j = 0; j < temp.rows; ++j)
					if (temp.at<uchar>(j, i) == label) {
						y_num.push_back((float)j);
						break;
					}
			}
			// 线中间部分最小值
			int index1 = (int)y_num.size() / 4, index2 = 3 * (int)y_num.size() / 4;
			for (int i = index1; i <index2; ++i)
				if (water_line > y_num[i])
					water_line = y_num[i];
			//// 平均值
			//float y1_1 = 9999,y2_1 = 9999,y3_1=9999;
			//float y1_2 = -9999,y2_2=- 9999, y3_2 = -9999;
			//int index1 = (int)y_num.size() / 4, index2 = 3 * (int)y_num.size() / 4;
			//for (int i = 0; i < index1; ++i) {
			//	y1_1 = y1_1 < y_num[i] ? y1_1 : y_num[i];
			//	y1_2 = y1_2 > y_num[i] ? y1_2 : y_num[i];
			//}
			//for (int i = index1; i < index2; ++i) {
			//	y2_1 = y2_1 < y_num[i] ? y2_1 : y_num[i];
			//	y2_2 = y2_2 > y_num[i] ? y2_2 : y_num[i];
			//}
			//for (int i = index2; i < y_num.size(); ++i) {
			//	y3_1 = y3_1 < y_num[i] ? y3_1 : y_num[i];
			//	y3_2 = y3_2 > y_num[i] ? y3_2 : y_num[i];
			//}
			//water_line = round((y1_1 + y1_2 + y2_1 + y2_2 + y3_1 + y3_2) / 6);
			water_line = round(water_line) < assist_file.base_image.rows - 1 ? round(water_line) : assist_file.base_image.rows - 1;
			break;
		}
		else {
			sigmarS = sigmarS - 1;
			sigmarR = sigmarR - 2;
		}
	}
	// water_line
	Mat water_line_point = Mat::zeros(Size(2, 2), CV_64F);
	water_line_point.at<double>(0, 0) = 0;
	int y2_1 = water_line;
	for (int i = y2_1;i>=0;--i)
		if (mask.at<int>(i,0) >= 0) {
			y2_1 = i;
			break;
		}
	water_line_point.at<double>(0, 1) = y2_1;
	water_line_point.at<double>(1, 0) = assist_file.base_image.cols-1;
	int y2_2 = water_line;
	for (int i = y2_2; i >= 0; --i)
		if (mask.at<int>(i,assist_file.base_image.cols - 1) >= 0) {
			y2_2 = i;
			break;
		}
	water_line_point.at<double>(1, 1) = y2_2;
	water_line_point = compute_point(water_line_point, assist_file.r);
	assist_file.water_lines.clear();
	assist_file.water_lines.push_back(water_line_point.at<double>(0, 0));
	assist_file.water_lines.push_back(water_line_point.at<double>(0, 1));
	assist_file.water_lines.push_back(water_line_point.at<double>(1, 0));
	assist_file.water_lines.push_back(water_line_point.at<double>(1, 1));
	// water number
	water_line = y2_1 < y2_2 ? y2_1 : y2_2;
	assist_file.water_number = (1-(water_line + 1) / (double)assist_file.base_image.rows)*assist_file.length;
}
bool get_label_mask(Mat mask,int & label, Mat &label_mask, assist_information assist_file,int y_t)
{
	Mat data = mask.clone();
	// 得到共有多少个label，label 为n+1
	int n = 0;
	int *mask_ptr = mask.ptr<int>(0);
	for (int i = 0; i < mask.total(); ++i) {
		if (mask_ptr[i] > n)
			n = mask_ptr[i];
	}
	// 寻找类别中，同行所有列中均存在的类别
	vector<vector<int>> temp_label(n + 1);
	for (int i = 0; i < mask.rows; ++i) {
		int j = 0;
		for (j = 0; j < mask.cols; ++j) {
			if (mask.at<int>(i, j) != mask.at<int>(i, 0)||mask.at<int>(i,0)<0)
				break;
		}
		if (j == mask.cols)
			temp_label[mask.at<int>(i, 0)].push_back(i);
	}
	// 排除
	for (int i = 0; i < y_t; ++i) {
		for (int j = 0; j < mask.cols; ++j) {
			if (mask.at<int>(i, j)>=0)
				temp_label[mask.at<int>(i, j)].clear();
		}
	}
	// 寻找到一个合适的类别
	float label_t = (2.5 / assist_file.length)*mask.rows;
	label = -1;
	for (int i =0; i <=n; ++i) {
		if (temp_label[i].size() < label_t)
			continue;
		if (label < 0)
			label = i;
		else {
			if (temp_label[i].size() > temp_label[label].size()) {
				label = i;
			}
		}
	}
	if (label<0)
		return false;
	// //如果上方锯齿，则需要进行膨胀腐蚀之类的操作
	// //如果内部孔洞多 则需要否决
	//Mat temp_mask = Mat::zeros(mask.size(), CV_8UC1);
	//label_mask = Mat::zeros(mask.size(), CV_8UC1);
	//for (int i = 0; i < mask.total(); ++i) {
	//	*(label_mask.ptr<uchar>(0) + i) = *(mask.ptr<int>(0) + i) == label ? 1 : 0;
	//	*(temp_mask.ptr<uchar>(0) + i) = *(mask.ptr<int>(0) + i) != label ? -1 : 0;
	//}
	//vector<vector<Point>> contours;
	//vector<Vec4i> hierarchy;
	//Mat temp = Mat::ones(Size(label_mask.cols + 2, label_mask.rows), CV_8UC1);
	//label_mask.copyTo(temp.colRange(Range(1, temp.cols - 1)));
	////进行膨胀操作  
	//Mat element = getStructuringElement(MORPH_RECT, Size(3, 3));
	//dilate(temp, temp, element);
	//findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point());
	//if (contours.size()>7)
	//	return false;
	int y1_min = label_mask.rows, y1_max = 0, y2_min = label_mask.rows, y2_max = 0;
	for (int i = 0; i < label_mask.cols; ++i) {
		int y1 = -1; int y2 = -1;
		for (int j = 0; j < label_mask.rows; ++j) {
			if (label_mask.at<uchar>(j, i) == 1) {
				y1 = j;
				break;
			}
		}
		for (int j = label_mask.rows - 1; j >= 0; --j) {
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

void get_water_line(assist_information & assist_file, Mat ref_image)
{
	Mat im = assist_file.base_image.clone();
	int n = im.rows / assist_file.length;
	vector<float> score1,score2,score3;
	float max_score = 0.5;
	int c1 = im.cols / 2,c2 = im.cols;
	// 一个E
	for (int i = 0; i < (int)assist_file.length/5; ++i) {
		int r1 = i * 5*n;
		int r2 = (i + 1)*5*n;
		Mat temp1 = i % 2 ==0 ? im(Range(r1,r2 ), Range(0,c1)) : 
			im(Range(r1, r2), Range(c1,c2));
		Mat temp2 = i % 2 ==0 ? ref_image(Range(r1, r2), Range(0, c1)) :
			ref_image(Range(r1, r2), Range(c1, c2));
		Mat temp;
		matchTemplate(temp1, temp2, temp, CV_TM_CCOEFF_NORMED);
		max_score = max_score > temp.at<float>(0, 0)?max_score:temp.at<float>(0, 0);
		if (temp.at<float>(0, 0) < 0.5*max_score) {
			// E的小部分
			for (int j = 0; j < 5; ++j) {
				Mat temp_temp1 = temp1.rowRange(j*n,(j+1)*n);
				Mat temp_temp2 = temp2.rowRange(j*n, (j + 1)*n);
				Mat temp;
				matchTemplate(temp_temp1, temp_temp2, temp, CV_TM_CCOEFF_NORMED);
				if (temp.at<float>(0, 0) < 0.3*max_score) {
					for (int k = 0; k < n; ++k) {
						Mat temp_temp_temp1 = temp_temp1.row(k);
						Mat temp_temp_temp2 = temp_temp2.row(k);
						Mat temp;
						matchTemplate(temp_temp_temp1, temp_temp_temp2, temp, CV_TM_CCOEFF_NORMED);
						if (temp.at<float>(0, 0) < 0.2*max_score)
							break;
						else
							score3.push_back(temp.at<float>(0, 0));
					}
					break;
				}
				else
					score2.push_back(temp.at<float>(0, 0));
			}
			break;
		}
		else
			score1.push_back(temp.at<float>(0, 0));
	}
	// water number
	assist_file.water_number = assist_file.length-5*score1.size()-score2.size()-score3.size()/10.0;
	//	water_line 
	double water_line = (1-assist_file.water_number/assist_file.length)*assist_file.base_image.rows-1;
	Mat water_line_point = Mat::zeros(Size(2, 2), CV_64F);
	water_line_point.at<double>(0, 0) = 0;
	water_line_point.at<double>(0, 1) = water_line;
	water_line_point.at<double>(1, 0) = c2 - 1;
	water_line_point.at<double>(1, 1) = water_line;
	water_line_point = compute_point(water_line_point, assist_file.r);
	assist_file.water_lines.clear();
	assist_file.water_lines.push_back(water_line_point.at<double>(0, 0));
	assist_file.water_lines.push_back(water_line_point.at<double>(0, 1));
	assist_file.water_lines.push_back(water_line_point.at<double>(1, 0));
	assist_file.water_lines.push_back(water_line_point.at<double>(1, 1));

}

void save_file(Mat im, vector<assist_information> assist_files, string base_name, string image_result, string para_result)
{
	//结果保存
	Mat result = im.clone();
	vector<vector<float>> temp_water_number;
	for (int i = 0; i < assist_files.size(); ++i) {
		assist_information temp = assist_files[i];
		// 子部分
		fstream _file;
		_file.open("sub_"+base_name+"_"+ to_string(i+1)+".bmp", ios::in);
		if (!_file) {
			imwrite("sub_" + base_name + "_" + to_string(i + 1) + ".bmp", temp.base_image);
		}
		_file.close();
		// 画出水尺两侧域
		for (auto i : temp.parrallel_left) {
			circle(result, i, 1, Scalar(0, 0, 255), 1);
		}
		for (auto i : temp.parrallel_right) {
			circle(result, i, 1, Scalar(0, 0, 255), 1);
		}
		// 画出水尺区域线
		line(result, temp.parrallel_lines[0], temp.parrallel_lines[1], Scalar(255, 0, 0), 2);
		line(result, temp.parrallel_lines[2], temp.parrallel_lines[3], Scalar(255, 0, 0), 2);
		// 画出水线
		line(result, Point2d(temp.water_lines[0], temp.water_lines[1]), Point2d(temp.water_lines[2], temp.water_lines[3]), Scalar(0, 255, 0), 2);
		// 画出数值
		ostringstream ss;
		ss << round(temp.water_number * 10) / 10;
		string text = ss.str() + "cm";
		int font_face = cv::FONT_HERSHEY_COMPLEX;
		double font_scale = 1;
		int thickness = 2;
		//获取文本框的长宽 
		int baseline;
		Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);
		Point2f temp_point;
		temp_point.x = (temp.water_lines[0] + temp.water_lines[2]) / 2;
		temp_point.y = (temp.water_lines[1] + temp.water_lines[3]) / 2;
		if (temp_point.x < result.cols / 2)
			temp_point.x = temp_point.x + 30;
		else
			temp_point.x = temp_point.x - 30 - text_size.width;
		putText(result, text, temp_point, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);

	}
	// 读写图像
	imwrite(image_result, result);
	//fstream _file;
	//_file.open(image_result, ios::in);
	//if (!_file) {
	//	imwrite(image_result, result);
	//}
	//_file.close();
	// 读写文件
	stable_sort(assist_files.begin(), assist_files.end(),
		[](assist_information a, assist_information b) {return a.water_lines[0] < b.water_lines[0]; });
	ofstream file(para_result);
	for (int i = 0; i < assist_files.size(); ++i) {
		assist_information temp = assist_files[i];
		file << "No=";
		file << (i + 1) << ";" << endl;
		file << "WaterLevel=";
		file << fixed << setprecision(1) << round(temp.water_number * 10) / 10 << ";" << endl;
		file << "GuageArea=";
		for (int j = 0; j < temp.parrallel_lines.size(); ++j) {
			file << fixed << setprecision(2) << temp.parrallel_lines[j].x;
			file << ",";
			file << fixed << setprecision(2) << temp.parrallel_lines[j].y;			
			if ((j % 2) == 1)
				file << ";";
			else
				file << ",";
		}
		file << endl;
		file << "WaterLine=";
		for (int j = 0; j < temp.water_lines.size(); ++j) {
			file << fixed << setprecision(2) << temp.water_lines[j];
			if ((j % 4) == 3)
				file << ";";
			else
				file << ",";
		}
		file << endl;
	}
	file.close();
}

vector<vector<double>> part_row_point(vector<vector<double>> point, int r1, int r2)
{
	vector<vector<double>> result;
	int m = (int)point.size();
	int n = m == 0 ? 0: (int)point[0].size();
	if (m == 0 || n == 0)
		return result;
	if (r2 < r1)
		swap(r1, r2);
	r1 = r1 < m ? r1 : m;
	r2 = r2 < m ? r2 : m;
	for (int i = r1; i < r2; ++i)
		result.push_back(point[i]);
	return result;
}

vector<vector<double>> part_col_point(vector<vector<double>> point, int c1, int c2)
{
	vector<vector<double>> result;
	int m = (int)point.size();
	int n = m == 0 ? 0 : (int)point[0].size();
	if (m == 0 || n == 0)
		return result;
	if (c2 < c1)
		swap(c1, c2);
	c1 = c1 < n ? c1 : n;
	c2 = c2 < n ? c2 : n;
	for (auto i : point) 
		result.push_back(vector<double>{i.begin()+c1, i.begin() + c2});
	return result;
}

vector<vector<double>> swap_point(vector<vector<double>> &data)
{
	for (auto &i : data) {
		vector<double> temp = i;
		i[0] = temp[2];
		i[1] = temp[3];
		i[2] = temp[0];
		i[3] = temp[1];
	}
	return data;
}

Mat vector2Mat(vector<vector<double>> data)
{
	int m = 0, n = 0;
	m = (int)data.size();
	if (m==0)
		return Mat();
	n = (int)data[0].size();
	if (n == 0)
		return Mat();
	Mat result = Mat::zeros(Size(n, m), CV_64F);
	for (int i =0;i<m;++i)
		for (int j = 0; j < n; ++j) {
			result.at<double>(i, j) = (data[i])[j];
		}
	return result;
}

vector<vector<double>> Mat2vector(Mat data)
{
	int m = data.rows, n = data.cols;
	vector<vector<double>> result(m);
	for (int i = 0; i < m; ++i) {
		vector<double> temp(n);
		for (int j = 0; j < n; ++j) {
			temp[j] = data.at<double>(i, j);
		}
		result[i] = temp;
	}
	return result;
}

double Edge_Detect(Mat im, int aperture_size)
{
	Mat test_im;
	//
	Mat data;
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
	vector<Point3f> points1 = localmax_point_score(dx, dx.cols / 3, dx.cols / 3 * 2, 3, 0);
	vector<Point3f> points2 = localmax_point_score(-dx, dx.cols / 3, dx.cols / 3 * 2, 3, 0);
	Mat temp_dx = dx.clone();
	temp_dx.colRange(0, dx.cols / 3).setTo(0);
	temp_dx.colRange(dx.cols / 3 * 2, dx.cols).setTo(0);
	// 筛选points
	Mat flag_image = Mat::zeros(data.size(), CV_8UC1);
	vector<vector<vector<Point3f>>> E_area1 = get_pointline(dx, dy, points1, true, flag_image);
	vector<vector<vector<Point3f>>> E_area2 = get_pointline(dx, dy, points2, false, flag_image);
	//vector<vector<vector<Point3f>>> rest_point_line = select_point_line(E_area1, E_area2);
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
	//rgb[0] = Scalar(0, 255, 0);
	//rgb[1] = Scalar(0, 255, 255);
	//rgb[2] = Scalar(255, 255, 0);
	//for (auto i : rest_point_line) {
	//	for (int j = 0; j<3; ++j)
	//		for (auto k : i[j]) {
	//			test_im.at<Vec3b>(k.y, k.x).val[0] = rgb[j].val[0];
	//			test_im.at<Vec3b>(k.y, k.x).val[1] = rgb[j].val[1];
	//			test_im.at<Vec3b>(k.y, k.x).val[2] = rgb[j].val[2];
	//		}
	//}
	// 去除非中心区域
	double result = 0.0;
	vector<vector<vector<Point3f>>> E_area;
	E_area.insert(E_area.end(), E_area1.begin(), E_area1.end());
	E_area.insert(E_area.end(), E_area2.begin(), E_area2.end());
	stable_sort(E_area.begin(), E_area.end(),
		[](vector<vector<Point3f>> a, vector<vector<Point3f>> b) {return (a[0])[0].y < (b[0])[0].y; });
	if (E_area.size() ==0)
		return 0;
	else
		return (*((E_area[E_area.size()-1])[0].end() - 1)).y;
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

vector<vector<vector<Point3f>>> get_pointline(Mat dx, Mat dy, vector<Point3f> points, bool flag, Mat flag_image)
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
		if (i.y<3 || i.y>point_scoreimage.rows - 4 || point_scoreimage.at<float>(i.y, i.x)<point_score_t)
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
		temp_data = -line_scoreimage(Range(y1, i.y + 3), Range(x1, x2)).clone();
		minMaxLoc(temp_data, NULL, NULL, NULL, &index);
		float score_t1 = temp_data.at<float>(index.y, index.x)*0.4;
		temp_vvp3f = cluster_point(temp_data, score_t1, 8, index);
		if (temp_vvp3f.size() < 1)
			continue;
		temp_line1 = cluster_point(-line_scoreimage.clone(), score_t1, Point(index.x + x1, index.y + y1), temp_flag_image.clone(), 8);
		if (temp_line1.size() < 1)
			continue;
		temp_line1 = cluster_point(temp_line1, false, temp_flag_image);
		temp_data = line_scoreimage(Range(i.y - 2, y2), Range(x1, x2)).clone();
		minMaxLoc(temp_data, NULL, NULL, NULL, &index);
		float score_t2 = temp_data.at<float>(index.y, index.x)*0.4;
		temp_vvp3f = cluster_point(temp_data, score_t2, 8, index);
		if (temp_vvp3f.size() < 1)
			continue;
		temp_line2 = cluster_point(line_scoreimage.clone(), score_t2, Point(index.x + x1, index.y + i.y - 2), temp_flag_image.clone(), 8);
		if (temp_line2.size() < 1)
			continue;
		temp_line2 = cluster_point(temp_line2, false, temp_flag_image);

		vector<vector<Point3f>> temp_result = new_point_line1_line2(dx, dy, temp_point, temp_line1, temp_line2, flag);
		if (temp_result.size() != 3)
			continue;
		if (temp_result[0].size()>temp_result[1].size() || temp_result[0].size()>temp_result[2].size())
			continue;
		point_scoreimage.rowRange((temp_result[0])[0].y, (temp_result[0])[temp_result[0].size() - 1].y + 1).setTo(0);
		line_scoreimage.rowRange((temp_result[0])[0].y, (temp_result[0])[temp_result[0].size() - 1].y + 1).setTo(0);

		vector<Point> draw_temp;
		draw_temp.push_back(Point((temp_result[0])[0].x, (temp_result[0])[0].y));
		draw_temp.push_back(Point((temp_result[0])[temp_result[0].size() - 1].x, (temp_result[0])[temp_result[0].size() - 1].y));
		if (flag) {
			draw_temp.push_back(Point((temp_result[2])[0].x, (temp_result[2])[0].y));
			draw_temp.push_back(Point((temp_result[1])[0].x, (temp_result[1])[0].y));
		}
		else {
			draw_temp.push_back(Point((temp_result[2])[temp_result[2].size() - 1].x, (temp_result[2])[temp_result[2].size() - 1].y));
			draw_temp.push_back(Point((temp_result[1])[temp_result[1].size() - 1].x, (temp_result[1])[temp_result[1].size() - 1].y));
		}
		Mat mask = Mat::zeros(flag_image.size(), CV_8UC1);
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
		fillPoly(flag_image, vector<vector<Point>>{draw_temp}, Scalar((int)result.size() + 1));
		result.push_back(temp_result);
	}
	// 去除掉一些非横线
	stable_sort(result.begin(), result.end(),
		[](vector<vector<Point3f>> a, vector<vector<Point3f>>b) {return (a[0])[0].y < (b[0])[0].y; });
	if (result.size() < 3)
		return result;
	vector<vector<vector<Point3f>>> temp_result;
	vector<Point2f> x_points;
	for (int i = 1; i < result.size() - 1; ++i) {
		vector<vector<Point3f>> temp = result[i];
		vector<vector<Point3f>> temp1 = result[i - 1];
		vector<vector<Point3f>> temp2 = result[i + 1];
		// x相差不大
		if (abs((temp[0])[0].x - (temp1[0])[0].x) >(flag_image.cols / 12 < 3 ? flag_image.cols / 12 : 3))
			continue;
		if (abs((temp[0])[0].x - (temp2[0])[0].x) > (flag_image.cols / 12 < 3 ? flag_image.cols / 12 : 3))
			continue;
		// y 差值不大
		if ((temp[0])[0].y - (temp1[0])[temp1[0].size() - 1].y > 2.5*temp[0].size())
			continue;
		if ((temp2[0])[0].y - (temp[0])[temp[0].size() - 1].y > 2.5*temp[0].size())
			continue;
		temp_result.push_back(temp);
		x_points.push_back(Point2f((temp[0])[0].x, (temp[0])[0].y));
		x_points.push_back(Point2f((temp[0])[temp[0].size() - 1].x, (temp[0])[temp[0].size() - 1].y));
	}
	test_im = draw_part_E(flag_image, temp_result);
	// 暂时仅需要这个
	return temp_result;
	// 
	if (x_points.size() == 0)
		return result;
	Vec4f line_para;
	fitLine(x_points, line_para, CV_DIST_FAIR, 0, 1e-2, 1e-2);
	temp_result.clear();
	for (auto i : result) {
		vector<Point3f> temp = i[0];
		float x1, x2, d;
		d = (temp[0].x - line_para[2])*line_para[0] + (temp[0].y - line_para[3])*line_para[1];
		x1 = d * line_para[0] + line_para[2];
		d = (temp[temp.size() - 1].x - line_para[2])*line_para[0] + (temp[temp.size() - 1].y - line_para[3])*line_para[1];
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
		if (*(score_image.ptr<float>(0) + i) > score_t) {
			points.push_back(Point(i % score_image.cols, i / score_image.cols));
		}
	}
	Mat flag = Mat::zeros(score_image.size(), CV_8U);
	vector<vector<Point3f>> result;
	for (auto i : points) {
		vector<Point3f> temp = cluster_point(score_image, score_t, i, flag, number);
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
	for (auto i : data)
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
	while (x < data[data.size() - 1].x) {
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
	if (y - point_first.y < 0 || y - point_end.y>0 || point_end.y - point_first.y<3)
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

vector<vector<vector<Point3f>>> select_point_line(vector<vector<vector<Point3f>>> &left_e, vector<vector<vector<Point3f>>> &right_e)
{
	vector<vector<vector<Point3f>>> data, temp_part, rest_point_line;
	data.insert(data.end(), left_e.begin(), left_e.end());
	data.insert(data.end(), right_e.begin(), right_e.end());
	stable_sort(data.begin(), data.end(),
		[](vector<vector<Point3f>> a, vector<vector<Point3f>> b) {return (a[0])[0].y < (b[0])[0].y; });
	temp_part.clear();
	for (auto i : left_e) {
		vector<vector<Point3f>> temp = i;
		float y1, y2;
		y1 = (i[0])[0].y - 4 * i[0].size();
		y2 = (i[0])[i[0].size() - 1].y + 4 * i[0].size();
		vector<vector<vector<Point3f>>> temp_data;
		for (auto j : data) {
			if ((j[0])[0].y > y1 && (j[0])[j[0].size() - 1].y < y2) {
				if (abs((j[0])[0].x - (i[0])[0].x) < 2)
					temp_data.push_back(j);
			}
		}
		if (temp_data.size() < 2) {
			rest_point_line.push_back(i);
			continue;
		}
		temp_part.push_back(i);
	}
	left_e = temp_part;
	temp_part.clear();
	for (auto i : right_e) {
		float y1, y2;
		y1 = (i[0])[0].y - 4 * i[0].size();
		y2 = (i[0])[i[0].size() - 1].y + 4 * i[0].size();
		vector<vector<vector<Point3f>>> temp_data;
		for (auto j : data) {
			if ((j[0])[0].y > y1 && (j[0])[j[0].size() - 1].y < y2) {
				if (abs((j[0])[0].x - (i[0])[0].x) < 2)
					temp_data.push_back(j);
			}
		}
		if (temp_data.size() < 2) {
			rest_point_line.push_back(i);
			continue;
		}
		temp_part.push_back(i);
	}
	right_e = temp_part;
	return rest_point_line;
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
