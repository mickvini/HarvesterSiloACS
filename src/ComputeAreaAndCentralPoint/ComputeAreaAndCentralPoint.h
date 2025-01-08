#pragma once
#include <math.h>
#include <opencv2/opencv.hpp>
#include <vector>

const double MIN_CONTOUR_AREA = 7000.0; // Минимальная площадь контура
const double MIN_ASPECT_RATIO = 1.5;     // Минимальное соотношение сторон
const double MAX_ASPECT_RATIO = 4.5;     // Максимальное соотношение сторон

std::vector<cv::Rect> FindBoundingBoxesOfObject(const std::vector<std::vector<cv::Point>> contours);
std::vector<std::pair<cv::Rect, cv::Point>> CalculateBoxesCentersReturnPair(const std::vector<cv::Rect> boundingBoxes);
std::vector<std::vector<cv::Point>> FindBoundingParallelograms(const std::vector<std::vector<cv::Point>>& contours);
cv::Point CalculateBoxCenter(const cv::Rect boundingBox);
cv::Point FindNotFullfilAreas(const cv::Mat& depthMap, const cv::Rect& boundingBox);
bool EnsureAngleSignificant(double angle);
double ComputeAngleDegress(int dx, int dy);