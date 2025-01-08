#include "AnalyzeDepth.h"


// Вспомогательная функция: Преобразование в градации серого
static void ensureGrayscale(cv::Mat& image) {
	if (image.channels() > 1)
	{
		cv::cvtColor(image, image, cv::COLOR_BGR2GRAY);
	}
}
//TODO:: Use StereSGBM with real one camera
static cv::Mat StereoDepthMap(cv::Ptr<cv::StereoSGBM> bm, cv::Mat left, cv::Mat right)
//static cv::Mat StereoDepthMap(cv::Ptr<cv::StereoBM> bm, cv::Mat left, cv::Mat right)
{
	cv::Mat disparity;
	bm->compute(left, right, disparity);
	cv::Mat disparityNormalized;
	cv::normalize(disparity, disparityNormalized, 0, 255, cv::NORM_MINMAX, CV_8U);
	return disparityNormalized;
}

// Основная функция анализа глубины
cv::Mat AnalyzeDepth(cv::Mat& leftImage, cv::Mat& rightImage,
	int numDisparities, int blockSize)
{
	// Проверяем входные изображения
	if (leftImage.empty() || rightImage.empty())
	{
		Logger::LogError("Ошибка: одно из изображений стереопары пустое.");
		//std::cerr << "Ошибка: одно из изображений стереопары пустое." << std::endl;
		return cv::Mat();
	}
	// Убеждаемся, что изображения в градациях серого
	ensureGrayscale(leftImage);
	ensureGrayscale(rightImage);
	// Создаем объект для стереосопоставления
	cv::Ptr<cv::StereoSGBM> stereo = cv::StereoSGBM::create(1, numDisparities, blockSize);
	//cv::Ptr<cv::StereoBM> stereo = cv::StereoBM::create(numDisparities, blockSize);
	return StereoDepthMap(stereo, leftImage, rightImage);;
}

// Перегруженная функция: работа с путями к файлам
cv::Mat AnalyzeDepth(const std::string& leftPath, const std::string& rightPath,
	int numDisparities, int blockSize)
{
	// Загружаем изображения
	cv::Mat leftImage = cv::imread(leftPath, cv::IMREAD_COLOR);
	cv::Mat rightImage = cv::imread(rightPath, cv::IMREAD_COLOR);
	// Используем основную функцию для анализа
	return AnalyzeDepth(leftImage, rightImage, numDisparities, blockSize);
}
