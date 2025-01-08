#include "HarvesterSiloACS.h"

void LoadParameters(const std::string& configFilePath) {
	ParameterParser parser(configFilePath);

	//LOG_FILE_PATH = parser.GetParameter<std::string_view>("LOG_FILE_PATH");
	//LEFT_IMAGE_PATH = parser.GetParameter<std::string_view>("LEFT_IMAGE_PATH");
	//RIGHT_IMAGE_PATH = parser.GetParameter<std::string_view>("RIGHT_IMAGE_PATH");
	//MIN_DEPTH = parser.GetParameter<int>("MIN_DEPTH");
	//MAX_DEPTH = parser.GetParameter<int>("MAX_DEPTH");
	//MIN_CONTOUR_AREA = parser.GetParameter<int>("MIN_CONTOUR_AREA");
	//CAPTURE_INTERVAL = parser.GetParameter<int>("CAPTURE_INTERVAL");
	//RESIZE_DELIMETER = parser.GetParameter<int>("RESIZE_DELIMETER");
	////NUMBER_OF_SECTIONS = parser.GetParameter<int>("NUMBER_OF_SECTIONS");
	//COMPRESSION_QUALITY = parser.GetParameter<int>("COMPRESSION_QUALITY");
}

int main() {
	std::setlocale(LC_ALL, "Russian");

	try {
		// Загрузка параметров из файла
		const std::string_view configFilePath = "config.txt";
		LoadParameters(configFilePath.data());

		Logger::init(LOG_FILE_PATH);

		cv::Mat leftImage = cv::imread(LEFT_IMAGE_PATH.data());
		cv::Mat rightImage = cv::imread(RIGHT_IMAGE_PATH.data());

		std::atomic<bool> stopFlag = false;
		// Поток для захвата кадров

		std::thread captureThread(CaptureFrames, CAPTURE_INTERVAL, VIDEO_PATH.data(), std::ref(leftImage), std::ref(rightImage), std::ref(stopFlag), true);
		while (!stopFlag) {
			/*leftImage = cv::imread(LEFT_IMAGE_PATH.data());
			rightImage = cv::imread(RIGHT_IMAGE_PATH.data());*/
			if (!leftImage.empty() && !rightImage.empty()) {
				Timer timer;
				timer.start();
				/*cv::resize(leftImage, leftImage, cv::Size(leftImage.cols / 3, leftImage.rows / 3));
				cv::resize(rightImage, rightImage, cv::Size(rightImage.cols / 3, rightImage.rows / 3));*/
				cv::Mat depthMap = ProcessDepth(leftImage, rightImage);
				ProcessContours(depthMap, leftImage);
				/*cv::imshow("Left Lens", leftImage);
				cv::imshow("Right Lens", rightImage);*/
				std::cout << "Method(Analyze Depth) execution time: " << timer.elapsed() << " seconds" << std::endl;
			}

			if (cv::waitKey(100) >= 0) break;
		}

		stopFlag = true;
		captureThread.join();
		Logger::close();

	}
	catch (const std::exception& e) {
		Logger::LogError(e.what());
	}

	return 0;
}
