//***********************************************************************
// Project		    : GeoMatch
// Author           : Shiju P K
// Email			: shijupk@gmail.com
// Created          : 10-01-2010
//
// File Name		: GeoMatch.cpp
// Last Modified By : Shiju P K
// Last Modified On : 13-07-2010
// Description      : class to implement edge based template matching
//
// Copyright        : (c) . All rights reserved.
//***********************************************************************

#include "stdafx.h"
#include "GeoMatch.h"


GeoMatch::GeoMatch(void)
{
	noOfCordinates = 0;  // Initilize  no of cppodinates in model points
	modelDefined = false; 
}
// destructor
GeoMatch::~GeoMatch(void)
{

}

int GeoMatch::CreateGeoMatchModel(Mat &src,double maxContrast,double minContrast)
{
	//Mat mask;
	//threshold(src, mask,0, 255, CV_THRESH_OTSU | CV_THRESH_BINARY);
	//vector <vector<Point>>contours;
	//findContours(mask,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
	src.convertTo(src, CV_8UC1);

	modelHeight = src.rows;		//Save Template height
	modelWidth =src.cols;			//Save Template width

	noOfCordinates=0;											//initialize	
	


	// Calculate gradient of Template
	Mat gx = Mat::zeros(src.size(), CV_16SC1);
	Mat gy = Mat::zeros(src.size(), CV_16SC1);

	Sobel( src, gx, CV_16S,1,0, 3);		//gradient in X direction			
	Sobel( src, gy, CV_16S, 0, 1, 3 );	//gradient in Y direction
	
	Mat nmsEdges = Mat::zeros(src.size(), CV_32F);		//create Matrix to store Final nmsEdges

	double fdx,fdy;	
    double MagG, DirG;
	double MaxGradient=-99999.99;
	double direction;
	Mat orients = Mat::zeros(src.size(), CV_8UC1);
	int count = 0,i,j; // count variable;
	
	Mat magMat = Mat::zeros(src.size(), CV_64FC1);
	
	for( i = 1; i < src.rows-1; i++ )
    {
    	for( j = 1; j < src.cols -1; j++ )
        { 		 

			fdx = gx.at<short>(i,j); 
			fdy = gy.at<short>(i, j);        // read x, y derivatives

			MagG = sqrt((float)(fdx*fdx) + (float)(fdy*fdy)); //Magnitude = Sqrt(gx^2 +gy^2)
			direction =cvFastArctan((float)fdy,(float)fdx);	 //Direction = invtan (Gy / Gx)
			magMat.at<double>(i,j) = MagG;
				
			if(MagG>MaxGradient)
				MaxGradient=MagG; // get maximum gradient value for normalizing.

				
			// get closest angle from 0, 45, 90, 135 set
                if ( (direction>0 && direction < 22.5) || (direction >157.5 && direction < 202.5) || (direction>337.5 && direction<360)  )
                    direction = 0;
                else if ( (direction>22.5 && direction < 67.5) || (direction >202.5 && direction <247.5)  )
                    direction = 45;
                else if ( (direction >67.5 && direction < 112.5)||(direction>247.5 && direction<292.5) )
                    direction = 90;
                else if ( (direction >112.5 && direction < 157.5)||(direction>292.5 && direction<337.5) )
                    direction = 135;
                else 
					direction = 0;
				
			orients.at<uchar>(i, j) = direction;
		}
	}
	

	// non maximum suppression
	double leftPixel,rightPixel;
	
	for (i = 1; i < src.rows - 1; i++)
	{
		for (j = 1; j < src.cols - 1; j++)
		{
				switch (orients.at<uchar>(i, j))
                {
                   case 0:
                        leftPixel  = magMat.at<double>(i, j-1);
						rightPixel = magMat.at<double>(i, j+1);
                        break;
                    case 45:
                        leftPixel  = magMat.at<double>(i-1, j +1); //magMat[i - 1][j + 1];
						rightPixel = magMat.at<double>(i+1, j - 1); //magMat[i+1][j-1];
                        break;
                    case 90:
                        leftPixel  = magMat.at<double>(i - 1, j);// magMat[i - 1][j];
						rightPixel = magMat.at<double>(i + 1, j );// magMat[i+1][j];
                        break;
                    case 135:
                        leftPixel  = magMat.at<double>(i - 1, j-1);// magMat[i-1][j-1];
						rightPixel = magMat.at<double>(i + 1, j+1);//magMat[i+1][j+1];
                        break;
				 }
				// compare current pixels value with adjacent pixels
                if ((magMat.at<double>(i, j) < leftPixel ) || (magMat.at<double>(i, j) < rightPixel ) )
					nmsEdges.at<float>(i, j)=0;
                else
					nmsEdges.at<float>(i, j) =(magMat.at<double>(i, j) /MaxGradient*255);
			
			}
		}
	

	int XSum=0, YSum =0;
	int curX,curY;
	int flag=1;
	noOfCordinates = 0;
	int max_value = 0;
	for (int i = 0; i < magMat.total(); ++i) {
		if (*(magMat.ptr<double>(0) + i) > max_value)
			max_value = *(magMat.ptr<double>(0) + i);
	}
	vector<int> temp_value(max_value + 2, 0);
	for (int i = 0; i < magMat.total(); ++i) {
		++temp_value[floor(*(magMat.ptr<double>(0) + i))];
	}
	max_value = 0;
	for (int i = temp_value.size() - 1; i >= 0;--i) {
		max_value = max_value + temp_value[i];
		if (max_value > magMat.total()*0.2) {
			maxContrast = i/ MaxGradient*255;
			minContrast = maxContrast *0.3;
			break;
		}
	}
	//Hysterisis threshold
	for (i = 1; i < src.rows - 1; i++)
	{
		for (j = 1; j < src.cols - 1; j++)
		{
			fdx = gx.at<short>(i, j);
			fdy = gy.at<short>(i, j);        // read x, y derivatives	
			MagG = sqrt(fdx*fdx + fdy*fdy); //Magnitude = Sqrt(gx^2 +gy^2)
			DirG =cvFastArctan((float)fdy,(float)fdx);	 //Direction = tan(y/x)
			flag=1;
			if(nmsEdges.at<float>(i, j) < maxContrast)
			{
				if(nmsEdges.at<float>(i, j) < minContrast)
				{
					nmsEdges.at<float>(i, j) =0;
					flag=0; // remove from edge
				}
				else
				{   // if any of 8 neighboring pixel is not greater than max contraxt remove from edge
					if( (nmsEdges.at<float>(i-1, j) < maxContrast)	&&
						(nmsEdges.at<float>(i-1, j)< maxContrast)	&&
						(nmsEdges.at<float>(i-1, j) < maxContrast)	&&
						(nmsEdges.at<float>(i, j-1) < maxContrast)		&&
						(nmsEdges.at<float>(i, j+1) < maxContrast)		&&
						(nmsEdges.at<float>(i+1, j-1) < maxContrast)	&&
						(nmsEdges.at<float>(i+1, j)< maxContrast)		&&
						(nmsEdges.at<float>(i+1, j+1) < maxContrast)	)
					{
						nmsEdges.at<float>(i, j) =0;
						flag=0;
						////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
					}
				}
				
			}
			// save selected edge information
			curX = j;	curY = i;
			if (flag != 0)
			{
				if (fdx != 0 || fdy != 0)
				{
					XSum = XSum + curX;
					YSum = YSum + curY; // Row sum and column sum for center of gravity

					cordinates.push_back(Point(curX, curY));

					edgeDerivativeX.push_back(fdx);
					edgeDerivativeY.push_back(fdy);

					//handle divide by zero
					if (MagG != 0)
						edgeMagnitude.push_back(1 / MagG);  // gradient magnitude 
					else
						edgeMagnitude.push_back(0);  // gradient magnitude 

					noOfCordinates++;
				}
			}
		}
	}
	//int n = 0;
	//for (i = 1; i < nmsEdges.rows - 1; i++)
	//{
	//	n = 0;
	//	for (j = 1; j < nmsEdges.cols/2; j++)
	//	{
	//		fdx = gx.at<short>(i, j);
	//		fdy = gy.at<short>(i, j);
	//		if (nmsEdges.at<float>(i,j)> 0)
	//		{
	//			if (fdx != 0 || fdy != 0)
	//			{
	//				nmsEdges.at<float>(i, j) = 0;
	//				cordinates.push_back(Point(j, i));
	//				edgeDerivativeX.push_back(fdx);
	//				edgeDerivativeY.push_back(fdy);
	//				if (MagG != 0)
	//					edgeMagnitude.push_back(1 / MagG);  // gradient magnitude 
	//				else
	//					edgeMagnitude.push_back(0);  // gradient magnitude 
	//				noOfCordinates++;
	//				n++;
	//				if (n==2)
	//					break;
	//			}
	//		}
	//	}
	//	n = 0;
	//	for (j = nmsEdges.cols - 2; j > nmsEdges.cols/2; --j)
	//	{
	//		fdx = gx.at<short>(i, j);
	//		fdy = gy.at<short>(i, j);
	//		if (nmsEdges.at<float>(i, j) > 0)
	//		{
	//			if (fdx != 0 || fdy != 0)
	//			{
	//				nmsEdges.at<float>(i, j) = 0;
	//				cordinates.push_back(Point(j, i));
	//				edgeDerivativeX.push_back(fdx);
	//				edgeDerivativeY.push_back(fdy);
	//				if (MagG != 0)
	//					edgeMagnitude.push_back(1 / MagG);  // gradient magnitude 
	//				else
	//					edgeMagnitude.push_back(0);  // gradient magnitude 
	//				noOfCordinates++;
	//				n++;
	//				if (n == 2)
	//					break;
	//			}
	//		}
	//	}
	//}
	//for (j = 1; j < nmsEdges.cols - 1; j++)
	//{
	//	n = 0;
	//	for (i = 1; i < nmsEdges.rows/2; i++)
	//	{
	//		fdx = gx.at<short>(i, j);
	//		fdy = gy.at<short>(i, j);
	//		if (nmsEdges.at<float>(i, j) > 0)
	//		{
	//			if (fdx != 0 || fdy != 0)
	//			{
	//				nmsEdges.at<float>(i, j) = 0;
	//				cordinates.push_back(Point(j, i));
	//				edgeDerivativeX.push_back(fdx);
	//				edgeDerivativeY.push_back(fdy);
	//				if (MagG != 0)
	//					edgeMagnitude.push_back(1 / MagG);  // gradient magnitude 
	//				else
	//					edgeMagnitude.push_back(0);  // gradient magnitude 
	//				noOfCordinates++;
	//				n++;
	//				if (n == 2)
	//					break;
	//			}
	//		}
	//	}
	//}
	//float m_step = 1 > noOfCordinates / 200.0 ? 1 : noOfCordinates / 200.0;
	//vector< Point> temp_cordinates = cordinates;
	//vector< double> temp_edgeDerivativeX = edgeDerivativeX;
	//vector< double> temp_edgeDerivativeY = edgeDerivativeY;
	//vector< double> temp_edgeMagnitude = edgeMagnitude;
	//cordinates.clear();
	//edgeDerivativeX.clear();
	//edgeDerivativeY.clear();
	//edgeMagnitude.clear();
	//noOfCordinates = 0;
	//for(int m=0;m< temp_cordinates.size();m = m+m_step)
	//{
	//	cordinates.push_back(temp_cordinates[m]);
	//	edgeDerivativeX.push_back(temp_edgeDerivativeX[m]);
	//	edgeDerivativeY.push_back(temp_edgeDerivativeY[m]);
	//	edgeMagnitude.push_back(temp_edgeMagnitude[m]);
	//	noOfCordinates++;
	//}
	//centerOfGravity.x = XSum /noOfCordinates; // center of gravity
	//centerOfGravity.y = YSum /noOfCordinates ;	// center of gravity
	////	
	//// change coordinates to reflect center of gravity
	//for(int m=0;m< noOfCordinates;m++)
	//{
	//	int temp;
	//	temp=cordinates[m].x;
	//	cordinates[m].x=temp- centerOfGravity.x;
	//	temp=cordinates[m].y;
	//	cordinates[m].y =temp- centerOfGravity.y;
	//}
	

	modelDefined=true;
	return 1;
}

void GeoMatch::MatchModelBetter(Mat &src)
{
	//vector< Point> temp_cordinates = cordinates;
	//vector< double> temp_edgeDerivativeX = edgeDerivativeX;
	//vector< double> temp_edgeDerivativeY = edgeDerivativeY;
	//vector< double> temp_edgeMagnitude = edgeMagnitude;
	//cordinates.clear();
	//edgeDerivativeX.clear();
	//edgeDerivativeY.clear();
	//edgeMagnitude.clear();
	//noOfCordinates = 0;
	//for(int m=0;m< temp_cordinates.size();m = m+1)
	//{
	//	cordinates.push_back(temp_cordinates[m]);
	//	edgeDerivativeX.push_back(temp_edgeDerivativeX[m]);
	//	edgeDerivativeY.push_back(temp_edgeDerivativeY[m]);
	//	edgeMagnitude.push_back(temp_edgeMagnitude[m]);
	//	noOfCordinates++;
	//}
}


double GeoMatch::FindGeoMatchModel(Mat src,double minScore,double greediness, Point &resultPoint, Mat assist_score)
{

	
	double resultScore=0;
	double partialSum=0;
	double sumOfCoords=0;
	double partialScore;

	int i,j,m ;			// count variables
	double iTx,iTy,iSx,iSy;
	double gradMag;    
	int curX,curY;

	Mat magMat = Mat::zeros(src.size(), CV_64FC1);


	if(!modelDefined)
	{
		return 0;
	}

	// source image size



	Mat Sdx = Mat::zeros(src.size(), CV_16SC1);
	Mat Sdy = Mat::zeros(src.size(), CV_16SC1);
	Sobel(src, Sdx, CV_16S, 1, 0, 3);		//gradient in X direction			
	Sobel(src, Sdy, CV_16S, 0, 1, 3);	//gradient in Y direction


		
	// stoping criterias to search for model
	//double normMinScore = minScore /noOfCordinates; // precompute minumum score 
	//double normGreediness = ((1- greediness * minScore)/(1-greediness)) /noOfCordinates; // precompute greedniness 
	double normMinScore = minScore /noOfCordinates; // precompute minumum score 
	double normGreediness = ((1- greediness * minScore)/(1-greediness)) /noOfCordinates; // precompute greedniness 

	for( i = 0; i < src.rows; i++ )
    {

		 for( j = 0; j < src.cols; j++ )
		{
				iSx= Sdx.at<short>(i,j);  // X derivative of Source image
				iSy= Sdy.at<short>(i,j);  // Y derivative of Source image

				gradMag=sqrt((iSx*iSx)+(iSy*iSy)); //Magnitude = Sqrt(dx^2 +dy^2)
							
				if(gradMag!=0) // hande divide by zero
					magMat.at<double>(i,j)=1/gradMag;   // 1/Sqrt(dx^2 +dy^2)
				else
					magMat.at<double>(i, j) =0;
				
		}
	}

	float temp_score = -1;
	for (i = 0; i < assist_score.rows; i++)
		for (j = 0; j < assist_score.cols; j++){
			if (temp_score < assist_score.at<float>(i, j))
				temp_score = assist_score.at<float>(i, j);
		}
	for (i = 0; i < assist_score.rows/2; i++)
	{
			for( j = 0; j < assist_score.cols; j++ )
             { 
				int temp_r = i-centerOfGravity.y, temp_c = j- centerOfGravity.x;
				if (temp_r<0 || temp_c<0 || temp_r> assist_score.rows - 1 || temp_c> assist_score.cols - 1)
					;
				else
					if (!color_flag)
						if (assist_score.at<float>(i, j) < 0.3*temp_score)
						continue;
				 partialSum = 0; // initilize partialSum measure
				 for(m=0;m<noOfCordinates;m++)
				 {
					 curX	= i + cordinates[m].y ;	// template X coordinate
					 curY	= j + cordinates[m].x ; // template Y coordinate
					 iTx	= edgeDerivativeX[m];	// template X derivative
					 iTy	= edgeDerivativeY[m];    // template Y derivative

					 if(curX<0 ||curY<0||curX> src.rows -1 ||curY> src.cols -1)
						 continue;
					 
					 iSx=Sdx.at<short>(curX,curY); // get curresponding  X derivative from source image
					 iSy=Sdy.at<short>(curX, curY);// get curresponding  Y derivative from source image
						
					if((iSx!=0 || iSy!=0) && (iTx!=0 || iTy!=0))
					 {
						 //partial Sum  = Sum of(((Source X derivative* Template X drivative) + Source Y derivative * Template Y derivative)) / Edge magnitude of(Template)* edge magnitude of(Source))
						 partialSum = partialSum + ((iSx*iTx)+(iSy*iTy))*(edgeMagnitude[m] * magMat.at<double>(curX, curY));
									
					 }

					sumOfCoords = m + 1;
					partialScore = partialSum /sumOfCoords ;
					// check termination criteria
					// if partial score score is less than the score than needed to make the required score at that position
					// break serching at that coordinate.
					//normMinScore = resultScore / noOfCordinates; // precompute minumum score 
					//normGreediness = ((1 - greediness * resultScore) / (1 - greediness)) / noOfCordinates; // precompute greedniness 

					//if( partialScore < (MIN((1-resultScore ) + normGreediness*sumOfCoords,normMinScore*  sumOfCoords)))
					//	break;

				}
				if(partialScore > resultScore)
				{
					resultScore = partialScore; //  Match score
					resultPoint.y = i;			// result coordinate X		
					resultPoint.x = j;			// result coordinate Y
				}
			} 
		}
	
	// free used resources and return score


	
	return resultScore;
}




// draw contours around result image
void GeoMatch::DrawContours(Mat &source, Point  COG,CvScalar color,int lineWidth)
{
	vector<Mat> splt(3, Mat());
	if (source.channels() == 1) {
		splt[0] = source;
		splt[1] = source;
		splt[2] = source;
		merge(splt, source);
	}
	Point temp_point;
	for(int i=0; i<noOfCordinates; i++)
	{	
		temp_point.x=cordinates[i].x + COG.x;
		temp_point.y=cordinates[i].y + COG.y;
		line(source, temp_point, temp_point,color,lineWidth);
	}
}

// draw contour at template image
void GeoMatch::DrawContours(Mat &source,CvScalar color,int lineWidth)
{
	vector<Mat> splt(3, Mat());
	if (source.channels() == 1) {
		splt[0] = source;
		splt[1] = source;
		splt[2] = source;
		merge(splt, source);
	}

	Point point;
	for(int i=0; i<noOfCordinates; i++)
	{
		point.x=cordinates[i].x;
		point.y=cordinates[i].y;
		//point.x=cordinates[i].x + centerOfGravity.x;
		//point.y=cordinates[i].y + centerOfGravity.y;
		line(source,point,point,color,lineWidth);
	}
}


bool geo_match(Mat temp1, Mat temp2, float & score, Mat & draw_image, Point & result, vector<double> roi,bool color_flag)
{
	score = -2;
	GeoMatch GM;				// object to implent geometric matching	
	GM.color_flag = color_flag;
	int lowThreshold = 10;		//deafult value
	int highThreashold = 50;	//deafult value
	double minScore = 0.6;		//deafult value
	double greediness = 0.4;		//deafult value

	//IplImage* templateImage = cvCloneImage(&(IplImage)temp2);
	//if (templateImage == NULL)
	//	return false;
	//IplImage* searchImage = cvCloneImage(&(IplImage)temp1);
	//if (searchImage == NULL)
	//	return false;

	//CvSize templateSize = cvSize(templateImage->width, templateImage->height);
	//IplImage* grayTemplateImg = cvCreateImage(templateSize, IPL_DEPTH_8U, 1);
	Mat searchImage = temp1.clone();
	Mat templateImage = temp2.clone();
	Mat grayTemplateImg;
	// Convert color image to gray image.
	if (templateImage.channels() == 3)
	{
		cvtColor(templateImage, grayTemplateImg, CV_BGR2GRAY);
	}
	else
	{
		grayTemplateImg = templateImage.clone();
	}
	GaussianBlur(grayTemplateImg, grayTemplateImg, cvSize(3,3), 0);
	
	GM.CreateGeoMatchModel(grayTemplateImg,  80, 20);
	draw_image = temp2.clone();
	GM.DrawContours(draw_image, CV_RGB(0, 255, 0), 1);
	//CvSize searchSize = cvSize(searchImage->width, searchImage->height);
	//IplImage* graySearchImg = cvCreateImage(searchSize, IPL_DEPTH_8U, 1);
	Mat graySearchImg;
	// Convert color image to gray image. 
	if (searchImage.channels() == 3)
	{
		cvtColor(searchImage, graySearchImg, CV_BGR2GRAY);
	}
	else
	{
		graySearchImg = searchImage.clone();
	}
	GaussianBlur(searchImage, searchImage, cvSize(5, 5), 0);
	Mat assist_score;

	matchTemplate(graySearchImg, grayTemplateImg, assist_score, CV_TM_CCOEFF_NORMED);
	int r = assist_score.rows;
	int c = assist_score.cols;
	assist_score.colRange(0, 0.1*c).setTo(-2);
	assist_score.colRange(0.9*c, c).setTo(-2);
	assist_score.rowRange(0.5*r, r).setTo(-2);
	double max_score = -2;
	for (int i = 0; i < assist_score.total(); ++i) {
		if (max_score < *(assist_score.ptr<float>(0) + i))
			max_score = *(assist_score.ptr<float>(0) + i);
	}
	//assist_score.setTo(1);
	// 抑制部分，固定区域
	if (roi.size() > 3) {
		GM.color_flag = false;
		roi[0] = roi[0] - draw_image.cols / 2;
		roi[2] = roi[2] - draw_image.cols / 2;

		roi[1] = roi[1] - draw_image.rows / 2;
		roi[3] = roi[3] - draw_image.rows / 2;
		for (int i = 0; i < 4; ++i) {
			roi[i] = roi[i] >= 0 ? roi[i] : 0;
		}
		assist_score.colRange(0, roi[0]).setTo(-2);
		assist_score.colRange(roi[2], assist_score.cols).setTo(-2);
		assist_score.rowRange(0, roi[1]).setTo(-2);
		assist_score.rowRange(roi[3], assist_score.rows).setTo(-2);
	}


	score = GM.FindGeoMatchModel(graySearchImg, minScore, greediness, result,assist_score);
	//result.x = 726;
	//result.y = 3;
	//score = 0.9;
	//float temp_score = -1;
	//for (int i=0;i<assist_score.rows;++i)
	//	for (int j = 0; j < assist_score.cols; ++j) {
	//		if (assist_score.at<float>(i, j) > temp_score) {
	//			temp_score = assist_score.at<float>(i, j);
	//			result.x = j;
	//			result.y = i;
	//		}
	//	}
	if (!color_flag) {
		double temp_score = 0;
		for(int i= -grayTemplateImg.rows / 4;i< grayTemplateImg.rows/4;++i)
			for (int j = -grayTemplateImg.cols / 4; j < grayTemplateImg.cols/4; ++j) {
				if (result.y + i < 0 || result.x + j < 0)
					continue;
				if (result.y + i > assist_score.rows || result.x + j > assist_score.cols)
					continue;
				temp_score += assist_score.at<float>(result.y+i, result.x+j);
			}
		temp_score /= grayTemplateImg.total()/4;
		score = 0.5*score+0.5*temp_score;
	}
	draw_image = temp1.clone();
	GM.DrawContours(draw_image, result, CV_RGB(0, 255, 0), 1);
	//score = GM.FindGeoMatchModel(255-graySearchImg, minScore, greediness, result, assist_score);
	////result.x = 726;
	////result.y = 3;
	////score = 0.9;

	//draw_image = temp1.clone();
	//GM.DrawContours(draw_image, result, CV_RGB(0, 255, 0), 1);

	//result.x = result.x - GM.centerOfGravity.x;
	//result.y = result.y - GM.centerOfGravity.y;


	return true;
}


