// assist_water.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "water.h"

int main(int argc, char** argv)
{
	//// 制作模板xml
	//vector<Mat> model;
	//vector<string> files = getFiles("D:\\project\\water\\assist_water", "template", ".bmp");
	//for (auto &i : files) {
	//	Mat temp_im = imread(i, CV_LOAD_IMAGE_GRAYSCALE);
	//	model.push_back(temp_im.clone());
	//}
	//FileStorage fs("template.xml", FileStorage::WRITE);
	//fs << "model" << "[";
	//for (auto &i : model) {
	//	fs << i;
	//}
	//fs << "]";
	//fs.release();
	if (argc == 1) {
		cout << "please input image name\n";
		return 0;
	}
	if (argc != 2) {
		cout << "please only input image name\n";
		return 1;
	}
	string image_name(argv[1]);
	Mat image = imread(image_name, IMREAD_COLOR);
	if (!image.data)
	{
		cout << " No image data \n";
		return -1;
	}
	string base_name;
	for (int i = 0; i < image_name.size() - 4; ++i) {
		base_name += image_name[i];
	}
	string assist_file_name("assist_" + base_name + ".txt");
	string template_image_name("template.xml");
	bool flag;
	vector<Mat> template_image;
	flag = input_template(template_image_name, template_image);
	if (!flag) {
		cout << " template file error \n";
		return 2;
	}
	vector<assist_information> assist_files;
	flag = input_assist(assist_file_name, assist_files, template_image);
	if (!flag) {
		cout << " assist file error \n";
		return 3;
	}
	compute_water_area(image, assist_files);
}

