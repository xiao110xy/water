// format_image.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"


int main(int argc,char *argv[])
{
	if (argc != 2)
		return -1;
	string name(argv[1]);
	Mat im = imread(name);
	imwrite(name, im);
	cout << "ok" << endl;
    return 0;
}

