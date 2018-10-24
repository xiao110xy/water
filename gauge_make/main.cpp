
#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <map>
using namespace std;
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"  
#include "opencv2/objdetect/objdetect.hpp"
using namespace cv;
int main(int argc, char *argv[]) {
	int n = 50;
	float base_n = 100;
	cin >> n;
	if (n<1)
		return 0;
	Mat im = imread("base.bmp");
	int r = n / base_n * im.rows;
	Mat result = Mat::zeros(Size(im.cols, r), CV_8UC3);
	for (int i = r; i > 0; --i) {
		if (i - im.rows >= 0) {
			im.copyTo(result.rowRange(i - im.rows, i));
			i = i - im.rows;
		}
		else {
			im.rowRange(im.rows - i, im.rows).copyTo(result.rowRange(0, i));
			break;
		}
	}
	imwrite("cache.bmp",result);
}