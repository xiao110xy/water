#pragma once


#include <iostream>
#include <algorithm>
#include <vector> 
#include <string>
#include <list>
#include <io.h>

class water_line
{
public:
	water_line();
	~water_line();
};
Mat draw_line(Mat data,vector<Matx<float, 6, 1>> lines);
Mat draw_line(Mat data, vector<Matx<float, 12, 1>> lines);

Mat draw_point(Mat data, vector<Point2i> points, Scalar rgb);
Mat draw_point(Mat data, vector<Mat> points);
Mat draw_point(Mat data, vector<vector<float>> points);

void get_line(Mat image, float det_v, float det_h, vector<Matx<float, 6, 1>> &lines1, vector<Matx<float, 6, 1>> &lines2);
void get_line(Mat image,vector<Matx<float, 6, 1>> &lines1, vector<Matx<float, 6, 1>> &lines2);

vector<Matx<float, 12, 1>> get_parallel_lines(Mat image, vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2);
vector<Matx<float, 6, 1>> merge_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines, vector<Matx<float, 6, 1>> &data, float angle_t, float d_v_t, float d_p_t);
vector<Matx<float, 7, 1>> project_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines);
vector<Matx<float, 6, 1>> judge_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines, vector<Matx<float, 6, 1>> lines2);
vector<Matx<float, 6, 1>> intersect_line(vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2);
bool pnpoly(int nvert, float *vertx, float *verty, float testx, float testy);

float segement_area(Mat I, vector<Matx<float, 12, 1>> parallel_lines, vector<vector<Mat>> &model);
Mat sub_water_area(Mat I,Mat &line1,Mat &line2);
vector<Matx<float, 6, 1>> select_v_lines(Mat I, vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2);
Mat get_e_boundary(Mat I, vector<Matx<float, 6, 1>> lines);

vector<vector<float>> compute_e_point(Mat I, Mat location);
vector<Point2i> localmax_point(Mat score_image, float d_t,float scale);
vector<Mat> get_e_proposal_points(Mat im, vector<vector<Point2i>> points);
vector<vector<float>> better_e_points(Mat im, vector<Mat> points);
vector<vector<float>> get_e_location(Mat im, vector<float> score1, vector<float> score2, float distance_t, float score_t, int x);
float corr_data(Mat im, vector<Mat> data);
vector<int> class_score(Mat corr_matrix, float score_t);

vector<vector<float>> number_recgnition(Mat data, vector<vector<Mat>> &model);
vector<string> getFiles(string folder, string firstname, string lastname);

vector<int> sub2ind(Mat m, vector<Point2f> point);
vector<Point2i> ind2sub(Mat m, vector<int> ind);