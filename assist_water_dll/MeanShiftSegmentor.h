#pragma once

#include "msImageProcessor.h"
#include "BgImage.h"
#include "BgEdge.h"
#include "BgEdgeList.h"
#include "BgEdgeDetect.h"
#include "BgDefaults.h"


class MeanShiftSegmentor
{
public:
	MeanShiftSegmentor(void);
	~MeanShiftSegmentor(void);

	void SetImage(const Mat& _img);
	void Run();
	void fixResult();
	void ShowResult(const Vec3b& _color = Vec3b(0, 0, 255));

	Mat m_Img;		// Input Image
	Mat m_Result;
	Mat showImg;

	int m_SigmaS;
	float m_SigmaR;
	int m_MinRegion;
	SpeedUpLevel m_SpeedUpLevel;
	int   gradWindowRadius ;
	float mixtureParam;
	float threshold;
};

