#include "ProcessImage.h"


std::mutex ImageMutex;

// Function to capture and process video frames (resize and divide).
void CaptureFrames(double CaptureInterval, const std::string& VideoSource,
	cv::Mat& LeftFrame, cv::Mat& RightFrame,
	std::atomic<bool>& StopFlag, bool IsMilliseconds)
{
	cv::VideoCapture VideoCapture(VideoSource);
	if (!VideoCapture.isOpened()) {
		Logger::LogError("Error: Unable to open video stream: " + VideoSource);
		StopFlag = true;
		return;
	}

	auto LastCaptureTime = std::chrono::steady_clock::now();

	while (!StopFlag) {
		cv::Mat Frame;
		VideoCapture >> Frame;

		if (Frame.empty()) {
			Logger::LogError("Error: End of video stream or frame read error.");
			StopFlag = true;
			break;
		}

		auto CurrentTime = std::chrono::steady_clock::now();
		auto Elapsed = IsMilliseconds
			? std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - LastCaptureTime).count()
			: std::chrono::duration_cast<std::chrono::seconds>(CurrentTime - LastCaptureTime).count();

		if (Elapsed >= CaptureInterval) {
			cv::resize(Frame, Frame, cv::Size(Frame.cols / 4, Frame.rows / 4));
			int HalfWidth = Frame.cols / 2;

			{
				std::lock_guard<std::mutex> Lock(ImageMutex);
				LeftFrame = Frame(cv::Rect(0, 0, HalfWidth, Frame.rows)).clone();
				RightFrame = Frame(cv::Rect(HalfWidth, 0, HalfWidth, Frame.rows)).clone();
			}

			LastCaptureTime = CurrentTime;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long>(CaptureInterval)));
	}

	VideoCapture.release();
}

std::vector<uchar> CompressFrame(const cv::Mat& Frame) {
	std::vector<uchar> Buffer;
	std::vector<int> CompressionParams = { cv::IMWRITE_JPEG_QUALITY, 90 };
	if (!cv::imencode(".jpg", Frame, Buffer, CompressionParams)) {
		throw std::runtime_error("Failed to compress frame");
	}
	return Buffer;
}

std::deque<cv::Mat> depthMapCache;
std::mutex cacheMutex;

cv::Mat ProcessDepth(const cv::Mat& LeftImage, const cv::Mat& RightImage) {
	if (LeftImage.empty() || RightImage.empty()) {
		return cv::Mat();
	}

	cv::Mat LeftImageClone, RightImageClone;
	{
		std::lock_guard<std::mutex> Lock(ImageMutex);
		LeftImageClone = LeftImage.clone();
		RightImageClone = RightImage.clone();
	}

	cv::Mat depthMap = AnalyzeDepth(LeftImageClone, RightImageClone, 32, 15);
	// Добавляем новую карту глубины в буфер
	{
		std::lock_guard<std::mutex> lock(cacheMutex);
		depthMapCache.push_back(depthMap);
		if (depthMapCache.size() > DEPTH_MAP_CACHE_SIZE) {
			depthMapCache.pop_front();
		}
	}

	return depthMap;
}
cv::Mat GetAverageDepthMap() {
	std::lock_guard<std::mutex> lock(cacheMutex);
	if (depthMapCache.empty()) {
		return cv::Mat();
	}

	// Создаем матрицу для усредненной карты глубины
	cv::Mat averageDepthMap = cv::Mat::zeros(depthMapCache.front().size(), depthMapCache.front().type());

	// Суммируем все карты глубины
	for (const auto& depthMap : depthMapCache) {
		cv::add(averageDepthMap, depthMap, averageDepthMap);
	}

	// Усредняем результат
	averageDepthMap /= static_cast<double>(depthMapCache.size());

	return averageDepthMap;
}
cv::Mat GenerateDepthMask(const cv::Mat& DepthMap, int MinDepth, int MaxDepth) {
	if (DepthMap.empty()) {
		Logger::LogError("Error: DepthMap is empty, cannot create mask.");
		return cv::Mat();
	}

	cv::Mat DepthMask;
	cv::inRange(DepthMap, MinDepth, MaxDepth, DepthMask);
	return DepthMask;
}

std::vector<cv::Point2f> ApproximateRectangle(const std::vector<cv::Point2f>& Points) {
	if (Points.size() < 4) {
		return {};
	}

	cv::RotatedRect RotatedRect = cv::minAreaRect(Points);
	cv::Point2f RectPoints[4];
	RotatedRect.points(RectPoints);

	const double Tolerance = 10.0;
	for (int i = 0; i < 4; ++i) {
		cv::Point2f Vec1 = RectPoints[i] - RectPoints[(i + 1) % 4];
		cv::Point2f Vec2 = RectPoints[(i + 1) % 4] - RectPoints[(i + 2) % 4];
		double Angle = std::fabs(std::acos((Vec1.x * Vec2.x + Vec1.y * Vec2.y) /
			(cv::norm(Vec1) * cv::norm(Vec2))) * 180.0 / CV_PI);
		if (std::fabs(Angle - 90.0) > Tolerance) {
			return {};
		}
	}

	return std::vector<cv::Point2f>(RectPoints, RectPoints + 4);
}

void DetectRectangleUsingCorners(const cv::Mat& DepthMask, const cv::Mat& LeftImage) {
	// Ищем углы только на участках, где маска активна
	std::vector<cv::Point2f> Corners;
	cv::goodFeaturesToTrack(DepthMask, Corners, 100, 0.01, 10);

	// Отрисовка углов
	cv::Mat DebugImage = LeftImage.clone();
	for (const auto& Corner : Corners) {
		cv::circle(DebugImage, Corner, 2, cv::Scalar(0, 0, 255), -1); // Красные точки для углов
	}

	// Построение графа и отрисовка связей
	const double DistanceThreshold = 20.0; // Порог расстояния для соединения точек
	for (size_t i = 0; i < Corners.size(); ++i) {
		for (size_t j = i + 1; j < Corners.size(); ++j) {
			double distance = cv::norm(Corners[i] - Corners[j]); // Вычисляем расстояние между точками
			if (distance < DistanceThreshold) {
				// Если расстояние меньше порога, рисуем линию между точками
				cv::line(DebugImage, Corners[i], Corners[j], cv::Scalar(0, 255, 0), 2); // Зеленая линия для связи
			}
		}
	}

	// Отображение результата
	cv::imshow("Corners and Connections", DebugImage);
}

void DrawContoursAndCenters(cv::Mat& resultImage, const std::vector<std::vector<cv::Point>>& contours) {
	for (size_t i = 0; i < contours.size(); ++i) {
		double contourArea = cv::contourArea(contours[i]);
		if (contourArea < MIN_CONTOUR_AREA) continue;

		cv::Moments m = cv::moments(contours[i]);
		if (m.m00 != 0) {
			cv::Point center(static_cast<int>(m.m10 / m.m00), static_cast<int>(m.m01 / m.m00));
			cv::drawContours(resultImage, contours, static_cast<int>(i), cv::Scalar(0, 255, 0), 2);
			cv::circle(resultImage, center, 5, cv::Scalar(255, 0, 0), -1);
		}
	}
}

void ProcessSections(cv::Mat& resultImage, const std::vector<AvgSection>& sections, const cv::Point& center) {
	double sumPosX = 0;
	size_t count = 0;

	for (const auto& section : sections) {
		if (section.IsPreferred()) {
			cv::Point2d centerS = section.GetCenter();
			cv::circle(resultImage, centerS, 1, cv::Scalar(255, 255, 255), -1);
			sumPosX += centerS.x;
			count++;
		}
	}

	if (count > 0) {
		cv::Point prefferedPoint = cv::Point(sumPosX / count, center.y);
		cv::circle(resultImage, prefferedPoint, 10, cv::Scalar(255, 255, 255), -1);

		int dx = prefferedPoint.x - resultImage.cols / 2;
		int dy = resultImage.rows / 2;

		cv::putText(resultImage, "System follow trailer", cv::Point(10, 30), cv::FONT_HERSHEY_COMPLEX_SMALL,
			2, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);

		if (dx != 0) {
			double angleDegrees = ComputeAngleDegress(dx, dy);
			if (EnsureAngleSignificant(angleDegrees)) {
				std::cout << "Angle deviation: " << angleDegrees << " degrees" << std::endl;
				SaveAngleToFile(angleDegrees);
			}
		}
	}

}
void ProcessBoundingBox(cv::Mat& resultImage, const cv::Mat& depthMap, const cv::Mat& adjustedDepthMask,
	const cv::Rect& box, const cv::Point& center) {
	cv::Point point = FindNotFullfilAreas(depthMap, box);
	AvgSection bigBlock = AvgSection(box, depthMap, adjustedDepthMask, 0.0);
	double avgVal = bigBlock.GetAverageValue();

	cv::circle(resultImage, bigBlock.GetCenter(), 1, cv::Scalar(255, 255, 0), 2);
	cv::putText(resultImage, std::to_string(avgVal), cv::Point2d(box.width / 2, box.y),
		cv::FONT_HERSHEY_COMPLEX_SMALL, 1, cv::Scalar(0, 255, 0));

	cv::rectangle(resultImage, box, cv::Scalar(0, 255, 0), 2);
	cv::circle(resultImage, point, 5, cv::Scalar(0, 0, 255), -1);

	std::vector<AvgSection> sections = bigBlock.DivideBoundingBox(bigBlock.GetBoundingBox().width, depthMap, adjustedDepthMask, avgVal);
	ProcessSections(resultImage, sections, center);
}

void ProcessBoundingBoxes(cv::Mat& resultImage, const cv::Mat& depthMap, const cv::Mat& adjustedDepthMask,
	const std::vector<std::pair<cv::Rect, cv::Point>>& boxAndPointPairs) {
	if (boxAndPointPairs.empty()) {
		cv::putText(resultImage, "Trailer not found", cv::Point(10, 30), cv::FONT_HERSHEY_COMPLEX_SMALL,
			2, cv::Scalar(0, 0, 255), 2, cv::LINE_AA);
		return;
	}

	for (const auto& pair : boxAndPointPairs) {
		const cv::Rect& box = pair.first;
		const cv::Point& center = pair.second;

		ProcessBoundingBox(resultImage, depthMap, adjustedDepthMask, box, center);
	}
}


void ProcessContoursWithBoundingBoxes(const cv::Mat& generalDepthMask, const cv::Mat& depthMap,
	const cv::Mat& adjustedDepthMask, cv::Mat& leftImage) {
	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(generalDepthMask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	std::vector<cv::Rect> boundingBoxes = FindBoundingBoxesOfObject(contours);
	std::vector<std::pair<cv::Rect, cv::Point>> boxAndPointPairs = CalculateBoxesCentersReturnPair(boundingBoxes);
	std::vector<std::vector<cv::Point>> parallel = FindBoundingParallelograms(contours);

	cv::Mat resultImage;
	{
		std::lock_guard<std::mutex> Lock(ImageMutex);
		resultImage = leftImage.clone();

		// Отрисовка параллелограммов
		for (const auto& parallelogram : parallel) {
			if (parallelogram.size() == 4) { // Убедимся, что это действительно параллелограмм (4 точки)
				// Рисуем линии между вершинами параллелограмма
				for (size_t i = 0; i < 4; ++i) {
					cv::line(resultImage, parallelogram[i], parallelogram[(i + 1) % 4], cv::Scalar(0, 255, 255), 2); // Желтые линии
				}
			}
		}

		// Отрисовка контуров и центров
		DrawContoursAndCenters(resultImage, contours);

		// Обработка и отрисовка bounding boxes
		ProcessBoundingBoxes(resultImage, depthMap, adjustedDepthMask, boxAndPointPairs);
	}

	// Отображение результата
	cv::imshow("Result image", resultImage);
	SaveCompressedFrameToFile(CompressFrame(resultImage), "compr.jpg");
}

void ProcessContours(cv::Mat& depthMap, cv::Mat& leftImage) {
	if (depthMap.empty() || leftImage.empty()) {
		Logger::LogError("Invalid input: depthMap or leftImage is empty.");
		return;
	}

	// Получаем усредненную карту глубины
	cv::Mat averagedDepthMap = GetAverageDepthMap();

	if (averagedDepthMap.empty()) {
		Logger::LogError("Averaged depth map is empty.");
		return;
	}

	cv::imshow("Depth Map", averagedDepthMap);

	cv::Mat generalDepthMask = GenerateDepthMask(averagedDepthMap, MIN_DEPTH, MAX_DEPTH);
	cv::Mat frontSideDepthMask = GenerateDepthMask(averagedDepthMap, 75, 110);
	cv::Mat adjustedDepthMask;

	
	
	
	cv::Rect rectangleToExclude(frontSideDepthMask.cols / 2 - 40, 0, 120, frontSideDepthMask.rows / 2);
	cv::Mat rectMask = cv::Mat::zeros(frontSideDepthMask.size(), frontSideDepthMask.type());
	cv::rectangle(rectMask, rectangleToExclude, cv::Scalar(255), cv::FILLED);
	
	cv::subtract(generalDepthMask, frontSideDepthMask, adjustedDepthMask);
	cv::subtract(generalDepthMask, rectMask, generalDepthMask);
	cv::imshow("Gen", generalDepthMask);
	cv::imshow("Front", frontSideDepthMask);
	cv::imshow("adj", adjustedDepthMask);
	//DetectRectangleUsingCorners(adjustedDepthMask, leftImage);
	
	
	if (!adjustedDepthMask.empty()) {
		ProcessContoursWithBoundingBoxes(generalDepthMask, averagedDepthMap, adjustedDepthMask, leftImage);
	}
}