//***********************************************************************
// Project		    : GeoMatch
// Author           : Shiju P K
// Email			: shijupk@gmail.com
// Created          : 10-01-2010
//
// File Name		: GeoMatch.h
// Last Modified By : Shiju P K
// Last Modified On : 13-07-2010
// Description      : class to implement edge based template matching
//
// Copyright        : (c) . All rights reserved.
//***********************************************************************

#pragma once

#include <math.h>
#include <time.h>
class GeoMatch
{
public:
	int				noOfCordinates;		//Number of elements in coordinate array
	CvPoint			*cordinates;		//Coordinates array to store model points	
	int				modelHeight;		//Template height
	int				modelWidth;			//Template width
	double			*edgeMagnitude;		//gradient magnitude
	double			*edgeDerivativeX;	//gradient in X direction
	double			*edgeDerivativeY;	//radient in Y direction	
	CvPoint			centerOfGravity;	//Center of gravity of template 
	
	bool			modelDefined;
	
	void CreateDoubleMatrix(double **&matrix,CvSize size);
	void ReleaseDoubleMatrix(double **&matrix,int size);
public:
	GeoMatch(void);
	~GeoMatch(void);

	int CreateGeoMatchModel(Mat templateArr,double,double);
	double FindGeoMatchModel(Mat srcarr,double minScore,double greediness, Point &resultPoint);
	void DrawContours(Mat &source, Point COG,CvScalar,int);
	void DrawContours(Mat & source,CvScalar,int);
};
void GetImageEdge(Mat im, int &bestLowThresh, int& bestHighThresh);
bool geo_match(Mat temp1, Mat temp2, float &score, Mat &draw_image, Point &result);