// water_line.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "water_line.h"
#include <iostream>


int main(int argc, char** argv)
{

	vector<vector<Mat>> model;
	string model_path(argv[2]);
	DEBUG_IF(1) {
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
	}
	char* imageName = argv[1];
	// 读入图像
	Mat image, imshow_image;
	image = imread(imageName, IMREAD_COLOR);
	if (argc < 2 || !image.data)
	{
		printf(" No image data \n ");
		return -1;
	}

	vector<Matx<float, 6, 1>> lines1, lines2, result;
	get_line(image, lines1, lines2);
	auto parallel_lines = get_parallel_lines(image, lines1, lines2);
	float water_line_number = segement_area(image, parallel_lines, model);
	// 显示平行线
	imshow_image = draw_line(image, parallel_lines);
	waitKey(0);
	return 0;
}
