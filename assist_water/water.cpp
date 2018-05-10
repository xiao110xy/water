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
		if (temp.size() == 0|| assist_files.size() != temp[0])
			break;
		temp_assist_file.base_image = template_image[temp[1] - 1];
		temp_assist_file.length = temp[2];
		// roi
		vector<double> roi;
		getline(assist_file_name, temp_name);
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
			roi.push_back((double)temp_value);
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
				temp.push_back((double)temp_value);
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

void compute_water_area(Mat im, vector<assist_information> &assist_files)
{
	for (auto &assist_file : assist_files) {
		// 延伸
		assist_file.add_row = 25;
		Mat base_image = assist_file.base_image;
		Mat add_image = Mat::zeros(Size(base_image.cols, assist_file.add_row), CV_64F);
		vconcat(base_image, add_image, assist_file.wrap_image);
		Mat image_rotate = correct_image(im, assist_file);
		get_water_line(assist_file);
	}
}

Mat correct_image(Mat im, assist_information &assist_file)
{
	Mat result,map_x,map_y;
	map_coord(assist_file, map_x, map_y);
	remap(im, result, map_x, map_y,CV_INTER_CUBIC);
	assist_file.wrap_image = result;
	// 将roi 即水尺区域的平行线保存
	vector<double> parrallel_lines;
	parrallel_lines.push_back((double)map_x.at<float>(0, 0));
	parrallel_lines.push_back((double)map_y.at<float>(0, 0));
	parrallel_lines.push_back((double)map_x.at<float>(map_x.rows - 1, 0));
	parrallel_lines.push_back((double)map_y.at<float>(map_y.rows - 1, 0));
	parrallel_lines.push_back((double)map_x.at<float>(0, map_x.cols - 1));
	parrallel_lines.push_back((double)map_y.at<float>(0, map_y.cols - 1));
	parrallel_lines.push_back((double)map_x.at<float>(map_x.rows - 1, map_x.cols - 1));
	parrallel_lines.push_back((double)map_y.at<float>(map_x.rows - 1, map_y.cols - 1));
	assist_file.parrallel_lines = parrallel_lines;
	return result;
}

void map_coord(assist_information & assist_file, Mat &map_x, Mat &map_y)
{
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
	float water_line ;
	double sigmarS = 5, sigmarR = 5;
	int label;
	int y = 0;

	Mat mask, label_mask;
	while (1) {
		// 处理分割
		MeanShiftSegmentor s;
		s.m_SigmaS = sigmarS;
		s.m_SigmaR = sigmarR;
		s.SetImage(im);
		s.Run();
		s.ShowResult();
		assist_file.segment_result = s.showImg.clone();
		mask = s.m_Result.clone();
		// 超出部分，插值为0
		// 将超出部分的label设置为-1;
		for (int i = 0; i < mask.rows; ++i)
			for (int j = 0; j < mask.cols; ++j) {
				Vec3b  temp = im.at<Vec3b>(i, j);
				if (temp[0] == 0 && temp[1] == 0 && temp[2] == 0) {
					mask.at<int>(i, j) = -1;
				}
			}
		bool flag = get_label_mask(mask,label, label_mask, assist_file);
		if (flag) {
			vector<float> y_num;
			for (int i = 0; i < mask.cols; ++i) {
				for (int j = 0; j < mask.rows; ++j)
					if (mask.at<int>(j, i) == label) {
						y_num.push_back((float)j);
						break;
					}
			}
			//// 线中间部分最小值
			//int index1 = (int)y_num.size() / 3, index2 = 2 * (int)y_num.size() / 3;
			//for (int i = index1; i < index2; ++i)
			//	if (water_line < y_num[i])
			//		water_line = y_num[i];
			// 平均值
			float y1_1 = 9999,y2_1 = 9999,y3_1=9999;
			float y1_2 = -9999,y2_2=- 9999, y3_2 = -9999;
			int index1 = (int)y_num.size() / 4, index2 = 3 * (int)y_num.size() / 4;
			for (int i = 0; i < index1; ++i) {
				y1_1 = y1_1 < y_num[i] ? y1_1 : y_num[i];
				y1_2 = y1_2 > y_num[i] ? y1_2 : y_num[i];
			}
			for (int i = index1; i < index2; ++i) {
				y2_1 = y2_1 < y_num[i] ? y2_1 : y_num[i];
				y2_2 = y2_2 > y_num[i] ? y2_2 : y_num[i];
			}
			for (int i = index2; i < y_num.size(); ++i) {
				y3_1 = y3_1 < y_num[i] ? y3_1 : y_num[i];
				y3_2 = y3_2 > y_num[i] ? y3_2 : y_num[i];
			}
			water_line = round((y1_1 + y1_2 + y2_1 + y2_2 + y3_1 + y3_2) / 6);
			water_line = round(water_line) < assist_file.base_image.rows - 1 ? round(water_line) : assist_file.base_image.rows - 1;
			break;
		}
		else {
			sigmarS = sigmarS + 1;
			sigmarR = sigmarR + 2;
		}
		if (sigmarR > 30) {
			water_line = assist_file.base_image.rows - 1;
			break;
		}
	}
	// water_line
	Mat water_line_point = Mat::zeros(Size(2, 2), CV_64F);
	water_line_point.at<double>(0, 0) = 0;
	water_line_point.at<double>(0, 1) = water_line;
	water_line_point.at<double>(1, 0) = assist_file.base_image.cols-1;
	water_line_point.at<double>(1, 1) = water_line;
	water_line_point = compute_point(water_line_point, assist_file.r);
	assist_file.water_lines.clear();
	assist_file.water_lines.push_back(water_line_point.at<double>(0, 0));
	assist_file.water_lines.push_back(water_line_point.at<double>(0, 1));
	assist_file.water_lines.push_back(water_line_point.at<double>(1, 0));
	assist_file.water_lines.push_back(water_line_point.at<double>(1, 1));
	// water number
	assist_file.water_number = (1-(water_line + 1) / (double)assist_file.base_image.rows)*assist_file.length;
}
bool get_label_mask(Mat mask,int & label, Mat &label_mask, assist_information assist_file)
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
	// 寻找到一个合适的类别
	float label_t = (2.5 / assist_file.length)*mask.rows;
	label = -1;
	for (int i = 0; i < n; ++i) {
		if (temp_label[i].size() > label_t) {
			label = i;
			break;
		}
	}
	if (label<0)
		return false;
	 //如果上方锯齿，则需要进行膨胀腐蚀之类的操作
	 //如果内部孔洞多 则需要否决
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
	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point());
	if (contours.size()>7)
		return false;
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

void save_file(Mat im, vector<assist_information> assist_files, string image_result, string para_result)
{
	//结果保存
	Mat result = im.clone();
	vector<vector<float>> temp_water_number;
	for (int i = 0; i < assist_files.size(); ++i) {

		assist_information temp = assist_files[i];
		// 画出水尺区域线
		line(result, Point2d(temp.parrallel_lines[0], temp.parrallel_lines[1]), Point2d(temp.parrallel_lines[2], temp.parrallel_lines[3]), Scalar(255, 0, 0), 2);
		line(result, Point2d(temp.parrallel_lines[4], temp.parrallel_lines[5]), Point2d(temp.parrallel_lines[6], temp.parrallel_lines[7]), Scalar(255, 0, 0), 2);
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
			file << fixed << setprecision(2) << temp.parrallel_lines[j];
			if ((j % 4) == 3)
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

