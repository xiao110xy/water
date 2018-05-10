#include "stdafx.h"
#include "MeanShiftSegmentor.h"

//IMPLEMENT_DYNCRT_CLASS(MeanShiftSegmentor);

MeanShiftSegmentor::MeanShiftSegmentor(void)
{

	m_SigmaS = 10;
	m_SigmaR = 15;
	m_MinRegion = 25;
	m_SpeedUpLevel = MED_SPEEDUP;

	gradWindowRadius=4;
	mixtureParam = 0.3;
	threshold= 0.9;
}

MeanShiftSegmentor::~MeanShiftSegmentor(void)
{
}

void MeanShiftSegmentor::SetImage(const Mat & _img)
{
	m_Img = _img.clone();
	m_Result.create(m_Img.size(), CV_32SC1);
}


void MeanShiftSegmentor::Run()
{

	IplImage* image = new IplImage(m_Img);
	msImageProcessor *iProc = new msImageProcessor();

	imageType gtype= COLOR;
	int m_height, m_width;
	m_height	=	image->height;
	m_width		=	image->width;

	iProc->DefineImage((uchar*)image->imageData, gtype, m_height, m_width);	

	float speedUpThreshold=0.1f;
	// 计算weight map
	float *gradMap = new float[m_height * m_width];
	float *confMap = new float[m_height * m_width];
	BgEdgeDetect edgeDetector(gradWindowRadius);
	BgImage inputImage;
	inputImage.SetImage((uchar*)image->imageData, m_width, m_height, true);
	edgeDetector.ComputeEdgeInfo(&inputImage, confMap, gradMap);
	float *weightMap = new float[m_height * m_width];
	int i;
	for (i = 0; i < m_width*m_height; i++) {
		if (gradMap[i] > 0.02) {
			weightMap[i] = mixtureParam*gradMap[i] + (1 - mixtureParam)*confMap[i];
		}
		else {
			weightMap[i] = 0;
		}
	}
	// 
	Mat temp_gradMap = Mat::zeros(Size(m_width, m_height), CV_32F);
	for (i = 0; i < m_width*m_height; ++i) {
		float*temp = temp_gradMap.ptr<float>(0);
		temp[i] = gradMap[i];
	}
	Mat temp_confMap = Mat::zeros(Size(m_width, m_height), CV_32F);
	for (i = 0; i < m_width*m_height; ++i) {
		float*temp = temp_confMap.ptr<float>(0);
		temp[i] = confMap[i];
	}
	Mat temp_weightMap = Mat::zeros(Size(m_width, m_height), CV_32F); {
		for (i = 0; i < m_width*m_height; ++i) {
			float*temp = temp_weightMap.ptr<float>(0);
			temp[i] = weightMap[i];
		}
	}
	iProc->SetWeightMap(weightMap,threshold);
	//
	iProc->SetSpeedThreshold(speedUpThreshold);	//进行图像过滤
	iProc->Filter(m_SigmaS, m_SigmaR, m_SpeedUpLevel);
	iProc->FuseRegions(m_SigmaR, m_MinRegion);			//进行图像融合


	int L=m_width*m_height;
	int *labels2=(int*)malloc(L*sizeof(int));
	iProc->GetLabels(labels2);
	
	//int m_RegionNum=iProc->regionCount;

	for (int i = 0; i < m_height; i++)
	{
		int* ptr = m_Result.ptr<int>(i);
		for (int j = 0; j < m_width; j++)
		{
			ptr[j] = labels2[i*m_width+j];
		}
	}

	delete iProc;
	fixResult();
}

void MeanShiftSegmentor::fixResult()
{
	// 调整超像素编号，使得编号从0开始，且连续。返回超像素个数(最大编号+1）
	// 并不保证编号一定按从小到大排序
	Mat _fixedSp(m_Result.size(), CV_32SC1);

	int regionNum = 0;
	int* lookUpTable = new int[m_Result.cols*m_Result.rows];	// look up table,初始为-1,
	memset(lookUpTable, -1, m_Result.cols*m_Result.rows);
	for (int i = 0; i < m_Result.rows; i++)
	{
		const int* ptr = m_Result.ptr<int>(i);
		for (int j = 0; j < m_Result.cols; j++)
		{
			int spId = ptr[j];
			if (lookUpTable[spId] < 0)	// 当前Id在查找表中不存在，则加入查找表
			{
				lookUpTable[spId] = regionNum;
				regionNum++;
			}
		}
	}

	for (int i = 0; i < m_Result.rows; i++)
	{
		const int* ptrSp = m_Result.ptr<int>(i);
		int* ptrFix = _fixedSp.ptr<int>(i);
		for (int j = 0; j < m_Result.cols; j++)
		{
			int spId = ptrSp[j];
			if (lookUpTable[spId] >= 0)
			{
				ptrFix[j] = lookUpTable[spId];
			}
		}
	}
	m_Result.release();
	m_Result = _fixedSp.clone();
}

void MeanShiftSegmentor::ShowResult(const Vec3b & _color)
{
	int w = m_Img.cols, h = m_Img.rows;
	int label, top, bot, left, right;//, topl, topr, botl, botr;
	showImg = m_Img.clone();
	Mat mask(m_Img.size(), CV_8UC1);
	mask = 0;

	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			label = m_Result.at<int>(i, j);
			top = m_Result.at<int>((i - 1 > -1 ? i - 1 : 0), j);
			bot = m_Result.at<int>((i + 1 < h ? i + 1 : h - 1), j);
			left = m_Result.at<int>(i, (j - 1>-1 ? j - 1 : 0));
			right = m_Result.at<int>(i, (j + 1<w ? j + 1 : w - 1));
			if (label != top || label != left || label != right || label != bot)
				mask.at<uchar>(i, j) = 255;
		}
	}
	showImg.setTo(_color, mask);

}