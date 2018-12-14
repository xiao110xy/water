#include "stdafx.h"
#include "water.h"
#define no_water_flag false
#define night_way false
vector<string> getFiles(string folder, string firstname, string lastname)
{
	vector<string> files;
	//文件句柄  
	long long hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;   //大家可以去查看一下_finddata结构组成                            
								   //以及_findfirst和_findnext的用法，了解后妈妈就再也不用担心我以后不会编了  
	string p(folder);
	if (folder == "")
		p.append(".");
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
				string temp = p.assign(folder).append(fileinfo.name);
				files.push_back(p.assign(folder).append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	return files;
}


bool input_assist(Mat im,map<string, string> main_ini, vector<assist_information> & assist_files)
{
	//提取sift特征
	MySift sift(0, 5, 0.1, 10, 1.5, true);
	vector<vector<Mat>> gauss_pyr, dog_pyr;
	vector<KeyPoint> keypoints;
	Mat descriptors;
	sift.detect(im, gauss_pyr, dog_pyr, keypoints);
	sift.comput_des(gauss_pyr, keypoints, descriptors);

	fstream assist_file_name(main_ini["assist_txt"]);
	if (!assist_file_name)
		return false;
	// 用于边缘检测时排除干扰
	Mat temp_im = im.clone();
	//
	assist_files.clear();
	string temp_name;
	getline(assist_file_name, temp_name);
	vector<double> temp;
	get_number(temp_name, temp);
	int ruler_number = temp[0];
	while (!assist_file_name.eof())
	{
		temp.clear();
		assist_information temp_assist_file;
		temp_assist_file.correct_flag = false;

		// 多少个水尺
		temp_assist_file.ruler_number = ruler_number;
		//
		getline(assist_file_name, temp_name);
		get_number(temp_name, temp);
		if (temp.size() != 5) {
			if (temp.size() == 6) {
				temp_assist_file.roi_order = temp[3];
			}
			else
				break;
		}
		temp_assist_file.left_right_no_water = temp[temp.size()-2];
		int n_water = temp[0];
		temp_assist_file.ref_index = temp[1];
		input_assist_image(main_ini["assist_image"],temp_assist_file);
		// 读取模板图片
		string temp_name = string(main_ini["template"].begin(), main_ini["template"].end() - 4) 
				+ "_" + to_string((int)temp[2]) + string(main_ini["template"].end() - 4, main_ini["template"].end());
		Mat template_image = imread(temp_name, IMREAD_GRAYSCALE);
		if (!template_image.data)
			return false;
		temp_assist_file.base_image = template_image;
		temp_assist_file.length = temp[2];
		// roi
		vector<double> roi;
		getline(assist_file_name, temp_name);
		get_number(temp_name, roi);
		if (roi.size() != 4)
			break;
		temp_assist_file.roi = roi;
		//
		vector<double> sub_roi;
		getline(assist_file_name, temp_name);
		get_number(temp_name, sub_roi);
		if (sub_roi.size() != 4)
			break;
		temp_assist_file.sub_roi = sub_roi;
		// point
		vector<vector<double>> temp_point;
		for (int n = 0; n < temp[temp.size()-1]; ++n) {
			getline(assist_file_name, temp_name);
			vector<double> temp;
			get_number(temp_name, temp);
			temp_point.push_back(temp);
		}
		temp_assist_file.point = temp_point;
		temp_assist_file.water_number = 0;
		//
		temp_assist_file.keypoints = keypoints;
		temp_assist_file.descriptors = descriptors;
		// 如果没有，则保存
 		if (assist_files.size() == n_water - 1) {
			temp_assist_file.correct_flag = correct_control_point(temp_im, temp_assist_file);
			assist_files.push_back(temp_assist_file);
		}
		// 如果有了
		else if (assist_files.size() == n_water) {
			// 判断点是不是更多
			if (assist_files[n_water - 1].correct_score > 1.5)
				continue;
			temp_assist_file.correct_flag = correct_control_point(temp_im, temp_assist_file);
			if (assist_files[n_water - 1].correct_flag) {
				if (temp_assist_file.correct_flag) {
					//点的个数
					if (assist_files[n_water - 1].correct_point.size() < temp_assist_file.correct_point.size()) {
						assist_files[n_water - 1] = temp_assist_file;
					}
				}
			}
			else {
				// 有点
				if (temp_assist_file.correct_flag) {
					assist_files[n_water - 1] = temp_assist_file;
				}
				// 没点
				else{
					if (temp_assist_file.correct_score > assist_files[n_water - 1].correct_score) {
						assist_files[n_water - 1] = temp_assist_file;
					}
				}
			}
		}
		for (int i = 0; i < assist_files.size(); ++i) {
			assist_information temp_assist = assist_files[i];
			if (temp_assist.correct_score > 1.5) {
				int x1 = im.rows, x2 = 0;
				if (temp_assist.temp_correct_point.size() < 2)
					continue;
				for (int j = 0; j < temp_assist.temp_correct_point.size(); ++j) {
					if (x1 > (temp_assist.temp_correct_point[j])[0])
						x1 = (temp_assist.temp_correct_point[j])[0];
					if (x2 < (temp_assist.temp_correct_point[j])[0])
						x2 = (temp_assist.temp_correct_point[j])[0];
				}
				if (x2 - x1 < 2)
					continue;
				temp_im.colRange(x1+0.1*(x2-x1), x1 + 0.9*(x2 - x1)).setTo(0);
				for (int j = 0; j < keypoints.size(); ++j) {
					int x = keypoints[j].pt.x;
					if (x > x1 - im.cols / 10.0&&x < x2 + im.cols / 10.0) {
						descriptors.row(j).setTo(10);
					}
				}
			}
		}

	}
	if (assist_files.size() == 0)
		return false;
	else {
		vector<assist_information> temp = assist_files;
		assist_files.clear();
		for (int i = 0; i < temp.size(); ++i) {
			if (temp[i].roi_order == 0)
				continue;
			else
				assist_files.push_back(temp[i]);
		}
		return true;
	}
}
bool get_number(string line_string, vector<double>& temp)
{
	temp.clear();
	for (int i = 0; i < line_string.size(); ++i) {
		if ((line_string[i] >= 48 && line_string[i] <= 57)|| line_string[i] == 45 ){
			double temp_value = 0;
			int j = i;
			int n = -1;
			bool flag = true;
			for (; j < line_string.size(); ++j) {
				if (line_string[j] == 45) {
					flag = false;
					continue;
				}
				if (line_string[j] == 44 || line_string[j] == 59) {
					break;
				}
				if (line_string[j] >= 48 && line_string[j] <= 57) {
					temp_value = temp_value * 10 + line_string[j] - 48;
				}
				if (line_string[j] == 46)
					n = j;
			}
			temp_value = n == -1 ? temp_value : temp_value / pow(10, j - n - 1);
			if (!flag)
				temp_value = temp_value * -1;
			temp.push_back(temp_value);
			i = j;

		}
	}
	return true;
}
bool input_assist_image(string file_name, assist_information &assist_file)
{
	string temp = string(file_name.begin(), file_name.end() - 4) + "_" + to_string(assist_file.ref_index) + string(file_name.end() - 4, file_name.end());
	Mat temp_image = imread(temp, IMREAD_COLOR);
	if (!temp_image.data)
		return false;
	assist_file.assist_image = temp_image.clone();
	return true;
}
void compute_water_area(Mat im, vector<assist_information> &assist_files, string ref_name)
{
	for (auto &assist_file:assist_files) {
		// 延伸一部分水尺区域
		int add_row = 10.0*assist_file.base_image.rows/assist_file.length;
		// 旋转校正 少部分 包含对原始影像进行矫正
		assist_file.wrap_image = Mat::zeros(Size(assist_file.base_image.cols,
			add_row + assist_file.base_image.rows), CV_64F);
		//assist_file.correct2poly = false;
		Mat image_rotate = correct_image(im, assist_file);
		//判断是否为无图像区域
		//  判断是否全是水
		if (isblank(im,assist_file)) {
			assist_file.parrallel_left.clear();
			assist_file.parrallel_right.clear();
			assist_file.parrallel_lines.clear();
			continue;
		}
		int water_line = (1 - assist_file.water_number / (double)assist_file.length)*assist_file.base_image.rows;
		Mat water_line_point = Mat::zeros(Size(2, 2), CV_64F);
		water_line_point.at<double>(0, 0) = 0;
		water_line_point.at<double>(0, 1) = water_line;
		water_line_point.at<double>(1, 0) = assist_file.base_image.cols - 1;
		water_line_point.at<double>(1, 1) = water_line;
		water_line_point = compute_point(water_line_point, assist_file.r);
		assist_file.water_lines.clear();
		assist_file.water_lines.push_back(water_line_point.at<double>(0, 0));
		assist_file.water_lines.push_back(water_line_point.at<double>(0, 1));
		assist_file.water_lines.push_back(water_line_point.at<double>(1, 0));
		assist_file.water_lines.push_back(water_line_point.at<double>(1, 1));
	}


}
void opt_assist_files(vector<assist_information>& assist_files)
{
	//for (int i = 0; i < assist_files.size(); ++i) {
	//	if (assist_files[i].parrallel_lines.size() < 1)
	//		continue;
	//	else
	//		temp_assist_files.push_back(assist_files[i]);
	//}
	//stable_sort(assist_files.begin(), assist_files.end(),
	//	[](assist_information a, assist_information b) {return a.correct_score > b.correct_score; });
	vector<bool> assist_file_flag(assist_files.size(), true);
	for (int i = 0; i < assist_files.size(); ++i) {
		if (!assist_file_flag[i])
			continue;
		if (assist_files[i].parrallel_lines.size() == 0) {
			assist_file_flag[i] = false;
			continue;
		}
		int x1 = 9999, x2 = 0;
		x1 = x1 < assist_files[i].parrallel_lines[0].x ? x1 : assist_files[i].parrallel_lines[0].x;
		x1 = x1 < assist_files[i].parrallel_lines[1].x ? x1 : assist_files[i].parrallel_lines[1].x;
		x2 = x2 > assist_files[i].parrallel_lines[2].x ? x2 : assist_files[i].parrallel_lines[2].x;
		x2 = x2 > assist_files[i].parrallel_lines[3].x ? x2 : assist_files[i].parrallel_lines[3].x;
		for (int j = i + 1; j < assist_files.size(); ++j) {
			if (!assist_file_flag[j])
				continue;
			for (int k = 0; k < 4; ++k) {
				if (assist_files[j].parrallel_lines.size() == 0) {
					assist_file_flag[j] = false;
					continue;
				}
				if (assist_files[j].parrallel_lines[k].x >= x1 &&
					assist_files[j].parrallel_lines[k].x <= x2) {
					if (assist_files[i].correct_score > assist_files[j].correct_score) {
						assist_file_flag[j] = false;
					}
					else {
						assist_file_flag[i] = false;
					}
					break;

				}
			}
		}
	}
	for (int i = 0; i < assist_file_flag.size(); ++i) {
		if (!assist_file_flag[i]) {
			assist_files[i].parrallel_left.clear();
			assist_files[i].parrallel_right.clear();
			assist_files[i].parrallel_lines.clear();
			assist_files[i].water_lines.clear();
		}
	}
}
bool isgrayscale(Mat im)
{
	Mat temp = im.clone();
	if (temp.channels() == 3) {
		temp.convertTo(temp, CV_8UC3);
		cv::Vec3b * data = temp.ptr<cv::Vec3b>(0);
		float num = 0;
		for (int i = 0; i < temp.total(); ++i) {
			int score = abs((data[i])[0] - (data[i])[1]);
			score += abs((data[i])[1] - (data[i])[2]);
			score += abs((data[i])[0] - (data[i])[2]);
			if (score <3)
				++num;
		}
		if (num / temp.total() < 0.8) {
			return false;
		}
		else {
			return true;
		}
		return false;
	}
	else {
		return true;
	}

}
bool isblank(Mat im, assist_information &assist_file)
{
	int n_length = assist_file.base_image.rows / assist_file.length *5;
	// 旋转校正 多部分 包含对原始影像进行矫正
	assist_information temp_assist_file = assist_file;
	temp_assist_file.wrap_image = Mat::zeros(Size(temp_assist_file.base_image.cols * 3,
		temp_assist_file.wrap_image.rows), CV_64F);
	for (int i = 0; i < temp_assist_file.point.size(); ++i) {
		(temp_assist_file.point[i])[0] += temp_assist_file.base_image.cols;
	}
	temp_assist_file.correct2poly = true;
	Mat image_rotate = correct_image(im, temp_assist_file);
	// 根据颜色去除
	Mat temp_im;
	cvtColor(image_rotate, temp_im, CV_BGR2GRAY);
	int histSize = 256;
	float range[] = { 0, 256 };
	const float* histRange = { range };
	bool uniform = true; bool accumulate = false;
	Mat hist;
	calcHist(&temp_im, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);
	float temp1 = 0, temp2 = 0;
	for (int i = 0; i < 40; ++i) {
		temp1 += hist.at<float>(i, 0);
		temp2 += hist.at<float>(histSize - i - 1, 0);
	}
	temp1 = temp1 / (temp_im.rows*temp_im.cols);
	temp2 = temp2 / (temp_im.rows*temp_im.cols);
	if (temp1 > 0.9 || temp2 > 0.9)
		return true;
	float num_zeros = 0;
	// 根据超出区域来
	for (int i = 0; i < assist_file.mask.total(); ++i) {
		Vec3s temp = *(assist_file.mask.ptr<Vec3s>(0) + i);
		if (temp[0] == temp[1] && temp[1] == temp[2] && temp[0] == 1024)
			num_zeros = num_zeros + 1;
	}
	num_zeros = num_zeros / assist_file.base_image.total();
	if (num_zeros > 0.15)
		return true;
	//
	assist_file.expand_wrap_image = image_rotate.clone();

	Mat left_image = image_rotate.colRange(0, assist_file.base_image.cols).clone();
	Mat base_image = image_rotate.colRange(assist_file.base_image.cols, assist_file.base_image.cols * 2).clone();
	Mat right_image = image_rotate.colRange(assist_file.base_image.cols * 2, assist_file.base_image.cols * 3).clone();
	assist_file.left_image = left_image.clone();
	assist_file.right_image = right_image.clone();
	if (!isgrayscale(im)) {

		//
		Mat temp1 = base_image.rowRange(0, n_length).clone();
		Mat temp2 = left_image.rowRange(0, n_length).clone();
		Mat temp3 = right_image.rowRange(0, n_length).clone();
		float score1, score2;
		Mat temp;
		matchTemplate(temp1, temp2, temp, CV_TM_SQDIFF_NORMED);
		score1 = temp.at<float>(0, 0);
		matchTemplate(temp1, temp3, temp, CV_TM_SQDIFF_NORMED);
		score2 = temp.at<float>(0, 0);
		if (score1 < 0.1 && score2 < 0.1)
			return true;

	}
	else {
	
		Mat temp = image_rotate.rowRange(0, n_length).clone();
		vector<Mat> splt;
		split(temp, splt);
		temp = splt[0].clone();
		float score = 0;
		for (int i = 0; i < temp.total(); ++i) {
			if (*(temp.ptr<uchar>(0) + i) > 160)
				score += 1;
		}
		score = score / temp.total();
		if (score > 0.5)
			return false;
		//
		Mat temp1 = base_image.rowRange(0, n_length).clone();
		Mat temp2 = left_image.rowRange(0, n_length).clone();
		Mat temp3 = right_image.rowRange(0, n_length).clone();
		float score1, score2;
		matchTemplate(temp1, temp2, temp, CV_TM_SQDIFF_NORMED);
		score1 = temp.at<float>(0, 0);
		matchTemplate(temp1, temp3, temp, CV_TM_SQDIFF_NORMED);
		score2 = temp.at<float>(0, 0);
		if (score1 < 0.1 && score2 < 0.1)
			return true;

	}
	if (!isgrayscale(im)) {
		temp_assist_file.wrap_image = Mat::zeros(Size(temp_assist_file.base_image.cols * 7,
			temp_assist_file.wrap_image.rows), CV_64F);
		for (int i = 0; i < temp_assist_file.point.size(); ++i) {
			(temp_assist_file.point[i])[0] += 2*temp_assist_file.base_image.cols;
		}
		Mat image_rotate = correct_image(im, temp_assist_file);
		assist_file.expand_wrap_image = image_rotate.clone();
	}
	return false;
}
bool notall(Mat im, assist_information &assist_file)
{

	bool flag = false;
	int num_e = assist_file.length / 5;
	int n_length = assist_file.base_image.rows / (float)assist_file.length * 5;
	Mat left_image = assist_file.left_image;
	Mat right_image = assist_file.right_image;
	Mat temp_im = im.clone();
	if (!isgrayscale(temp_im)) {
		cvtColor(left_image, left_image, CV_BGR2GRAY);
		cvtColor(right_image, right_image, CV_BGR2GRAY);
		cvtColor(temp_im, temp_im, CV_BGR2GRAY);
	}


	Mat temp,temp1, temp2, temp3;
	for (int i = 0; i < num_e; ++i) {
		temp1 = temp_im.rowRange(i*n_length, (i + 1)*n_length);
		temp2 = left_image.rowRange(i*n_length, (i + 1)*n_length);
		temp3 = right_image.rowRange(i*n_length, (i + 1)*n_length);
		matchTemplate(temp1, temp2, temp, CV_TM_SQDIFF_NORMED);
		assist_file.scores1.push_back(temp.at<float>(0, 0));
		if (temp.at<float>(0, 0) < 0.1)
			flag = true;
		matchTemplate(temp1, temp3, temp, CV_TM_SQDIFF_NORMED);
		assist_file.scores2.push_back(temp.at<float>(0, 0));
		if (temp.at<float>(0, 0) < 0.1)
			flag = true;
	}
	return flag;
}

bool water_line_isok(int water_line, int all_length, int n_length)
{
	if (water_line<0)
		return false;
	if (water_line > all_length - n_length)
		return false;
	return true;
}

bool isTooHighLightInNight(Mat im, int &water_line, int &gray_value)
{
	int c = im.cols / 3;
	float n = 0;
	int temp_water_line = water_line;
	if (temp_water_line < 1)
		temp_water_line = im.rows-1;
	for(int i = 0;i< temp_water_line;++i)
		for (int j = c; j < 2 * c; ++j) {
			if (im.at<uchar>(i, j) > gray_value) {
				++n;
			}
		}
	n = n / c / temp_water_line;
	if (n < 0.4)
		return false;
	Mat temp = im(Range(0, temp_water_line), Range(c, c * 2));
	gray_value = otsu(temp);
	int num = 0;
	int temp_n = 0;
	for (int i = 0; i < temp.total(); ++i) {
		if (*(temp.ptr<uchar>(0) + i) > gray_value) {
			num += *(temp.ptr<uchar>(0) + i);
			++temp_n;
		}
	}
	gray_value = num / temp_n -1;
	if (gray_value >= 230) {
		gray_value = 230;
		return true;
	}
	else {
		return false;
	}
}

bool correct_control_point(Mat im, assist_information & assist_file)
{
	string model = "similarity";
	// 将原始影像进行校正
	if (isgrayscale(im)^isgrayscale(assist_file.assist_image))
		return false;
	if (!assist_file.assist_image.data)
		return false;
	Mat roi_image = assist_file.assist_image(
		Range(assist_file.roi[1], assist_file.roi[1] + assist_file.roi[3]),
		Range(assist_file.roi[0], assist_file.roi[0] + assist_file.roi[2])
	).clone();  
	//类对象
	MySift sift(0,3, 0.1, 10, 1.5, true);

	//待配准图像特征点检测和描述
	vector<vector<Mat>> gauss_pyr_2, dog_pyr_2;
	vector<KeyPoint> keypoints_2;
	Mat descriptors_2;
	sift.detect(roi_image, gauss_pyr_2, dog_pyr_2, keypoints_2);
	sift.comput_des(gauss_pyr_2, keypoints_2, descriptors_2);
	//最近邻与次近邻距离比匹配
	Ptr<DescriptorMatcher> matcher = new FlannBasedMatcher;
	std::vector<vector<DMatch>> dmatchs;
	if (assist_file.descriptors.total() < 5)
		return false;
	if (descriptors_2.total() < 5)
		return false;
	matcher->knnMatch(assist_file.descriptors, descriptors_2, dmatchs, 2);
	// 不使用roi
	Mat homography, match_line_image;
	vector<vector<double>> temp_point;
	vector<assist_registration> temp_assist_reg = xy_match(im, roi_image, dmatchs, assist_file.keypoints,keypoints_2,
													model,1);
	if (temp_assist_reg.size() == 1) {
		homography = temp_assist_reg[0].homography.clone();
		temp_point = temp_assist_reg[0].points;
		match_line_image = temp_assist_reg[0].match_line_image;
		assist_file.correct_score = 2 ;
		// 矫正
		vector<vector<double>> points = assist_file.point;
		for (auto &point : points) {
			double x = point[2] - assist_file.roi[0];
			double y = point[3] - assist_file.roi[1];
			point[2] = x * homography.at<float>(0, 0) + y * homography.at<float>(0, 1) + homography.at<float>(0, 2);
			point[3] = x * homography.at<float>(1, 0) + y * homography.at<float>(1, 1) + homography.at<float>(1, 2);
		}
		assist_file.point = points;
	}
	else {
		if (assist_file.roi_order == 0) {
			return false;
		}
		Mat assist_image = assist_file.assist_image(
			Range(assist_file.sub_roi[1], assist_file.sub_roi[1] + assist_file.sub_roi[3]),
			Range(assist_file.sub_roi[0], assist_file.sub_roi[0] + assist_file.sub_roi[2])
		).clone();
		Point result;
		Mat draw_image;
		float score = -2;
		int temp_r = 0, temp_c = 0;
		vector<Mat> splt;
		Mat temp = im.clone();
		if (isgrayscale(temp)) {
			split(temp, splt);
			temp = splt[0].clone();
		}
		if (isgrayscale(assist_image)) {
			split(assist_image, splt);
			assist_image = splt[0].clone();
		}
		//temp.colRange(0, 700).setTo(Scalar{ 0,0,0 });
		float score1 = -2,score2 = -2;
		geo_match(temp, assist_image,score1, draw_image, result, !isgrayscale(assist_image));
		if (score1 > score) {
			score = score1;
			temp_r = result.y;
			temp_c = result.x;
		}
		//geo_match(temp, roi_image, score2, draw_image, result, !isgrayscale(assist_image));
		//if (score2 > score) {
		//	score = score2;
		//	temp_r = result.y;
		//	temp_c = result.x;
		//}

 		int x1 = temp_c; x1 = x1 >= 0 ? x1 : 0;
		int x2 = temp_c + assist_file.roi[2]; x2 = x2 < im.cols ? x2 : im.rows;
		int y1 = temp_r; y1 = y1 >= 0 ? y1 : 0;
		int y2 = temp_r + assist_file.roi[3]; y2 = y2 < im.rows ? y2 : im.rows;
		assist_file.correct_score = score - 1 + (x2 - x1)*(y2 - y1) / double(assist_file.roi[2] * assist_file.roi[3]);

		if (isgrayscale(im) ^ isgrayscale(assist_image))
			assist_file.correct_score = assist_file.correct_score - 0.1;
		// 将原始影像进行校正

		homography = Mat::zeros(Size(3, 3), CV_32F);
		homography.at<float>(0, 0) = 1;homography.at<float>(1, 1) = 1;homography.at<float>(2, 2) = 1;
		homography.at<float>(0, 2) = temp_c;
		homography.at<float>(1, 2) = temp_r;
		vector<vector<double>> points = assist_file.point;
		for (auto &point : points) {
			double x = point[2] - assist_file.sub_roi[0];
			double y = point[3] - assist_file.sub_roi[1];
			point[2] = x * homography.at<float>(0, 0) + y * homography.at<float>(0, 1) + homography.at<float>(0, 2);
			point[3] = x * homography.at<float>(1, 0) + y * homography.at<float>(1, 1) + homography.at<float>(1, 2);
		}
		assist_file.point = points;
	}
	// 判断在尺子内的个数
	if (temp_point.size() == 0)
		return false;
	// 保存
	assist_file.temp_correct_point = temp_point;
	//
	Mat r_inv = GeoCorrect2Poly(assist_file, false);
	Mat point1, point2 = vector2Mat(temp_point);
	point1 = compute_point(point2.colRange(0, 2), r_inv);

	vector<vector<double>> correct_points;
	for (int i = 0; i < temp_point.size(); ++i) {
		if (point1.at<double>(i, 0) < -0.1*assist_file.base_image.cols)
			continue;
		if (point1.at<double>(i, 1) < -0.1*assist_file.base_image.rows)
			continue;
		if (point1.at<double>(i, 0) > 1.1*assist_file.base_image.cols)
			continue;
		if (point1.at<double>(i, 1) > 1.1*assist_file.base_image.rows)
			continue;
		vector<double> temp;
		temp.push_back(point1.at<double>(i, 0));
		temp.push_back(point1.at<double>(i, 1));
		temp.insert(temp.end(), temp_point[i].begin(), temp_point[i].end());
		correct_points.push_back(temp);
	}
	std::stable_sort(correct_points.begin(), correct_points.end(),
		[](vector<double> a, vector<double> b) {return a[1] > b[1]; });
	assist_file.correct_point = correct_points;

	return true;

}
vector<assist_registration> xy_match(const Mat & image_1, const Mat & image_2,  vector<vector<DMatch>>& dmatchs, vector<KeyPoint> keys_1, vector<KeyPoint> keys_2, string model, int  ruler_number)
{
	//获取初始匹配的关键点的位置
	vector<Point2f> point_1, point_2;
	vector<DMatch> data_dmatchs, temp_dmatchs;
	Mat matched_line;
	for (size_t i = 0; i < dmatchs.size(); ++i)
	{
		double dis_1 = dmatchs[i][0].distance;
		double dis_2 = dmatchs[i][1].distance;
		if ((dis_1 / dis_2) <0.8)//如果满足距离比关系
		{
			data_dmatchs.push_back(dmatchs[i][0]);//保存正确的dmatchs
			//data_dmatchs.push_back(dmatchs[i][1]);//保存正确的dmatchs
		}
	}
	drawMatches(image_1, keys_1, image_2, keys_2, data_dmatchs, matched_line, Scalar(0, 255, 0), Scalar(255, 0, 0), vector<char>(), 2);

	vector<assist_registration> result;
	for (int i = 0; i < ruler_number; ++i) {
		//
		assist_registration temp_reg;
		//
		vector<DMatch> right_matchs;
		vector<Mat> homographys, match_line_images;
		Mat homography, match_line_image;
		bool flag = match(image_1, image_2, data_dmatchs, keys_1, keys_2,
			model, right_matchs, homography, match_line_image);
		if (!flag||right_matchs.size()<6)
			return result;
		//homography = homography.inv();
 		temp_reg.flag = flag;
		temp_reg.homography = homography;
		temp_reg.match_line_image = match_line_image;
		// 点的个数
		vector<vector<double>> temp_point;
		for (auto i : right_matchs) {
			vector<double> temp;
			temp.push_back(keys_1[i.queryIdx].pt.x);
			temp.push_back(keys_1[i.queryIdx].pt.y);
			temp.push_back(keys_2[i.trainIdx].pt.x);
			temp.push_back(keys_2[i.trainIdx].pt.y);
			temp_point.push_back(temp);
		}
		vector<vector<double>> temp_points;
		for (int i = 0; i < temp_point.size()-1; ++i) {
			bool flag = true;
			for (int j = 0; j < temp_points.size(); ++j)
				for (int k = 0; k < temp_point[j].size();k=k+2) {
					double temp1 = abs((temp_point[i])[k] - (temp_points[j])[k]);
					double temp2 = abs((temp_point[i])[k+1] - (temp_points[j])[k+1]);
					if((temp1<10)&&(temp2<10))
						flag = false;
				}
			if (flag)
				temp_points.push_back(temp_point[i]);
		}
		if (temp_points.size() < 6)
			temp_reg.flag = false;
		if (!temp_reg.flag)
			return result;
		if (temp_reg.flag) {
			temp_reg.distance_to_left = homography.at<float>(0, 2);
			temp_reg.points = temp_points;
		}
		int left_x = image_1.rows, right_x = 0;
		for (auto i : temp_reg.points) {
			if (left_x > i[0])
				left_x = i[0];
			if (right_x < i[0])
				right_x = i[0];
		}
		// 去除误差较小的值
		temp_dmatchs.clear();
		Mat temp_homography = homography.inv();
		for (int j = 0; j < data_dmatchs.size(); ++j) {
			double x1 = keys_1[data_dmatchs[j].queryIdx].pt.x;
			double y1 = keys_1[data_dmatchs[j].queryIdx].pt.y;
			double x2 = keys_2[data_dmatchs[j].trainIdx].pt.x;
			double y2 = keys_2[data_dmatchs[j].trainIdx].pt.y;
			double rms1 = x1 * temp_homography.at<float>(0, 0) + y1 * temp_homography.at<float>(0, 1) + temp_homography.at<float>(0, 2) -x2;
			double rms2 = x1 * temp_homography.at<float>(1, 0) + y1 * temp_homography.at<float>(1, 1) + temp_homography.at<float>(1, 2) -y2;
			//double rms2 = 1;
			if (x1 > 2*left_x - right_x&&
				x1 < 2 * right_x - left_x)
				continue;
			if (rms1 > 5 || rms2 > 5)
				temp_dmatchs.push_back(data_dmatchs[j]);
		}
		data_dmatchs = temp_dmatchs;
		drawMatches(image_1, keys_1, image_2, keys_2, data_dmatchs, matched_line, Scalar(0, 255, 0), Scalar(255, 0, 0), vector<char>(), 2);
		result.push_back(temp_reg);
	}
	stable_sort(result.begin(), result.end(),
		[](assist_registration a, assist_registration b) {return a.distance_to_left < b.distance_to_left; });
	return result;
}

void GetImageThreshold(Mat im, int &bestLowThresh ,int& bestHighThresh)
{
	int nWidth = im.cols;
	int nHeight = im.rows;

	int nGrayHistogram[256];
	double por_nGrayHistogram[256];
	double var = 0;
	double maxVar = 0;

	const int GrayLevel = 256;
	double allEpt = 0;
	double Ept[3] = { 0,0,0 };
	double por[3] = { 0,0,0 };
	int lowThresh = 0;
	int highThresh = 0;


	for (int i = 0; i < GrayLevel; i++)
	{
		nGrayHistogram[i] = 0;
	}
	int nPixel;
	for (int i = 0; i < im.total(); ++i) {
		nPixel = *(im.ptr<uchar>(0)+i);
		nGrayHistogram[nPixel]++;		
	}

	int nSum = 0;
	for (int i = 0; i < GrayLevel; i++)
	{
		nSum += nGrayHistogram[i];
	}

	for (int i = 0; i < GrayLevel; i++)
	{
		por_nGrayHistogram[i] = 1.0*nGrayHistogram[i] / nSum;
	}

	for (int i = 0; i < GrayLevel; i++)
	{
		allEpt = i * por_nGrayHistogram[i];
	}

	for (lowThresh = 0; lowThresh < (GrayLevel - 1); lowThresh++)
		for (highThresh = (lowThresh + 1); highThresh < GrayLevel; highThresh++)
		{

			var = 0;
			Ept[0] = Ept[1] = Ept[2] = 0;
			por[0] = por[1] = por[2] = 0;

			for (int i = 0; i < lowThresh; i++)
			{
				por[0] += por_nGrayHistogram[i];
				Ept[0] += i * por_nGrayHistogram[i];
			}
			Ept[0] /= por[0];

			for (int i = lowThresh; i < highThresh; i++)
			{
				por[1] += por_nGrayHistogram[i];
				Ept[1] += i * por_nGrayHistogram[i];
			}
			Ept[1] /= por[1];

			for (int i = highThresh; i < GrayLevel; i++)
			{
				por[2] += por_nGrayHistogram[i];
				Ept[2] += i * por_nGrayHistogram[i];
			}
			Ept[2] /= por[2];

			for (int i = 0; i < 3; i++)
			{
				var += ((Ept[i] - allEpt)*(Ept[i] - allEpt)*por[i]);
			}

			if (var > maxVar)
			{
				maxVar = var;
				bestLowThresh = lowThresh;
				bestHighThresh = highThresh;
			}
		}
}

Mat correct_image(Mat im, assist_information &assist_file)
{
	// 对原始图像进行处理

	//
	Mat result,map_x,map_y;
	map_coord(assist_file, map_x, map_y);// map_x,map_y float型数据
	im.convertTo(im, CV_16UC3);
	remap(im, result, map_x, map_y, CV_INTER_CUBIC, 0,Scalar{ 1024,1024,1024 });
	assist_file.mask = result;
	result.convertTo(result, CV_8UC3);
	im.convertTo(im, CV_8UC3);
	assist_file.wrap_image = result;
	// 将roi 即水尺区域的平行线保存
	// 存在一些问题 需要使用拟合的方式进行一定的处理
	Vec4f line_para;
	Point2d point1, point2;
	vector<Point2f> points;
	// 左侧线段
	//Mat test_im = Mat::zeros(Size(2000, 2000), CV_8UC3);
	//Mat temp_points1 = Mat::zeros(Size(2,assist_file.wrap_image.rows), CV_64F);
	for (int i = 0; i < assist_file.wrap_image.rows; ++i) {
		points.push_back(Point2f(map_x.at<float>(i, 0), map_y.at<float>(i, 0)));
		//temp_points1.at<double>(i, 0) = map_x.at<float>(i, 0);
		//temp_points1.at<double>(i, 1) = map_y.at<float>(i, 0);
		//circle(test_im, points[i], 1, Scalar(255, 0, 0));
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
	//line(test_im, point1, point2, Scalar(0, 255, 0));
	assist_file.parrallel_lines.push_back(point1);
	assist_file.parrallel_lines.push_back(point2);
	// 右侧线段
	points.clear();
	for (int i = 0; i < assist_file.wrap_image.rows; ++i) {
		points.push_back(Point2f(map_x.at<float>(i, assist_file.wrap_image.cols-1), map_y.at<float>(i, assist_file.wrap_image.cols - 1)));
		//circle(test_im, points[i], 1, Scalar(255, 0, 0));
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
	//line(test_im, point1, point2, Scalar(0, 255, 0));
	assist_file.parrallel_lines.push_back(point1);
	assist_file.parrallel_lines.push_back(point2);
	return result;
}

void map_coord(assist_information & assist_file, Mat & map_x, Mat & map_y, int base_x, int base_y)
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
		temp1.col(i).setTo(i+ base_x);
	}
	temp1 = temp1.reshape(0,(int)temp1.total());
	Mat temp2 = Mat::zeros(assist_file.wrap_image.size(), CV_64F);
	for (int i = 0; i < temp2.rows; ++i) {
		temp2.row(i).setTo(i+ base_y);
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
	// 
	//if (assist_file.correct_flag) {
	//	Mat temp = Mat::zeros(Size(2,assist_file.correct_point.size()), CV_64F);
	//	for (int i = 0; i < assist_file.correct_point.size();++i) {
	//		temp.at<double>(i, 0) = (assist_file.correct_point[i])[0];
	//		temp.at<double>(i, 1) = (assist_file.correct_point[i])[1];
	//	}
	//	temp= compute_point(temp, r_inv);
	//	assist_file.correct_line = 0;
	//	for (int i = 0; i < temp.rows; ++i) {
	//		assist_file.correct_line = assist_file.correct_line > temp.at<double>(i, 1) ?
	//			assist_file.correct_line : temp.at<double>(i, 1);
	//	}
	//}

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
	Mat M; 

	if (assist_file.correct2poly) {
		M = Mat::zeros(Size(6, n), CV_64F);
		M.col(0).setTo(1);
		X.copyTo(M.col(1));
		Y.copyTo(M.col(2));
		XY.copyTo(M.col(3));
		XX.copyTo(M.col(4));
		YY.copyTo(M.col(5));
	}
	else {
		M = Mat::zeros(Size(4, n), CV_64F);
		M.col(0).setTo(1);
		X.copyTo(M.col(1));
		Y.copyTo(M.col(2));
		XY.copyTo(M.col(3));

	}
	Mat A, B;
	Mat temp = M.t()*M;
	A = (M.t()*M).inv()*M.t()*point.col(2);
	B = (M.t()*M).inv()*M.t()*point.col(3);
	Mat result = Mat::zeros(Size(2, 6), CV_64F);
	for (int i = 0; i < A.rows; ++i) {
		result.at<double>(i, 0) = A.at<double>(i, 0);
		result.at<double>(i, 1) = B.at<double>(i, 0);
	}
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



bool get_label_mask(Mat mask,int & label, Mat &label_mask, double length,int y_t)
{
	Mat data = mask.clone();
	vector<float> temp_label;
	for (int i = 0; i < mask.rows; ++i) {
		float temp = 0;
		for (int j = 0; j < mask.cols; ++j) {
			if (mask.at<int>(i, j) == label) {
				temp += 1;
			}
		}
		temp = temp / mask.cols;
		temp_label.push_back(temp);
	}
	for (int i = temp_label.size() - 1; i >=0 ; --i) {
		if (temp_label[i] > 0.8) {
			temp_label.erase(temp_label.begin() + i, temp_label.end());
			break;
		}
	}
	float label_t = (2.5 / length)*mask.rows;

	if (temp_label.size()<label_t)
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
	//findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE, Point());
	//if (contours.size()>7)
	//	return false;
	int c_step = floor(label_mask.cols*0.1);
	int y1_min = label_mask.rows, y1_max = 0, y2_min = label_mask.rows, y2_max = 0;
	for (int i = c_step; i < label_mask.cols- c_step; ++i) {
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


vector<float> process_score(vector<float> temp_score, float score_t1, float score_t2)
{
	int n = 0;
	for (int i = temp_score.size(); i > 0; --i) {
		if (temp_score[i - 1] > score_t2) {
			n = i;
			break;
		}
	}
	for (int i = n; i < temp_score.size(); ++i) {
		if (temp_score[i] < score_t1) {
			if (i == 0 && temp_score[i + 1] > score_t1)
				continue;
			if (i == 0 || i == temp_score.size() - 1) {
				temp_score.erase(temp_score.begin() + i, temp_score.end());
				break;
			}
			if (temp_score[i + 1] < score_t1) {
				temp_score.erase(temp_score.begin() + i, temp_score.end());
				break;
			}
		}
	}
	return temp_score;
}


float get_water_line_day(Mat gc_im,assist_information &assist_file,int water_line,float scale)
{
	//flip(gc_im, gc_im, 0);
	Mat im = assist_file.wrap_image.clone();
	int n_length = assist_file.base_image.rows / assist_file.length * 5;
	int c = im.cols;
	Mat mask;
	Mat aBgd, aFgd; 
	// 
	// 得到横线
	vector<vector<float>> line1, line2,temp_line;
	temp_line = get_line(im, line1, line2);
	// 去除较短线
	for (int i = 0; i < line2.size(); ++i) {
		if ((line2[i])[5] < im.cols /5)
			line2.erase(line2.begin() + i, line2.end());
	}
	for (int i = 0; i < temp_line.size(); ++i) {
		if ((temp_line[i])[5] < im.cols / 5)
			temp_line.erase(temp_line.begin() + i, temp_line.end());
	}
	//
	mask = Mat::zeros(gc_im.size(), CV_8U);
	mask.colRange(0.9*c, c*2.1).setTo(2);
	mask(Range(0, water_line), Range(0.9*c, c*2.1)).setTo(3);
	mask = draw_line(mask, temp_line, c, vector<uchar>{0});
	mask = draw_line(mask, line2, c, vector<uchar>{1});
	for (int i = 0; i < assist_file.base_image.rows; i = i + n_length) {
		Scalar sum1 = sum(assist_file.base_image(Range(i, i + n_length), Range(0, c / 2)));
		Scalar sum2 = sum(assist_file.base_image(Range(i, i + n_length), Range( c / 2,c)));


		Mat temp1 = mask(Range(i, i + n_length), Range(c, c*1.5));
		Mat temp2 = mask(Range(i, i + n_length), Range(c*1.5, c*2));
		
		if (sum1[0] < sum2[0]) {
			temp2.setTo(2 + (i < water_line), temp2 == 1);
		}
		else {
			temp1.setTo(2 + (i < water_line), temp1 == 0);

		}
	}
	//
	grabCut(gc_im, mask, {}, aBgd, aFgd, 1, cv::GC_INIT_WITH_MASK);
	//
	int water_line1 = get_mask_line(mask, n_length, 0.4, 3);
	int water_line2= get_mask_line(mask, 2*n_length, 0.2, 3);
	if (abs(water_line2 - water_line1) < n_length / 5)
		return (water_line2 + water_line1) / 2;
	Mat temp1, temp2;
	while (1) {
		int y1 = water_line1 - n_length > 0 ? water_line1 - n_length : 0;
		int y2 = water_line2 + 1.5*n_length < im.rows - 1 ? water_line2 + 1.5*n_length : im.rows - 1;
		temp1 = gc_im.rowRange(y1, y2).clone();
		temp2 = Mat::zeros(temp1.size(), CV_8UC1);
		int det_y = y2 - water_line2;
		temp2(Range(0, temp2.rows - det_y), Range(c, 2* c)).setTo(3);
		temp2(Range(temp2.rows - det_y, temp2.rows - 0.5*det_y), Range(c, 2* c)).setTo(2);
		//temp2(Range(temp2.rows- n_length, temp2.rows), Range(0.9*c, 2.1 * c)).setTo(2);
		//retinex_process(temp1, temp1);
		grabCut(temp1, temp2, {}, aBgd, aFgd, 2, cv::GC_INIT_WITH_MASK);
		int temp_water_line = get_mask_line(temp2, n_length, 0.3, 3);
		if (temp_water_line < n_length/5) {
			water_line1 = y1 - n_length;
			water_line2 = y2 - n_length;
		}
		else {
			water_line = y1 + temp_water_line;
			break;
		}

	}

	return water_line;
}

float get_water_line_day_nowater(Mat gc_im, assist_information & assist_file, int water_line, float scale)
{
	//flip(gc_im, gc_im, 0);
	Mat im = assist_file.wrap_image.clone();
	int n_length = assist_file.base_image.rows / assist_file.length * 5;
	int c = im.cols;
	Mat mask;
	Mat aBgd, aFgd;
	// 
	Mat temp_im = gc_im.colRange(2*c,5*c);
	Mat temp1, temp2;
	int temp_water_line,det_y;
	//
	mask = Mat::zeros(temp_im.size(), CV_8UC1) + 2;
	mask.rowRange(0, water_line).setTo(3);
	//mask.rowRange(water_line+6*n_length, mask.rows).setTo(2);
	grabCut(temp_im, mask, {},aBgd, aFgd, 2, cv::GC_INIT_WITH_MASK);
	Mat temp_mask;
	hconcat(mask.colRange(0, c), mask.colRange(2 * c, 3 * c), temp_mask);
	temp_water_line = get_mask_line(temp_mask, n_length, 0.4, 3);
	if (temp_water_line > water_line)
		return temp_water_line;
	else
		return water_line;
	//
}

int get_mask_line(Mat mask, int n_length,float scale,int class_n)
{
	int c = mask.cols / 3;
	vector<int> sum_bg(mask.rows, 1);
	for (int i = 0; i < mask.rows; ++i) {
		float num = 0;
		for (int j = c; j < c * 2; ++j) {
			if (mask.at<uchar>(i, j) == class_n|| mask.at<uchar>(i, j) == class_n-2) {
				++num;
			}
		}
		if (num < (scale*c)) {
			sum_bg[i] = 0;
		}
	}
	int r=0;
	for (int i = n_length; i < mask.rows - 1; ++i) {
		if (sum_bg[i] == 0)
			continue;
		float num_scale = 0;
		for (int j = i - n_length; j < i; ++j) {
			if (sum_bg[j] > 0)
				num_scale++;
		}
		num_scale = num_scale / n_length;
		if (num_scale > scale)
			r = i;
		else
			return r;
	}
	return r;
}
int get_water_line(assist_information &assist_file, float a, float b)
{
	Mat im = assist_file.wrap_image;
	Mat ref_image = assist_file.ref_image;
	//Mat im_s = color_tansform(im, ref_image);
	int length = assist_file.length;
	if (!ref_image.data)
		return -1;
	vector<float> score1, score2, score3, temp_score;
	vector<vector<float>> scores;
	float max_score = 0.8;
	int n = assist_file.base_image.rows / length;
	float det_score = 0;
	int c1 = im.cols / 2, c2 = im.cols;
	int temp_step_r = n/2 ;
	int temp_step_c = (c2 - c1) / 10;
	// 一个E
	int sum_d_r = 0;
	for (int i = 0; i < (int)length / 5; ++i) {
		// 一个E区域
		int r1 = i * 5 * n;
		int r2 = (i + 1) * 5 * n;

		Mat temp1 = i % 2 == 0 ? im(Range(r1+ sum_d_r, r2+ sum_d_r), Range(0, c1)) :
			im(Range(r1+ sum_d_r, r2+ sum_d_r), Range(c1, c2));
		Mat temp2 = i % 2 == 0 ? ref_image(Range(r1 + temp_step_r, r2 - temp_step_r), Range(0 + temp_step_c, c1 - temp_step_c)) :
			ref_image(Range(r1 + temp_step_r, r2 - temp_step_r), Range(c1 + temp_step_c, c2 - temp_step_c));
		//Mat temp3 = i % 2 == 0 ? im_s(Range(r1, r2), Range(0, c1)) :
		//	im(Range(r1, r2), Range(c1, c2));
		// 求score
		int d_r = 0;
		Mat temp;
		matchTemplate(temp1, temp2, temp, CV_TM_CCOEFF_NORMED);
		float score1 = -2;
		for (int j = 0; j < temp.rows; ++j) 
			for (int k=0;k<temp.cols;++k){
				if (score1 < temp.at<float>(j, k)) {
					score1=temp.at<float>(j, k);
					d_r = j;
				}
			}
		d_r = d_r - temp_step_r;
		sum_d_r = sum_d_r + d_r;
		//matchTemplate(temp3, temp2, temp, CV_TM_SQDIFF_NORMED);
		//float score2 = 2;
		//for (int j = 0; j < temp.total(); ++j) {
		//	score2 = score2 < (*(temp.ptr<float>(0) + j)) ? score2 : (*(temp.ptr<float>(0) + j));
		//}
		//  求最大score
		//float score = 1.0*score1;
		max_score = max_score > score1 ? max_score : score1;
		temp_score.push_back(score1);
	}
	scores.push_back(temp_score);
	// 后处理
	score1 = process_score(temp_score, 0.8*max_score,a* max_score+b);
	if (score1.size() == (length / 5))
		return im.rows + 2;
	temp_score.clear();
	// 一个E区域的1/5
	for (int i = 0; i < 5; ++i) {
		int r1 = score1.size() * 5 * n + (i - 1)* n;
		int r2 = score1.size() * 5 * n + (i + 1) * n;
		r1 = r1 >= 0 ? r1 : 0;
		r2 = r2 <= im.rows - 1 ? r2 : im.rows - 1;
		r1 = r1 < r2 ? r1 : r2;

		Mat temp1 = score1.size() % 2 == 0 ? im(Range(r1, r2), Range(0, c1)) :
			im(Range(r1, r2), Range(c1, c2));
		Mat temp2 = score1.size() % 2 == 0 ?
			ref_image(Range(r1, r2), Range(0 + temp_step_c, c1 - temp_step_c)) :
			ref_image(Range(r1, r2), Range(c1 + temp_step_c, c2 - temp_step_c));
		Mat temp;
		matchTemplate(temp1, temp2, temp, CV_TM_CCOEFF_NORMED);
		float score = -2;
		for (int i = 0; i < temp.total(); ++i) {
			score = score > (*(temp.ptr<float>(0) + i)) ? score : (*(temp.ptr<float>(0) + i));
		}
		temp_score.push_back(score);
	}
	scores.push_back(temp_score);

	// 后处理
	score2 = process_score(temp_score, 0.6*max_score,0.8*max_score);
	temp_score.clear();
	// 子部分的值
	for (int i = 0; i < n; ++i) {
		int r1 = score1.size() * 5 * n + score2.size() * n + i - 1;
		int r2 = r1 + 1;
		r1 = r1 >= 0 ? r1 : 0;
		r2 = r2 <= ref_image.rows - 1 ? r2 : ref_image.rows - 1;
		r1 = r1< r2 ? r1 : r2;
		if (r1 == r2) {
			temp_score.push_back(0.3*max_score + 0.1);
			break;
		}
		Mat temp1 = score1.size() % 2 == 0 ?
			im(Range(r1, r2), Range(0, c1)) :
			im(Range(r1, r2), Range(c1, c2));
		Mat temp2 = score1.size() % 2 == 0 ?
			ref_image(Range(r1, r2), Range(0 + temp_step_c, c1 - temp_step_c)) :
			ref_image(Range(r1, r2), Range(c1 + temp_step_c, c2 - temp_step_c));
		Mat temp;
		matchTemplate(temp1, temp2, temp, CV_TM_CCOEFF_NORMED);
		float score = -2;
		for (int i = 0; i < temp.total(); ++i) {
			score = score > (*(temp.ptr<float>(0) + i)) ? score : (*(temp.ptr<float>(0) + i));
		}
		temp_score.push_back(score);
	}
	scores.push_back(temp_score);
	assist_file.scores = scores;
	// 后处理
	score3 = process_score(temp_score, 0.3*max_score, 0.8*max_score);
	temp_score.clear();
	float water_number = length - 5 * score1.size() - score2.size() - score3.size() / 10.0;

	return (1 - water_number / (double)length)*ref_image.rows;
}

Mat getBinMaskByMask(Mat mask)
{
	Mat binmask(mask.size(), CV_8U);
	binmask = mask & GC_FGD;
	binmask = binmask * 255;
	Mat tmp;
	binmask.copyTo(tmp);
	vector<vector<Point> > co;
	vector<Vec4i> hi;
	binmask *= 0;
	findContours(tmp, co, hi, RETR_EXTERNAL, CHAIN_APPROX_NONE);
	for (int i = 0; i < co.size(); i++) {
		if (contourArea(Mat(co[i])) < 50) continue;
		drawContours(binmask, co, i, CV_RGB(255, 255, 255), CV_FILLED, CV_AA);
	}
	return binmask;
}

int get_best_line(Mat mask, int x, int y)
{
	if (x < 0 || y < 0)
		return -1;
	if (x >mask.cols-1 || y> mask.rows - 1)
		return -1;
	if (mask.at<uchar>(y,x) == 0)
		return y;
	else {
		mask.at<uchar>(y, x) = 0;
	}
	
	int temp1, temp2, temp3;
	temp1 = get_best_line(mask, x - 1, y);
	temp2 = get_best_line(mask, x+ 1, y);
	temp3 = get_best_line(mask, x, y+1);
	y = y > temp1 ? y : temp1;
	y = y > temp2 ? y : temp2;
	y = y > temp3 ? y : temp3;
	return y;
}

vector<vector<float>> get_line(Mat image, vector<vector<float>>& lines1, vector<vector<float>>& lines2, float det_v, float det_h)
{
	Mat data;
	cvtColor(image, data, CV_BGR2GRAY);
	// lsd计算直线
	vector<Matx<float, 4, 1>> temp_lines;
	xy_LineSegmentDetector ls;
	vector<float> width;
	ls.detect(data, temp_lines, width);
	

	vector<vector<float>> result_lines;

	// 存贮检测到的直线
	for (int i = 0; i != temp_lines.size(); ++i) {
		float x1, x2, y1, y2, distance, angle;
		x1 = temp_lines[i].val[0]; y1 = temp_lines[i].val[1];
		x2 = temp_lines[i].val[2]; y2 = temp_lines[i].val[3];
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
			vector<float> temp{ x1,y1,x2,y2,angle,distance,width[i] };
			lines1.push_back(temp);
		}
		// 水平直线      
		if (angle > 180 - det_h || angle < det_h) {
			if (x1 > x2) {
				float temp;
				temp = x1; x1 = x2; x2 = temp;
				temp = y1; y1 = y2; y2 = temp;
			}
			vector<float>  temp{ x1,y1,x2,y2,angle,distance ,width[i] };
			lines2.push_back(temp);
		}
		if (angle < 90 - 2*(det_h + det_v) &&angle >2 * (det_h + det_v)) {
			vector<float>  temp{ x1,y1,x2,y2,angle,distance ,width[i] };
			result_lines.push_back(temp);
		}
		if (angle < 180 - 2 * (det_h + det_v) && angle >90+ 2 * (det_h + det_v)) {
			vector<float>  temp{ x1,y1,x2,y2,angle,distance ,width[i] };
			result_lines.push_back(temp);
		}
	}
	// 按长度进行排序
	stable_sort(lines1.begin(), lines1.end(),
		[](vector<float> a, vector<float> b) {return a[5] > b[5]; });
	stable_sort(lines2.begin(), lines2.end(),
		[](vector<float> a, vector<float> b) {return a[5] > b[5]; });
	stable_sort(result_lines.begin(), result_lines.end(),
		[](vector<float> a, vector<float> b) {return a[5] > b[5]; });
	return result_lines;
}

Mat draw_line(Mat data, vector<vector<float>> lines, int base_x, vector<uchar> rgb)
{
	int width = 0;
	Mat result = data.clone();
	Scalar temp_rgb;
	if (result.channels() == 1) {
		auto a = lines.begin(); auto b = lines.end();
		for (auto i = a; i != b; ++i) {
			Point2f temp1 = Point2f((*i)[0] + base_x, (*i)[1]);
			Point2f temp2 = Point2f((*i)[2] + base_x, (*i)[3]);
			width =1>int((*i)[6])?1:int((*i)[6]);
			temp_rgb[0] = rgb[0];
			line(result, temp1, temp2, temp_rgb, width, 8);
		}
	}
	else{
		for (int i = 0; i < 3; ++i) {
			temp_rgb[i] = rgb[i];
		}
		auto a = lines.begin(); auto b = lines.end();
		for (auto i = a; i != b; ++i) {
			Point2f temp1 = Point2f((*i)[0] + base_x, (*i)[1]);
			Point2f temp2 = Point2f((*i)[2] + base_x, (*i)[3]);
			width = 1 > int((*i)[6]) ? 1 : int((*i)[6]);
			line(result, temp1, temp2, temp_rgb, width, 8);
		}
	}

	return result;
}



float get_water_line_night(Mat im,assist_information & assist_file)
{
	int water_line = -1;
	int add_rows = assist_file.wrap_image.rows- assist_file.base_image.rows;
	vector<Mat> splt;
	split(assist_file.expand_wrap_image, splt);
	Mat temp = splt[0].clone();
	int n_length = 3 * (double)assist_file.base_image.rows / assist_file.length;
	int c = temp.cols / 3;
	if (!assist_file.left_right_no_water) {
		int temp_t = otsu(temp.colRange(c, c * 2));
		threshold(temp, temp, temp_t, 255, CV_THRESH_BINARY);
		water_line = get_water_line_seg(temp.colRange(c, c * 2), assist_file.length, add_rows);
		int gray_value = 230;
		if (isTooHighLightInNight(splt[0], water_line, gray_value)) {
			threshold(splt[0], temp, gray_value, 255, CV_THRESH_BINARY);
			water_line = get_water_line_seg(temp.colRange(c, c * 2), assist_file.length, add_rows);
		}
		//if (water_line>temp.rows-5)
		//	water_line = get_water_line(assist_file);
	}
	else {
		threshold(temp, temp, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		Mat result;
		hconcat(temp.colRange(0.5*c,c), temp.colRange(c*2, c*2.5), result);
		int water_line2 = -1;
		water_line = get_water_line_seg(result, assist_file.length, add_rows);
		
		int gray_value = 230;
		if (isTooHighLightInNight(splt[0], water_line, gray_value)) {
			int temp_t = otsu(temp);
			threshold(temp, temp, temp_t, 255, CV_THRESH_BINARY);
			if (left_right_water(temp, n_length)) {
				water_line = get_water_line_seg(temp.colRange(c, 2 * c), assist_file.length, add_rows);
			}
			else {

				hconcat(temp.colRange(0.5*c, c), temp.colRange(c * 2, c*2.5), result);
				water_line = get_water_line_seg(result, assist_file.length, add_rows,0.05);
				//water_line = 0.5*water_line + 0.5*water_line2;
			}
		}
		//else {
		//	if (water_line > 0&&water_line2>0) {
		//		water_line = water_line2;

		//	}
		//}

	}



	return water_line;
}

float get_water_line_night_local(Mat im, assist_information & assist_file)
{
	int water_line = -1;
	int add_rows = assist_file.wrap_image.rows - assist_file.base_image.rows;
	vector<Mat> splt;
	split(assist_file.expand_wrap_image, splt);
	int c = splt[0].cols / 3;
	Mat temp,temp_im;
	int n_length = 5 * (double)assist_file.base_image.rows / assist_file.length;
	// 
	temp_im = splt[0].clone();
	threshold(temp_im, temp, 0, 255, CV_THRESH_BINARY| CV_THRESH_OTSU);
	float temp_t1 =0, temp_t2 = 2,temp_n =0;
	for (int i = 0; i < temp.rows; ++i)
		for (int j = 0; j < temp.cols; ++j){
			if (temp.at<uchar>(i, j) > 100) {
				temp_t1 += temp_im.at<uchar>(i, j);
				temp_n++;
			} 
			else
				temp_t2 += temp_im.at<uchar>(i, j);
		}
	temp_t1 = temp_t1 / temp_n;
	temp_t2 = temp_t2 / (temp_im.total()-temp_n);
	int water_line1 = get_water_line_seg(temp.colRange(c,c*2), assist_file.length, add_rows);
	//
	temp_im = splt[0].colRange(c, c * 2).clone();
	vector<int> score;
	for (int i = 0; i < temp_im.rows;) {
		int y1 = i;
		int y2 = i + 5*n_length;
		if (y2 > temp_im.rows) {
			if (y1 < temp_im.rows) {
				threshold(temp_im.rowRange(y1, temp_im.rows), temp_im.rowRange(y1, temp_im.rows), temp_t1, 255, CV_THRESH_BINARY);
			}
			break;
		}
		Mat temp1 = temp_im(Range(y1,y2), Range(0, c*0.5));
		Mat temp2 = temp_im(Range(y1,y2), Range(c*0.5, c));
		threshold(temp1, temp1, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		threshold(temp2, temp2, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		//
		i = y2;
	}
	temp_im.rowRange(temp_im.rows - add_rows, temp_im.rows).setTo(0);
	int water_line2 = get_water_line_seg(temp_im, assist_file.length, add_rows);



	return water_line1>water_line2?water_line1:water_line2;
}

bool left_right_water(Mat gc_im, int length)
{
	vector<Mat> splt;
	split(gc_im, splt);
	Mat im = splt[0].clone();
	Mat temp;
	threshold(im, temp, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	int n_length = 5 * im.rows / (float)length;
	int c = im.cols / 3;
	float num = 0;
	for (int i = 0; i < n_length; ++i) {
		int temp_num1 = 0,temp_num2 = 0;
		for (int j = 0; j < 1.5*c; ++j) {
			if (temp.at<uchar>(i, j) > 100)
				++temp_num1;
		}
		for (int j = 1.5*c; j < 3*c; ++j) {
			if (temp.at<uchar>(i, j) > 100)
				++temp_num2;
		}
		if ((temp_num1 > (0.6 * c))&& (temp_num2 > (0.6 * c)))
			++num;
	}
	if (num > 0.4*n_length)
		return false;
	else
		return true;


}

int get_water_line_seg(Mat im, int length, int add_rows, float scale)
{
	int n_length = 5 * (double)im.rows / length;
	int c = im.cols;
	bool ref_flag;
	//
	vector<double> im_score(im.rows, 0);
	for (int i = 0; i < im.rows; ++i) {
		int n = 0;
		for (int j = 0; j < im.cols; ++j) {
			if (im.at<uchar>(i, j) > 100)
				++n;
		}
		im_score[i] = n;
	}
	//
	ref_flag = false;
	int temp_n = 0;
	for (int i = 0; i < n_length; ++i) {
		if (im_score[i] < scale*im.cols)
			++temp_n;
	}
	if (temp_n > 0.8*n_length)
		ref_flag = true;
	if (ref_flag) {
		return -1;
	}
	//
	int seg_line = im.rows;
	for (int i = 0; i < im.rows - n_length; ++i) {
		if (im_score[i] < scale*c) {
			float temp_score = 0;
			for (int j = 0; j < n_length; ++j) {
				if (im_score[i + j] < scale*c) {
					temp_score++;
				}
			}
			if (temp_score > 0.8*n_length) {
				seg_line = i + 2*n_length;
				break;
			}
			else {
				i = i + 0.5*n_length;
			}
		}
	}
	for (int i = seg_line; i < im.rows; ++i) {
		im_score[i] = 0;
	}
	//
	vector<double> div1;
	vector<double> div2;
	for (int i = 0; i < im.rows; ++i) {
		if (i < n_length - 1 || i >= im.rows - n_length) {
			div1.push_back(0);
			div2.push_back(0);
			continue;
		}
		int n1 = 0;
		for (int j = i - n_length + 1; j <= i; ++j) {
			n1 += im_score[j];
			if (im_score[j] == 0)
				n1 -= 0.1 *c;
		}
		int n2 = 0;
		int n3 = 0;
		for (int j = i + 1; j <= i + n_length; ++j) {
			n2 += im_score[j];
			if (im_score[j] == 0) {
				n3 -= 0.1 *c;
			}
			else {
				n3 += im_score[j];
			}
		}
		div1.push_back(n1 - n2);
		div2.push_back(n1 - n3);
	}
	int water_line1 = 0;
	int water_line2 = 0;
	float score = 0;
	for (int i = 0; i < div1.size(); ++i) {
		if (score < div1[i]) {
			score = div1[i];
			water_line1 = i;
		}
	}
	score = 0;
	for (int i = 0; i < div2.size(); ++i) {
		if (score < div2[i]) {
			score = div2[i];
			water_line2 = i;
		}
	}
	if (water_line2 > im.rows - add_rows)
		return water_line2;
	// 判断是不是过长
	int n_flag = 0;
	for (int i = water_line2; i < water_line2 + add_rows; ++i) {
		if (im_score[i]< 0.5*c)
			++n_flag;
	}
	if (n_flag < 0.4*add_rows)
		return im.rows - add_rows + 1;
	else
		return water_line2;

}

void save_file(Mat im, vector<assist_information> assist_files, map<string, string> main_ini)
{
	//结果保存
	Mat result = im.clone();
	vector<vector<float>> temp_water_number;
	for (int i = 0; i < assist_files.size(); ++i) {

		assist_information temp = assist_files[i];
		// 子部分
		fstream _file;
		string temp_sub(main_ini["sub"].begin(), main_ini["sub"].end() - 4);
		_file.open(temp_sub +"_"+ to_string(temp.roi_order)+".bmp", ios::in);
		if (!_file) {
			cv::imwrite(temp_sub + "_" + to_string(temp.roi_order) + ".bmp", temp.wrap_image.rowRange(0,temp.base_image.rows));
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
		if (temp.parrallel_lines.size() == 4) {
			line(result, temp.parrallel_lines[0], temp.parrallel_lines[1], Scalar(255, 0, 0), 2);
			line(result, temp.parrallel_lines[2], temp.parrallel_lines[3], Scalar(255, 0, 0), 2);		
		}
		if (temp.water_lines.size() == 4){
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


	}
	// 读写图像
	cv::imwrite(main_ini["result_image"], result);
	//fstream _file;
	//_file.open(image_result, ios::in);
	//if (!_file) {
	//	imwrite(image_result, result);
	//}
	//_file.close();
	// 读写文件
	// 读写文件
	ofstream file(main_ini["result_txt"]);
	for (int i = 0; i < assist_files.size(); ++i) {
		assist_information temp = assist_files[i];
		file << "No=";
		file << (i + 1) << ";" << endl;
		file << "WaterLevel=";
		if (temp.water_lines.size() != 4|| temp.parrallel_lines.size() != 4)
			file << ";" << endl;
		else
			file << fixed << setprecision(1) << round(temp.water_number * 10) / 10 << ";" << endl;
		file << "GuageArea=";
		for (int j = 0; j < temp.parrallel_lines.size(); ++j) {
			file << fixed << setprecision(2) << temp.parrallel_lines[j].x;
			file << ",";
			file << fixed << setprecision(2) << temp.parrallel_lines[j].y;
			file << ",";
		}
		file << ";";
		file << endl;
		file << "WaterLine=";
		for (int j = 0; j < temp.water_lines.size(); ++j) {
			file << fixed << setprecision(2) << temp.water_lines[j];
			file << ",";
		}
		file << ";"<<endl;
		//for (auto j : temp.scores) {
		//	int m = 0;
		//	for (auto k : j) {
		//		file <<fixed << setprecision(2) << k;
		//		file << ";";
		//		++m;
		//		if (m == 5) {
		//			m = 0;
		//			file << endl;
		//		}
		//	}
		//	file <<'.'<< endl;
		//}
	}
	file.close();
	string cache_name;
	string temp = main_ini["assist_txt"];
	int n = 0;
	for (int i = temp.size()-1; i >=0; --i) {
		if (temp[i] == 47) {
			n = i + 1;
			break;
		}		
	}
	if (n == 0)
		cache_name = "temp_" + temp;
	else {
		cache_name = temp;
		temp = "temp_";
		cache_name.insert(cache_name.begin() + n, temp.begin(), temp.end());
	}
	ofstream file_cache(cache_name);
	for (int i = 0; i < assist_files.size(); ++i) {
		file_cache << "0,0,0,0,0," << (int)assist_files[i].point.size() << endl;
		for (int j = 0; j < 3; ++j) {
			file_cache << fixed << (int)assist_files[i].roi[j] << ",";
		}
		file_cache << fixed << (int)assist_files[i].roi[3] << ";" << endl;
		for (int j = 0; j < 3; ++j) {
			file_cache << fixed << (int)assist_files[i].sub_roi[j] << ",";
		}
		file_cache << fixed << (int)assist_files[i].sub_roi[3] << ";" << endl;
		for (int j = 0; j < assist_files[i].point.size(); ++j) {
			for (int k = 0; k < assist_files[i].point[j].size(); ++k) {
				file_cache << fixed << setprecision(2) << (assist_files[i].point[j])[k];
				if (k != 3)
					file_cache << ",";
				else
					file_cache << ";" << endl;
			}
		}
	}
	file_cache.close();
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

int otsu(Mat im)
{
	int nWidth = im.cols;
	int nHeight = im.rows;

	int nGrayHistogram[256];
	double por_nGrayHistogram[256];
	double var = 0;
	double maxVar = 0;

	const int GrayLevel = 256;
	double allEpt = 0;
	double Ept[3] = { 0,0,0 };
	double por[3] = { 0,0,0 };
	int lowThresh = 0;
	int highThresh = 0;


	for (int i = 0; i < GrayLevel; i++)
	{
		nGrayHistogram[i] = 0;
	}
	int nPixel;
	for (int i = 0; i < im.total(); ++i) {
		nPixel = *(im.ptr<uchar>(0) + i);
		nGrayHistogram[nPixel]++;
	}

	int nSum = 0;
	for (int i = 0; i < GrayLevel; i++)
	{
		nSum += nGrayHistogram[i];
	}

	for (int i = 0; i < GrayLevel; i++)
	{
		por_nGrayHistogram[i] = 1.0*nGrayHistogram[i] / nSum;
	}

	for (int i = 0; i < GrayLevel; i++)
	{
		allEpt = i * por_nGrayHistogram[i];
	}
	int x = 0;
	int y = 0;
	for (lowThresh = 0; lowThresh < (GrayLevel - 1); lowThresh++)
		for (highThresh = (lowThresh + 1); highThresh < GrayLevel; highThresh++)
		{

			var = 0;
			Ept[0] = Ept[1] = Ept[2] = 0;
			por[0] = por[1] = por[2] = 0;

			for (int i = 0; i < lowThresh; i++)
			{
				por[0] += por_nGrayHistogram[i];
				Ept[0] += i * por_nGrayHistogram[i];
			}
			Ept[0] /= por[0];

			for (int i = lowThresh; i < highThresh; i++)
			{
				por[1] += por_nGrayHistogram[i];
				Ept[1] += i * por_nGrayHistogram[i];
			}
			Ept[1] /= por[1];

			for (int i = highThresh; i < GrayLevel; i++)
			{
				por[2] += por_nGrayHistogram[i];
				Ept[2] += i * por_nGrayHistogram[i];
			}
			Ept[2] /= por[2];

			for (int i = 0; i < 3; i++)
			{
				var += ((Ept[i] - allEpt)*(Ept[i] - allEpt)*por[i]);
			}

			if (var > maxVar)
			{
				maxVar = var;
				x = lowThresh;
				y = highThresh;
			}
		}
	return y;
}
