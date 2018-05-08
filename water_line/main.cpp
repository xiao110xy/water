// water_line.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "water_line.h"

#include <iostream>


int main(int argc, char** argv)
{
	vector<vector<Mat>> model;
	DEBUG_IF(1) {
		if (argc == 4) {
			string model_path(argv[2]);
			for (int i = 0; i < 10; ++i) {
				stringstream stream;
				stream << i;
				vector<Mat> temp;
				vector<string> files = getFiles(model_path, stream.str(), ".jpg");
				for (auto &j : files) {
					Mat temp_im = imread(j, CV_LOAD_IMAGE_GRAYSCALE);
					resize(255 - temp_im, temp_im, Size(24, 48), INTER_LINEAR);
					temp_im.convertTo(temp_im, CV_32F);
					normalize(temp_im, temp_im, 1.0, 0.0, NORM_MINMAX);
					temp.push_back(temp_im.clone());
				}
				model.push_back(temp);
			}
			FileStorage fs("mat.xml", FileStorage::WRITE);
			fs << "model" << "[";
			for (auto &i : model) {
				fs << "[";
				for (auto &j : i) {
					fs << j;
				}
				fs << "]";
			}
			fs << "]";
			fs.release();
		}
		else {
			FileStorage fs("mat.xml", FileStorage::READ);
			fs["model"] >> model;
			if (model.size() == 0)
			{
				printf(" No model data \n ");
				return -1;
			}
		}
	}
	else {
		FileStorage fs("mat.xml", FileStorage::READ);
		fs["model"] >> model;
		if (model.size() == 0)
		{
			printf(" No model data \n ");
			return -1;
		}
	}
	// 读入图像

	if (argc == 1) {
		cout << "please input image name\n";
		return 0;
	}
	string image_name(argv[1]);
	string result_image, result_txt;
	Mat image = imread(image_name, IMREAD_COLOR);
	if (!image.data)
	{
		cout << " No image data \n";
		return -1;
	}
	if (argc == 2) {
		vector<water_result> water = segement_area(image, model);
		svaefile(image, image_name, water);
	}
	if (argc == 3) {
		vector<vector<int>> roi;
		fstream roi_file(argv[2]);
		while (!roi_file.eof())
		{
			int n = 0;
			vector<int> temp_roi(4,0);
			string temp_name;
			getline(roi_file, temp_name);
			if (temp_name[0] < 48 && temp_name[0]>57)
				break;
			for (int i = 0; i < temp_name.size();++i) {
				int temp_value = 0;
				int j=0;
				for (j = i; j < temp_name.size(); ++j) {
					if (temp_name[j] >= 48 && temp_name[j] <= 57) {
						temp_value = temp_value * 10 + temp_name[j] - 48;
					}
					else {
						break;
					}
				}
				temp_roi[n] = temp_value;
				++n;
				i = j;
			}
			if (n==4)
				roi.push_back(temp_roi);
		}
		// 取部分进行处理

		vector<water_result> water = segement_roi_area(image, model, roi);
		svaefile(image, image_name, water);
	}



	//auto all_file = getFiles("./", "", ".png");
	//for (auto &image_name : all_file) {
	//	Mat image = imread(image_name, IMREAD_COLOR);
	//	if (!image.data)
	//	{
	//		cout<<" No image data \n";
	//		return -1;
	//	}
	//	vector<water_result> water = segement_area(image, model);
	//	svaefile(image,image_name, water);
	//}
	//return 0;
}
