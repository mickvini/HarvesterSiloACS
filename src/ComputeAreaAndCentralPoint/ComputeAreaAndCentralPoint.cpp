#include "ComputeAreaAndCentralPoint.h"

std::vector<cv::Rect> FindBoundingBoxesOfObject(const std::vector<std::vector<cv::Point>> contours)
{
	std::vector<cv::Rect> boundingBoxes;
	for (const auto& contour : contours) {
		double contourArea = cv::contourArea(contour);
		if (contourArea < MIN_CONTOUR_AREA) continue;

		cv::Rect boundingBox = cv::boundingRect(contour);
		double aspectRatio = static_cast<double>(boundingBox.width) / boundingBox.height;

		//if (aspectRatio < MIN_ASPECT_RATIO || aspectRatio > MAX_ASPECT_RATIO) continue;
		boundingBoxes.push_back(boundingBox);
	}
	return boundingBoxes;
}

std::vector<cv::Point> FindBoundingParallelogram(const std::vector<cv::Point>& contour)
{
	// Находим выпуклую оболочку контура
	std::vector<cv::Point> hull;
	cv::convexHull(contour, hull);

	// Находим минимальную ограничивающую рамку (параллелограмм)
	cv::RotatedRect rotatedRect = cv::minAreaRect(hull);

	// Получаем вершины параллелограмма
	std::vector<cv::Point2f> vertices(4);
	rotatedRect.points(vertices.data());

	// Преобразуем точки в целочисленные координаты
	std::vector<cv::Point> intVertices;
	for (const auto& vertex : vertices) {
		intVertices.push_back(cv::Point(static_cast<int>(vertex.x), static_cast<int>(vertex.y)));
	}

	return intVertices;
}

std::vector<std::vector<cv::Point>> FindBoundingParallelograms(const std::vector<std::vector<cv::Point>>& contours)
{
	std::vector<std::vector<cv::Point>> parallelograms;
	for (const auto& contour : contours) {
		double contourArea = cv::contourArea(contour);
		if (contourArea < MIN_CONTOUR_AREA) continue;

		std::vector<cv::Point> parallelogram = FindBoundingParallelogram(contour);
		parallelograms.push_back(parallelogram);
	}
	return parallelograms;
}

std::vector<std::pair<cv::Rect, cv::Point>> CalculateBoxesCentersReturnPair(const std::vector<cv::Rect> boundingBoxes)
{
	std::vector<std::pair<cv::Rect, cv::Point>> boxAndPointPairs;
	for (const cv::Rect box : boundingBoxes)
	{
		cv::Point boxCenter = CalculateBoxCenter(box);
		boxAndPointPairs.push_back(std::pair(box, boxCenter));
	}
	return boxAndPointPairs;
}

cv::Point FindNotFullfilAreas(const cv::Mat& depthMap, const cv::Rect& boundingBox)
{
	// UPD: Нужно добавить учёт маски т.к. ббокс может захватывать чуть больше чем кузов самосвала и скажем брать чуть поля
	// тогда точка сместиться на поле
	//Из BBOX берем коорды левого верхнего угла, ширину и высоту
	//Из depthMap берем кусок кадра который подподает под данные коорды и параметры
	//В этом куске для каждого столбца считаем среднюю глубину по строкам
	//Далее вычитаем или строим вектор изменения средней глубины по этому вектору будет ясно где незаполненное пространоство
	//и тут есть вариант «скользить» по этому вектору или просто взять точку к которой должен стремиться силосопровод

	cv::Rect validBox = boundingBox & cv::Rect(0, 0, depthMap.cols, depthMap.rows);

	// Extract the region of interest (ROI)
	cv::Mat roi = depthMap(validBox);

	// Vector to store column-wise average depths
	std::vector<double> columnAverages(roi.cols, 0.0);

	// Compute average depth for each column
	// TODO:: Use cv::reduce
	for (int col = 0; col < roi.cols; ++col)
	{
		cv::Mat column = roi.col(col);
		cv::Scalar avgDepth = cv::mean(column);

		columnAverages[col] = avgDepth[0]; // Get the average depth value
	}
	// Find areas with unfulfilled depths by analyzing depth changes
	int minPixelIdx = -1;

	double minPixelValue = std::numeric_limits<double>::max();

	for (size_t i = 1; i < columnAverages.size(); ++i)
	{
		if (columnAverages[i] < 120)
			continue;

		if (columnAverages[i] < minPixelValue)
		{
			minPixelValue = columnAverages[i];
			minPixelIdx = i;
		}
	}
	// Return the point corresponding to the area of interest
	if (minPixelIdx >= 0)
	{
		return cv::Point(validBox.x + minPixelIdx, validBox.y);
	}
	// Return an invalid point if no area is found
	return cv::Point(-1, -1);
}

cv::Point CalculateBoxCenter(const cv::Rect boundingBox)
{
	cv::Point currentCenter(boundingBox.x + boundingBox.width / 2, boundingBox.y + boundingBox.height / 2);
	return currentCenter;
}
bool EnsureAngleSignificant(double angle)
{
	return std::abs(angle) > 10;
}
double ComputeAngleDegress(int dx, int dy)
{
	return std::atan(static_cast<double>(dx) / dy) * 180 / CV_PI;
}