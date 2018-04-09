#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/core/core.hpp"
#include <string>
using namespace std;
using namespace cv;

#include "slic.h"

int main()
{
	// Load an image
	Mat3b img = imread("1.jpg");

	// Set the maximum number of superpixels
	UINT n_of_superpixels = 200;
	SLIC slic;

	// Compute the superpixels
	slic.GenerateSuperpixels(img, n_of_superpixels);

	// Visualize superpixels
	//Mat3b res = slic.GetImgWithContours(Scalar(0,0,255));

	// Get the labels
	Mat1i labelImg(img.rows, img.cols, slic.GetLabel());

	// Get the actual number of labels
	// may be less that n_of_superpixels
	double max_dlabel;
	minMaxLoc(labelImg, NULL, &max_dlabel);
	int max_label = int(max_dlabel);

	// Iterate over each label
		Mat3b superpixel_in_img;
	for (int label = 0; label <= max_label; ++label)
	{
		// Mask for each label
		Mat1b superpixel_mask = labelImg == label;

		// Superpixel in original image
		img.copyTo(superpixel_in_img, superpixel_mask);

		// Now you have the binary mask of each superpixel: superpixel_mask
		// and the superpixel in the original image: superpixel_in_img
	}

	return 0;
}