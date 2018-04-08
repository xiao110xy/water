#pragma once


#include <iostream>
#include <algorithm>
#include <vector> 
#include <string>
#include <list>
#include <io.h>
struct water_result {
	Mat data;
	vector<vector<float>> points;
	vector<vector<float>> number;
	int water_line;
	float water_number;
};
class water_line
{
public:
	water_line();
	~water_line();
};
Mat draw_line(Mat data,vector<Matx<float, 6, 1>> lines);
Mat draw_line(Mat data,vector<Matx<float, 6, 1>> lines,Scalar rgb);
Mat draw_line(Mat data, vector<Matx<float, 12, 1>> lines);
Mat draw_line(Mat data, vector<Matx<float, 12, 1>> lines,vector<Scalar> rgb);

Mat draw_point(Mat data, vector<Point3f> points, Scalar rgb);
Mat draw_point(Mat data, vector<Point2i> points, Scalar rgb);
Mat draw_point(Mat data, vector<Mat> points);
Mat draw_point(Mat data, vector<vector<float>> points);

void get_line(Mat image, float det_v, float det_h, vector<Matx<float, 6, 1>> &lines1, vector<Matx<float, 6, 1>> &lines2);
void get_line(Mat image,vector<Matx<float, 6, 1>> &lines1, vector<Matx<float, 6, 1>> &lines2);

vector<Matx<float, 12, 1>> get_parallel_lines(Mat image, vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2);
vector<Matx<float, 6, 1>> merge_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines, vector<Matx<float, 6, 1>> &data, float angle_t, float d_v_t, float d_p_t);
Matx<float, 6, 1> sub_merge_line(vector<Matx<float, 6, 1>> lines);
// Bresenham's line algorithm
vector<Point> get_line_point(Point2f point1, Point2f point2); 
vector<Matx<float, 7, 1>> project_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines);
vector<Matx<float, 6, 1>> judge_line(Matx<float, 6, 1> line, vector<Matx<float, 6, 1>> lines, vector<Matx<float, 6, 1>> lines2);
vector<Point2f> judge_parrallel_line(Matx<float, 6, 1> line1, Matx<float, 6, 1> line2);
vector<Matx<float, 6, 1>> intersect_line(vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2);
bool pnpoly(int nvert, float *vertx, float *verty, float testx, float testy);

vector<water_result> segement_area(Mat I, vector<vector<Mat>> &model);
vector<Matx<float, 12, 1>>  extend_line(Mat I, vector<Matx<float, 12, 1>> parallel_lines);
vector<Matx<float, 12, 1>>  subtract_iou(Mat I,vector<Matx<float, 12, 1>> parallel_lines);
Mat sub_water_area(Mat I,Mat &line1,Mat &line2);
vector<Matx<float, 6, 1>> select_h_lines(Mat I, vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2);
Mat get_e_boundary(Mat I, vector<Matx<float, 6, 1>> lines);

vector<vector<float>> compute_e_point(Mat I, Mat location);
vector<Point2i> localmax_point(Mat score_image, float d_t,float scale);
vector<Point3f> localmax_point_score(Mat score_image, float d_t, float scale);
vector<Point3f> localmax_point_score(Mat score_image,int x1,int x2,float d_t, float scale);
vector<Mat> get_e_proposal_points(Mat im, vector<vector<Point2i>> points);
vector<vector<float>> better_e_points(Mat im, vector<Mat> points);
vector<vector<float>> get_e_location(Mat im, vector<float> score1, vector<float> score2, float distance_t, float score_t, int x);
float corr_data(Mat im, vector<Mat> data);
vector<int> class_score(Mat corr_matrix,int n,float value);

vector<vector<float>> number_area_recognition(Mat data, vector<vector<float>> points,vector<vector<Mat>> &model);
vector<float> number_recognition(Mat data,vector<vector<Mat>> model);
vector<vector<float>> better_number_rec(vector<vector<float>> number,vector<vector<float>> points);
vector<string> getFiles(string folder, string firstname, string lastname);

float get_water_line(Mat data, vector<vector<float>> points);
vector<vector<float>> select_e_area_by_line(Mat im, vector<Matx<float, 6, 1>> lines1, vector<Matx<float, 6, 1>> lines2,float distance);
vector<vector<float>> cluster_v_line(vector<Matx<float, 6, 1>> &lines, vector<vector<Matx<float, 6, 1>>> &result, float distance);
vector<vector<float>> claaify_h_lines(Mat im,vector<Matx<float, 6, 1>> lines, vector<Point3f> points, vector<Matx<float, 6, 1>> &lines1, vector<Matx<float, 6, 1>> &lines2, vector<Mat> & data, vector<Mat> &scores,bool left_or_right);
vector<float> cluster_value(vector<float> data, vector<float> &index);
Mat get_area_by_lines(Mat im,vector<float> lines1,vector<float> lines2);

vector<int> sub2ind(Mat m, vector<Point2f> point);
vector<Point2i> ind2sub(Mat m, vector<int> ind);

void svaefile(string image_name, vector<water_result> water);



