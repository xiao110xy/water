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
	int				modelHeight;		//Template height
	int				modelWidth;			//Template width
	vector<Point>	cordinates;		//Coordinates array to store model points	
	vector<double>	edgeMagnitude;		//gradient magnitude
	vector<double>	edgeDerivativeX;	//gradient in X direction
	vector<double>	edgeDerivativeY;	//radient in Y direction	
	Point			centerOfGravity;	//Center of gravity of template 
	

	bool			modelDefined;

public:
	GeoMatch(void);
	~GeoMatch(void);



	int CreateGeoMatchModel(Mat &src,double,double);
	void MatchModelBetter(Mat &src);
	double FindGeoMatchModel(Mat srcarr,double minScore,double greediness, Point &resultPoint,Mat assist_score);
	void DrawContours(Mat &source, Point COG,CvScalar,int);
	void DrawContours(Mat & source,CvScalar,int);
	bool color_flag = true;
};
bool geo_match(Mat temp1, Mat temp2,float &score, Mat &draw_image, Point &result,vector<double> roi = vector<double>{ }, bool color_flag = true);