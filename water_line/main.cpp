// water_line.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "water_line.h"
#include <iostream>




int main(int argc, char** argv)
{
	char* imageName = argv[1];
	// 读入图像
	Mat image,img1;
	image = imread(imageName, IMREAD_COLOR);
	if (argc != 2 || !image.data)
	{
		printf(" No image data \n ");
		return -1;
	}
	vector<Matx<float, 6, 1>> lines1, lines2;
	get_line(image,lines1,lines2);
	get_parallel_lines(image, lines1, lines2);
	waitKey(0);
	return 0;
}
