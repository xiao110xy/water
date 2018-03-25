// water_line.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "water_line.h"
#include <iostream>


int main(int argc, char** argv)
{
	vector<vector<Mat>> model;
	DEBUG_IF(1) {
		if (argc == 3){
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
		if (model.size()==0)
		{
			printf(" No model data \n ");
			return -1;
		}
	}
	// 读入图像
	auto all_file = getFiles("D:/project/water/water_line", "", ".png");
	for (auto &image_name : all_file) {
		Mat image = imread(image_name, IMREAD_COLOR);
		if (!image.data)
		{
			printf(" No image data \n ");
			return -1;
		}
		vector<water_result> water = segement_area(image, model);
		svaefile(image_name, water);
	}

	return 0;
}
