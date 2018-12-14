#include <stdio.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
using namespace std;
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"  
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"  
#include "opencv2/objdetect/objdetect.hpp"
using namespace cv;

enum LineSegmentDetectorModes {
	xy_LSD_REFINE_NONE = 0, //!< No refinement applied
	xy_LSD_REFINE_STD = 1, //!< Standard refinement is applied. E.g. breaking arches into smaller straighter line approximations.
	xy_LSD_REFINE_ADV = 2 , //!< Advanced refinement. Number of false alarms is calculated, lines are
						 //!< refined through increase of precision, decrement in size, etc.
	xy_INTER_LINEAR_EXACT = 5
};
/////////////////////////////////////////////////////////////////////////////////////////
// Default LSD parameters
// SIGMA_SCALE 0.6    - Sigma for Gaussian filter is computed as sigma = sigma_scale/scale.
// QUANT       2.0    - Bound to the quantization error on the gradient norm.
// ANG_TH      22.5   - Gradient angle tolerance in degrees.
// LOG_EPS     0.0    - Detection threshold: -log10(NFA) > log_eps
// DENSITY_TH  0.7    - Minimal density of region points in rectangle.
// N_BINS      1024   - Number of bins in pseudo-ordering of gradient modulus.

#define M_3_2_PI    (3 * CV_PI) / 2   // 3/2 pi
#define M_2__PI     (2 * CV_PI)         // 2 pi

#ifndef M_LN10
#define M_LN10      2.30258509299404568402
#endif

#define NOTDEF      double(-1024.0) // Label for pixels with undefined gradient.

#define NOTUSED     0   // Label for pixels not used in yet.
#define USED        1   // Label for pixels already used in detection.

#define RELATIVE_ERROR_FACTOR 100.0

const double DEG_TO_RADS = CV_PI / 180;

#define log_gamma(x) ((x)>15.0?log_gamma_windschitl(x):log_gamma_lanczos(x))

struct edge
{
	cv::Point p;
	bool taken;
};

/////////////////////////////////////////////////////////////////////////////////////////

inline double distSq(const double x1, const double y1,
	const double x2, const double y2)
{
	return (x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1);
}

inline double dist(const double x1, const double y1,
	const double x2, const double y2)
{
	return sqrt(distSq(x1, y1, x2, y2));
}

// Signed angle difference
inline double angle_diff_signed(const double& a, const double& b)
{
	double diff = a - b;
	while (diff <= -CV_PI) diff += M_2__PI;
	while (diff > CV_PI) diff -= M_2__PI;
	return diff;
}

// Absolute value angle difference
inline double angle_diff(const double& a, const double& b)
{
	return std::fabs(angle_diff_signed(a, b));
}

// Compare doubles by relative error.
inline bool double_equal(const double& a, const double& b)
{
	// trivial case
	if (a == b) return true;

	double abs_diff = fabs(a - b);
	double aa = fabs(a);
	double bb = fabs(b);
	double abs_max = (aa > bb) ? aa : bb;

	if (abs_max < DBL_MIN) abs_max = DBL_MIN;

	return (abs_diff / abs_max) <= (RELATIVE_ERROR_FACTOR * DBL_EPSILON);
}

inline bool AsmallerB_XoverY(const edge& a, const edge& b)
{
	if (a.p.x == b.p.x) return a.p.y < b.p.y;
	else return a.p.x < b.p.x;
}

/**
 *   Computes the natural logarithm of the absolute value of
 *   the gamma function of x using Windschitl method.
 *   See http://www.rskey.org/gamma.htm
 */
inline double log_gamma_windschitl(const double& x)
{
	return 0.918938533204673 + (x - 0.5)*log(x) - x
		+ 0.5*x*log(x*sinh(1 / x) + 1 / (810.0*pow(x, 6.0)));
}

/**
 *   Computes the natural logarithm of the absolute value of
 *   the gamma function of x using the Lanczos approximation.
 *   See http://www.rskey.org/gamma.htm
 */
inline double log_gamma_lanczos(const double& x)
{
	static double q[7] = { 75122.6331530, 80916.6278952, 36308.2951477,
						 8687.24529705, 1168.92649479, 83.8676043424,
						 2.50662827511 };
	double a = (x + 0.5) * log(x + 5.5) - (x + 5.5);
	double b = 0;
	for (int n = 0; n < 7; ++n)
	{
		a -= log(x + double(n));
		b += q[n] * pow(x, double(n));
	}
	return a + log(b);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
class xy_LineSegmentDetector
{
public:

/**
 * Create a xy_LineSegmentDetector object. Specifying scale, number of subdivisions for the image, should the lines be refined and other constants as follows:
 *
 * @param _refine       How should the lines found be refined?
 *                      LSD_REFINE_NONE - No refinement applied.
 *                      LSD_REFINE_STD  - Standard refinement is applied. E.g. breaking arches into smaller line approximations.
 *                      LSD_REFINE_ADV  - Advanced refinement. Number of false alarms is calculated,
 *                                    lines are refined through increase of precision, decrement in size, etc.
 * @param _scale        The scale of the image that will be used to find the lines. Range (0..1].
 * @param _sigma_scale  Sigma for Gaussian filter is computed as sigma = _sigma_scale/_scale.
 * @param _quant        Bound to the quantization error on the gradient norm.
 * @param _ang_th       Gradient angle tolerance in degrees.
 * @param _log_eps      Detection threshold: -log10(NFA) > _log_eps
 * @param _density_th   Minimal density of aligned region points in rectangle.
 * @param _n_bins       Number of bins in pseudo-ordering of gradient modulus.
 */
    xy_LineSegmentDetector(int _refine = xy_LSD_REFINE_NONE, double _scale = 0.8,
        double _sigma_scale = 0.6, double _quant = 2.0, double _ang_th = 22.5,
        double _log_eps = 0, double _density_th = 0.7, int _n_bins = 1024);

/**
 * Detect lines in the input image.
 *
 * @param _image    A grayscale(CV_8UC1) input image.
 *                  If only a roi needs to be selected, use
 *                  lsd_ptr->detect(image(roi), ..., lines);
 *                  lines += Scalar(roi.x, roi.y, roi.x, roi.y);
 * @param _lines    Return: A vector of Vec4i or Vec4f elements specifying the beginning and ending point of a line.
 *                          Where Vec4i/Vec4f is (x1, y1, x2, y2), point 1 is the start, point 2 - end.
 *                          Returned lines are strictly oriented depending on the gradient.
 * @param width     Return: Vector of widths of the regions, where the lines are found. E.g. Width of line.
 * @param prec      Return: Vector of precisions with which the lines are found.
 * @param nfa       Return: Vector containing number of false alarms in the line region, with precision of 10%.
 *                          The bigger the value, logarithmically better the detection.
 *                              * -1 corresponds to 10 mean false alarms
 *                              * 0 corresponds to 1 mean false alarm
 *                              * 1 corresponds to 0.1 mean false alarms
 *                          This vector will be calculated _only_ when the objects type is REFINE_ADV
 */
    void detect(InputArray _image, OutputArray _lines,
                OutputArray width = noArray(), OutputArray prec = noArray(),
                OutputArray nfa = noArray());

/**
 * Draw both vectors on the image canvas. Uses blue for lines 1 and red for lines 2.
 *
 * @param size      The size of the image, where lines1 and lines2 were found.
 * @param lines1    The first lines that need to be drawn. Color - Blue.
 * @param lines2    The second lines that need to be drawn. Color - Red.
 * @param image     An optional image, where lines will be drawn.
 *                  Should have the size of the image, where the lines were found
 * @return          The number of mismatching pixels between lines1 and lines2.
 */
    int compareSegments(const Size& size, InputArray lines1, InputArray lines2, InputOutputArray _image = noArray());

private:
    Mat image;
    Mat scaled_image;
    Mat_<double> angles;     // in rads
    Mat_<double> modgrad;
    Mat_<uchar> used;

    int img_width;
    int img_height;
    double LOG_NT;

    bool w_needed;
    bool p_needed;
    bool n_needed;

    const double SCALE;
    const int doRefine;
    const double SIGMA_SCALE;
    const double QUANT;
    const double ANG_TH;
    const double LOG_EPS;
    const double DENSITY_TH;
    const int N_BINS;

    struct RegionPoint {
        int x;
        int y;
        uchar* used;
        double angle;
        double modgrad;
    };


    struct coorlist
    {
        Point2i p;
        struct coorlist* next;
    };

    std::vector<coorlist> list;

    struct rect
    {
        double x1, y1, x2, y2;    // first and second point of the line segment
        double width;             // rectangle width
        double x, y;              // center of the rectangle
        double theta;             // angle
        double dx,dy;             // (dx,dy) is vector oriented as the line segment
        double prec;              // tolerance angle
        double p;                 // probability of a point with angle within 'prec'
    };

    xy_LineSegmentDetector& operator= (const xy_LineSegmentDetector&); // to quiet MSVC

/**
 * Detect lines in the whole input image.
 *
 * @param lines         Return: A vector of Vec4f elements specifying the beginning and ending point of a line.
 *                              Where Vec4f is (x1, y1, x2, y2), point 1 is the start, point 2 - end.
 *                              Returned lines are strictly oriented depending on the gradient.
 * @param widths        Return: Vector of widths of the regions, where the lines are found. E.g. Width of line.
 * @param precisions    Return: Vector of precisions with which the lines are found.
 * @param nfas          Return: Vector containing number of false alarms in the line region, with precision of 10%.
 *                              The bigger the value, logarithmically better the detection.
 *                                  * -1 corresponds to 10 mean false alarms
 *                                  * 0 corresponds to 1 mean false alarm
 *                                  * 1 corresponds to 0.1 mean false alarms
 */
    void flsd(std::vector<Vec4f>& lines,
              std::vector<double>& widths, std::vector<double>& precisions,
              std::vector<double>& nfas);

/**
 * Finds the angles and the gradients of the image. Generates a list of pseudo ordered points.
 *
 * @param threshold The minimum value of the angle that is considered defined, otherwise NOTDEF
 * @param n_bins    The number of bins with which gradients are ordered by, using bucket sort.
 * @param list      Return: Vector of coordinate points that are pseudo ordered by magnitude.
 *                  Pixels would be ordered by norm value, up to a precision given by max_grad/n_bins.
 */
    void ll_angle(const double& threshold, const unsigned int& n_bins);

/**
 * Grow a region starting from point s with a defined precision,
 * returning the containing points size and the angle of the gradients.
 *
 * @param s         Starting point for the region.
 * @param reg       Return: Vector of points, that are part of the region
 * @param reg_angle Return: The mean angle of the region.
 * @param prec      The precision by which each region angle should be aligned to the mean.
 */
    void region_grow(const Point2i& s, std::vector<RegionPoint>& reg,
                     double& reg_angle, const double& prec);

/**
 * Finds the bounding rotated rectangle of a region.
 *
 * @param reg       The region of points, from which the rectangle to be constructed from.
 * @param reg_angle The mean angle of the region.
 * @param prec      The precision by which points were found.
 * @param p         Probability of a point with angle within 'prec'.
 * @param rec       Return: The generated rectangle.
 */
    void region2rect(const std::vector<RegionPoint>& reg, const double reg_angle,
                     const double prec, const double p, rect& rec) const;

/**
 * Compute region's angle as the principal inertia axis of the region.
 * @return          Regions angle.
 */
    double get_theta(const std::vector<RegionPoint>& reg, const double& x,
                     const double& y, const double& reg_angle, const double& prec) const;

/**
 * An estimation of the angle tolerance is performed by the standard deviation of the angle at points
 * near the region's starting point. Then, a new region is grown starting from the same point, but using the
 * estimated angle tolerance. If this fails to produce a rectangle with the right density of region points,
 * 'reduce_region_radius' is called to try to satisfy this condition.
 */
    bool refine(std::vector<RegionPoint>& reg, double reg_angle,
                const double prec, double p, rect& rec, const double& density_th);

/**
 * Reduce the region size, by elimination the points far from the starting point, until that leads to
 * rectangle with the right density of region points or to discard the region if too small.
 */
    bool reduce_region_radius(std::vector<RegionPoint>& reg, double reg_angle,
                const double prec, double p, rect& rec, double density, const double& density_th);

/**
 * Try some rectangles variations to improve NFA value. Only if the rectangle is not meaningful (i.e., log_nfa <= log_eps).
 * @return      The new NFA value.
 */
    double rect_improve(rect& rec) const;

/**
 * Calculates the number of correctly aligned points within the rectangle.
 * @return      The new NFA value.
 */
    double rect_nfa(const rect& rec) const;

/**
 * Computes the NFA values based on the total number of points, points that agree.
 * n, k, p are the binomial parameters.
 * @return      The new NFA value.
 */
    double nfa(const int& n, const int& k, const double& p) const;

/**
 * Is the point at place 'address' aligned to angle theta, up to precision 'prec'?
 * @return      Whether the point is aligned.
 */
    bool isAligned(int x, int y, const double& theta, const double& prec) const;
};