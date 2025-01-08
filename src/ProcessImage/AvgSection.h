#pragma once
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
class AvgSection {
public:
	// Конструктор
	AvgSection(const cv::Rect& boundingBox, const cv::Mat& depthMap, const cv::Mat& depthMask, double blockAvgVal);

	// Вычисление среднего значения глубины
	void ComputeAverage(const cv::Mat& depthMap, const cv::Mat& depthMask);

	// Проверка, является ли секция предпочтительной
	void CheckPreferredSection(double blockAvgVal, int offsetY);

	// Разделение bounding box на части
	std::vector<AvgSection> DivideBoundingBox(int numSections,
		const cv::Mat& depthMap, const cv::Mat& depthMask, double blockAvgVal);

	// Геттеры
	double GetAverageValue() const { return averageValue; }
	cv::Point2i GetCenter() const { return center; }
	cv::Rect GetBoundingBox() const { return boundingBox; }
	bool IsPreferred() const { return isPreferredSection; }

	// Отладка
	void PrintInfo() const {
		std::cout << "AvgSection: " << boundingBox
			<< ", Center: (" << center.x << ", " << center.y << ")"
			<< ", Average Depth: " << averageValue << std::endl;
	}

private:
	double averageValue;  // Среднее значение глубины
	cv::Point2i center;   // Центр блока
	cv::Rect boundingBox; // Границы блока
	bool isPreferredSection; // Флаг предпочтительности секции
};
