#include "AvgSection.h"

AvgSection::AvgSection(const cv::Rect& boundingBox, const cv::Mat& depthMap, const cv::Mat& depthMask, double blockAvgVal)
	: boundingBox(boundingBox)
{
	ComputeAverage(depthMap, depthMask);
	center = cv::Point2i(boundingBox.x + boundingBox.width / 2, boundingBox.y + boundingBox.height / 2);
	CheckPreferredSection(blockAvgVal, 0);
}

void AvgSection::ComputeAverage(const cv::Mat& depthMap, const cv::Mat& depthMask)
{
	averageValue = cv::mean(depthMap(boundingBox), depthMask(boundingBox))[0];
}

void AvgSection::CheckPreferredSection(double blockAvgVal, int offsetY)
{
	if (averageValue * 1.2 > blockAvgVal) {
		isPreferredSection = (averageValue <= blockAvgVal) || (averageValue < (blockAvgVal * 0.9));
	}
	else {
		isPreferredSection = false;
	}
}

std::vector<AvgSection> AvgSection::DivideBoundingBox(int numSections,
	const cv::Mat& depthMap, const cv::Mat& depthMask, double blockAvgVal)
{
	std::vector<AvgSection> sections;
	if (depthMap.empty() || depthMask.empty()) {
		return sections;
	}

	// Проверяем, что размеры делятся без остатка
	if (boundingBox.width % numSections != 0) {
		//std::cerr << "Bounding box dimensions must be divisible by the number of sections";
	}

	// Вычисление размеров блоков
	int sectionWidth = boundingBox.width / numSections;
	int sectionHeight = boundingBox.height;

	// Разбиение на блоки
	for (int i = 0; i < numSections; ++i) {
		cv::Rect sectionRect = cv::Rect(
			boundingBox.x + i * sectionWidth,
			boundingBox.y,
			sectionWidth,
			sectionHeight
		);

		// Создание объекта AvgSection
		sections.emplace_back(sectionRect, depthMap, depthMask, blockAvgVal);
	}

	return sections;
}