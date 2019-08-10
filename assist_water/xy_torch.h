#pragma once
#include <string>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"  
#if defined(_WIN64)
	#include <torch/torch.h>
	#include <torch/script.h>


class xy_torch {
public:
	std::shared_ptr<torch::jit::script::Module> module;
	bool load_model(std::string model_name) {
		std::ifstream fin(model_name);
		if (!fin)
			return false;
		fin.close();
		module = torch::jit::load(model_name);
		if (module != nullptr)
			return true;
		else
			return false;
	};
	cv::Mat process_image(cv::Mat image) {
		if (module == nullptr)
			return cv::Mat{};
		torch::NoGradGuard no_grad;

		cv::cvtColor(image, image, CV_BGR2RGB);
		image.convertTo(image, CV_32FC3);

		cv::Mat input_Mat = image.clone();
		cv::Mat channelsConcatenated;
		cv::Mat bgr[3];
		cv::split(input_Mat, bgr);
		float a[3] = { 0.485, 0.456, 0.406 };
		float b[3] = { 0.229, 0.224, 0.225 };
		for (int i = 0; i < 3; ++i) {
			bgr[i] = (bgr[i] / 255.0 - a[i]) / b[i];
		}
		vconcat(bgr[0], bgr[1], channelsConcatenated);
		vconcat(channelsConcatenated, bgr[2], channelsConcatenated);

		std::vector<int64_t> dims{ 1, static_cast<int64_t>(input_Mat.channels()),
								  static_cast<int64_t>(input_Mat.rows),
								  static_cast<int64_t>(input_Mat.cols) };
		at::TensorOptions options(at::kFloat);
		at::Tensor input = torch::from_blob(channelsConcatenated.data, at::IntList(dims), options).set_requires_grad(false).cpu();
		std::vector<torch::jit::IValue> inputs;
		inputs.emplace_back(input);
		at::Tensor output = module->forward(inputs).toTensor();
		cv::Mat output_Mat(cv::Size(input_Mat.cols, 2 * input_Mat.rows), CV_32FC1, output.data<float>());
		cv::Mat temp_result = cv::Mat::zeros(input_Mat.size(), CV_8UC1) + 50;
		for (int i = 0; i < input_Mat.rows; ++i)
			for (int j = 0; j < input_Mat.cols; ++j)
				if (output_Mat.at<float>(i, j) < output_Mat.at<float>(i + input_Mat.rows, j)) {
					temp_result.at<uchar>(i, j) = 255;
				}
		return temp_result;
	};
};
#endif
