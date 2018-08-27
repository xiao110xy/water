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


int GeoMatch::CreateGeoMatchModel(Mat templateArr,double maxContrast,double minContrast)
{

	//CvMat *gx = 0;		//Matrix to store X derivative
	//CvMat *gy = 0;		//Matrix to store Y derivative
	//CvMat *nmsEdges = 0;		//Matrix to store temp restult
	//CvSize Ssize;
	// Convert IplImage to Matrix for integer operations
	Mat src = templateArr.clone();
	src.convertTo(src, CV_8UC1);
	//if(CV_MAT_TYPE( src->type ) != CV_8UC1)
	//{	
	//	return 0;
	//}
	
	// set width and height

	modelHeight = src.rows;		//Save Template height
	modelWidth =src.cols;			//Save Template width

	noOfCordinates=0;											//initialize	
	cordinates =  new CvPoint[ modelWidth *modelHeight];		//Allocate memory for coorinates of selected points in template image
	
	edgeMagnitude = new double[ modelWidth *modelHeight];		//Allocate memory for edge magnitude for selected points
	edgeDerivativeX = new double[modelWidth *modelHeight];			//Allocate memory for edge X derivative for selected points
	edgeDerivativeY = new double[modelWidth *modelHeight];			////Allocate memory for edge Y derivative for selected points


	// Calculate gradient of Template
	Mat gx = Mat::zeros(src.size(), CV_16SC1);
	Mat gy = Mat::zeros(src.size(), CV_16SC1);

	Sobel( src, gx, CV_16S,1,0, 3);		//gradient in X direction			
	Sobel( src, gy, CV_16S, 0, 1, 3 );	//gradient in Y direction
	
	Mat nmsEdges = Mat::zeros(src.size(), CV_32F);		//create Matrix to store Final nmsEdges
	const short* _sdx; 
	const short* _sdy; 
	double fdx,fdy;	
    double MagG, DirG;
	double MaxGradient=-99999.99;
	double direction;
	int *orients = new int[ src.total()];
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
				
			orients[count] = (int)direction;
			count++;
		}
	}
	
	count=0; // init count
	// non maximum suppression
	double leftPixel,rightPixel;
	
	for (i = 1; i < src.rows - 1; i++)
	{
		for (j = 1; j < src.cols - 1; j++)
		{
				switch ( orients[count] )
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
					(nmsEdges.ptr<float>(0) + nmsEdges.step*i)[j]=0;
                else
                    (nmsEdges.ptr<float>(0) + nmsEdges.step*i)[j]=(uchar)(magMat.at<double>(i, j) /MaxGradient*255);
			
				count++;
			}
		}
	

	int RSum=0,CSum=0;
	int curX,curY;
	int flag=1;

	//Hysterisis threshold
	for (i = 1; i < src.rows - 1; i++)
	{
		for (j = 1; j < src.cols - 1; j++)
		{
			fdx = gx.at<short>(i, j);
			fdy = gy.at<short>(i, j);        // read x, y derivatives
				
			MagG = sqrt(fdx*fdx + fdy*fdy); //Magnitude = Sqrt(gx^2 +gy^2)
			DirG =cvFastArctan((float)fdy,(float)fdx);	 //Direction = tan(y/x)
		
			////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]= MagG;
			flag=1;
			if(((double)((nmsEdges.ptr<float>(0) + nmsEdges.step*i))[j]) < maxContrast)
			{
				if(((double)((nmsEdges.ptr<float>(0) + nmsEdges.step*i))[j])< minContrast)
				{
					
					(nmsEdges.ptr<float>(0) + nmsEdges.step*i)[j]=0;
					flag=0; // remove from edge
					////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
				}
				else
				{   // if any of 8 neighboring pixel is not greater than max contraxt remove from edge
					if( (((double)((nmsEdges.ptr<float>(0) + nmsEdges.step*(i-1)))[j-1]) < maxContrast)	&&
						(((double)((nmsEdges.ptr<float>(0) + nmsEdges.step*(i-1)))[j]) < maxContrast)		&&
						(((double)((nmsEdges.ptr<float>(0) + nmsEdges.step*(i-1)))[j+1]) < maxContrast)	&&
						(((double)((nmsEdges.ptr<float>(0) + nmsEdges.step*i))[j-1]) < maxContrast)		&&
						(((double)((nmsEdges.ptr<float>(0) + nmsEdges.step*i))[j+1]) < maxContrast)		&&
						(((double)((nmsEdges.ptr<float>(0) + nmsEdges.step*(i+1)))[j-1]) < maxContrast)	&&
						(((double)((nmsEdges.ptr<float>(0) + nmsEdges.step*(i+1)))[j]) < maxContrast)		&&
						(((double)((nmsEdges.ptr<float>(0) + nmsEdges.step*(i+1)))[j+1]) < maxContrast)	)
					{
						(nmsEdges.ptr<float>(0) + nmsEdges.step*i)[j]=0;
						flag=0;
						////((uchar*)(imgGDir->imageData + imgGDir->widthStep*i))[j]=0;
					}
				}
				
			}
			
			// save selected edge information
			curX=i;	curY=j;
			if(flag!=0)
			{
				if(fdx!=0 || fdy!=0)
				{		
					RSum=RSum+curX;	CSum=CSum+curY; // Row sum and column sum for center of gravity
					
					cordinates[noOfCordinates].x = curX;
					cordinates[noOfCordinates].y = curY;
					edgeDerivativeX[noOfCordinates] = fdx;
					edgeDerivativeY[noOfCordinates] = fdy;
					
					//handle divide by zero
					if(MagG!=0)
						edgeMagnitude[noOfCordinates] = 1/MagG;  // gradient magnitude 
					else
						edgeMagnitude[noOfCordinates] = 0;
															
					noOfCordinates++;
				}
			}
		}
	}

	centerOfGravity.x = RSum /noOfCordinates; // center of gravity
	centerOfGravity.y = CSum/noOfCordinates ;	// center of gravity
		
	// change coordinates to reflect center of gravity
	for(int m=0;m<noOfCordinates ;m++)
	{
		int temp;

		temp=cordinates[m].x;
		cordinates[m].x=temp-centerOfGravity.x;
		temp=cordinates[m].y;
		cordinates[m].y =temp-centerOfGravity.y;
	}
	
	////cvSaveImage("Edges.bmp",imgGDir);
	
	// free alocated memories
	delete[] orients;
	////cvReleaseImage(&imgGDir);


	
	modelDefined=true;
	return 1;
}


double GeoMatch::FindGeoMatchModel(Mat src,double minScore,double greediness, Point &resultPoint)
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

	for (i = 0; i < src.rows; i++)
	{
			for( j = 0; j < src.cols; j++ )
             { 
				 partialSum = 0; // initilize partialSum measure
				 for(m=0;m<noOfCordinates;m++)
				 {
					 curX	= i + cordinates[m].x ;	// template X coordinate
					 curY	= j + cordinates[m].y ; // template Y coordinate
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
					if( partialScore < (MIN((minScore -1) + normGreediness*sumOfCoords,normMinScore*  sumOfCoords)))
						break;
									
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
// destructor
GeoMatch::~GeoMatch(void)
{
	delete[] cordinates ;
	delete[] edgeMagnitude;
	delete[] edgeDerivativeX;
	delete[] edgeDerivativeY;
}

//allocate memory for doubel matrix
void GeoMatch::CreateDoubleMatrix(double **&matrix,CvSize size)
{
	matrix = new double*[size.height];
	for(int iInd = 0; iInd < size.height; iInd++)
		matrix[iInd] = new double[size.width];
}
// release memory
void GeoMatch::ReleaseDoubleMatrix(double **&matrix,int size)
{
	for(int iInd = 0; iInd < size; iInd++) 
        delete[] matrix[iInd]; 
}


// draw contours around result image
void GeoMatch::DrawContours(Mat &source, Point  COG,CvScalar color,int lineWidth)
{
	Point temp_point;
	for(int i=0; i<noOfCordinates; i++)
	{	
		temp_point.x=cordinates[i].y + COG.x;
		temp_point.y=cordinates[i].x + COG.y;
		line(source, temp_point, temp_point,color,lineWidth);
	}
}

// draw contour at template image
void GeoMatch::DrawContours(Mat &source,CvScalar color,int lineWidth)
{
	Point point;
	for(int i=0; i<noOfCordinates; i++)
	{
		point.y=cordinates[i].x + centerOfGravity.x;
		point.x=cordinates[i].y + centerOfGravity.y;
		line(source,point,point,color,lineWidth);
	}
}

void GetImageEdge(Mat im, int &bestLowThresh, int& bestHighThresh)
{
	int nWidth = im.cols;
	int nHeight = im.rows;

	int nGrayHistogram[256];
	double por_nGrayHistogram[256];
	double var = 0;
	double maxVar = 0;

	const int GrayLevel = 256;
	double allEpt = 0;
	double Ept[3] = { 0,0,0 };
	double por[3] = { 0,0,0 };
	int lowThresh = 0;
	int highThresh = 0;



	for (int i = 0; i < GrayLevel; i++)
	{
		nGrayHistogram[i] = 0;
	}
	int nPixel;
	for (int i = 0; i < im.total(); ++i) {
		nPixel = *(im.ptr<uchar>(0) + i);
		nGrayHistogram[nPixel]++;
	}

	int nSum = 0;
	for (int i = 0; i < GrayLevel; i++)
	{
		nSum += nGrayHistogram[i];
	}

	for (int i = 0; i < GrayLevel; i++)
	{
		por_nGrayHistogram[i] = 1.0*nGrayHistogram[i] / nSum;
	}

	for (int i = 0; i < GrayLevel; i++)
	{
		allEpt = i * por_nGrayHistogram[i];
	}

	for (lowThresh = 0; lowThresh < (GrayLevel - 1); lowThresh++)
		for (highThresh = (lowThresh + 1); highThresh < GrayLevel; highThresh++)
		{

			var = 0;
			Ept[0] = Ept[1] = Ept[2] = 0;
			por[0] = por[1] = por[2] = 0;

			for (int i = 0; i < lowThresh; i++)
			{
				por[0] += por_nGrayHistogram[i];
				Ept[0] += i * por_nGrayHistogram[i];
			}
			Ept[0] /= por[0];

			for (int i = lowThresh; i < highThresh; i++)
			{
				por[1] += por_nGrayHistogram[i];
				Ept[1] += i * por_nGrayHistogram[i];
			}
			Ept[1] /= por[1];

			for (int i = highThresh; i < GrayLevel; i++)
			{
				por[2] += por_nGrayHistogram[i];
				Ept[2] += i * por_nGrayHistogram[i];
			}
			Ept[2] /= por[2];

			for (int i = 0; i < 3; i++)
			{
				var += ((Ept[i] - allEpt)*(Ept[i] - allEpt)*por[i]);
			}

			if (var > maxVar)
			{
				maxVar = var;
				bestLowThresh = lowThresh;
				bestHighThresh = highThresh;
			}
		}

}

bool geo_match(Mat temp1, Mat temp2, float & score, Mat & draw_image, Point & result)
{

	GeoMatch GM;				// object to implent geometric matching	
	int lowThreshold = 10;		//deafult value
	int highThreashold = 100;	//deafult value
	GetImageEdge(temp1, lowThreshold, highThreashold);
	double minScore = 0.3;		//deafult value
	double greediness = 0.8;		//deafult value
	double total_time = 0;

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

	if (!GM.CreateGeoMatchModel(grayTemplateImg, lowThreshold, highThreashold))
	{
		return false;
	}
	//GM.DrawContours(temp2, CV_RGB(255, 0, 0), 1);
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

	clock_t start_time1 = clock();
	score = GM.FindGeoMatchModel(graySearchImg, minScore, greediness, result);
	clock_t finish_time1 = clock();
	total_time = (double)(finish_time1 - start_time1) / CLOCKS_PER_SEC;

	draw_image = temp1.clone();
	GM.DrawContours(draw_image, result, CV_RGB(0, 255, 0), 1);


	result.x = result.x - GM.centerOfGravity.y;
	result.y = result.y - GM.centerOfGravity.x;



	return true;
}


