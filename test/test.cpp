#include "stdafx.h"
#include "xy.h"
Mat imgSrc;//输入图像，彩色
Mat imgEdge;//边缘图像
Mat imgGray;//输入图像，灰度图像
Mat im;

int g_nThresh_low;
int g_nThresh_high;

//回调函数
void on_Trackbar(int, void*)
{
	//调用Canny边缘检测算法，imgGray为输入图像，imgEdge为边缘图像，g_nThresh_low与g_nThresh_high是两个阈值
	im = imgGray.clone();
	blur(im, im, Size(3, 3));
	Canny(im, imgEdge, g_nThresh_low, g_nThresh_high);
	imshow("Edge Detection", imgEdge);
}

int main(int argc,void **agrv)
{

	imgSrc = imread("12.jpg", CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_ANYCOLOR);
	cvtColor(imgSrc, imgGray, CV_RGB2GRAY);//彩色图像转换为灰度图
	im = imgGray.clone();
	imgEdge = xy_Canny(im,(double)0, (double)255,3,true);
	Edge_Detect(imgGray, 3);
	g_nThresh_low = 0;//初始化低阈值值0
	g_nThresh_high = 0;//初始化高阈值为0

	namedWindow("Edge Detection", CV_WINDOW_AUTOSIZE);//生成主窗口，用来容纳滚动条与图像
	imshow("Edge Detection", imgGray);
	namedWindow("Edge",CV_WINDOW_NORMAL);//生成主窗口，用来容纳滚动条与图像
	//创建一个滑动条，名字为Low:255，主窗口为Edge Detection，最大值为255，value为g_nThresh_low，回调函数为on_trackbar
	createTrackbar("Low: 255", "Edge", &g_nThresh_low, 1000, on_Trackbar);
	//创建一个滑动条，名字为High: 255，主窗口为Edge Detection，最大值为255，value为g_nThresh_high，回调函数为on_trackbar  
	createTrackbar("High: 255", "Edge", &g_nThresh_high, 1000, on_Trackbar);

	waitKey(0);
	return 0;
}
