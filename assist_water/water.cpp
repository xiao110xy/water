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
	if (template_image.size() > 0)
	{
		return true;
	}
	return false;
}

bool input_assist(string file_name, vector<assist_information> & assist_file, vector<Mat> template_image)
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
		if (temp.size() == 0||assist_file.size() != temp[0])
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
		assist_file.push_back(temp_assist_file);
	}
	if (assist_file.size() == 0)
		return false;
	else
		return true;
}

void compute_water_area(Mat im, vector<assist_information> assist_files)
{
	for (auto assist_file : assist_files) {
		Mat r = GeoCorrect2Poly(assist_file, true);
		Mat r_inv = GeoCorrect2Poly(assist_file, false);
		vector<vector<double>> point1,point2;
		point1 = assist_file.point;
		point2 = swap_point(assist_file.point);
		double rms_1 = compute_rms(point1, r);
		double rms_2 = compute_rms(point2, r_inv);
	}
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

double compute_rms(vector<vector<double>> point, Mat r)
{

	return 0.0;
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

