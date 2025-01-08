#pragma once
#include "Utils/Logger/Logger.h"
#include <iostream>
#include <opencv2/opencv.hpp>


cv::Mat AnalyzeDepth(const std::string& leftPath, const std::string& rightPath,
	int numDisparities = 64, int blockSize = 7);

cv::Mat AnalyzeDepth(cv::Mat& leftImage, cv::Mat& rightImage,
	int numDisparities = 64, int blockSize = 7);