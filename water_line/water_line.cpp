#include "stdafx.h"
#include "water_line.h"


water_line::water_line()
{
}


water_line::~water_line()
{
}

void get_line(Mat image)
{
	float det_v = 22.5; float det_h = 22.5;
	Mat data;
	cvtColor(image, data, CV_BGR2GRAY);
	// lsd计算直线
	Ptr<LineSegmentDetector> ls = createLineSegmentDetector(LSD_REFINE_STD);
	std::vector<Vec4f> lines_std;
	ls->detect(data, lines_std);
	//计算直线长度、角度
	auto b = lines_std.begin(); auto e = lines_std.end();
	for (auto i = b; i != b; ++i) {
		Vec4f temp(*i);
		int x = temp.val[0];

	}
	// 按长度进行排序
	// 竖直直线
	// 水平直线
	


	//ls->drawSegments(data, lines_std);
	//namedWindow("Test");
	//imshow("Test", data);

}
