
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

	string file ="base.bmp" ;
	cin >> file;
	float base_n = 100;
	int n = 350;
	cin >> n;
	if (n<1)
		return 0;
	Mat im = imread(file,CV_LOAD_IMAGE_GRAYSCALE);
	int r = n / base_n * im.rows;
	Mat result = Mat::zeros(Size(im.cols, r), CV_8UC1);
	for (int i = r; i > 0; --i) {
		if (i - im.rows >= 0) {
			int r1 = i - im.rows;
			int r2 = i;
			im.copyTo(result.rowRange(r1, r2));
			i = i - im.rows+1;
		}
		else {
			im.rowRange(im.rows - i, im.rows).copyTo(result.rowRange(0, i));
			break;
		}
	}
	imwrite("cache.png",result);
}