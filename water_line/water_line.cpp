// water_line.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/core/core.hpp"

using namespace std;
using namespace cv;
int main()
{
	Mat image = imread("4.png");  //存放自己图像的路径 
	imshow("显示图像", image);
	waitKey(0);
	
    return 0;
}

