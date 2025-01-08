#pragma once
#include <atomic>
#include <chrono>
#include <clocale>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <vector>

#include "AnalyzeDepth/AnalyzeDepth.h"
#include "AvgSection.h"
#include "ComputeAreaAndCentralPoint/ComputeAreaAndCentralPoint.h"
#include "Utils/IOFile/IOFile.h"
#include "Utils/Logger/Logger.h"

#include <deque>

const int DEPTH_MAP_CACHE_SIZE = 3; // Размер буфера для кэширования карт глубины

extern std::deque<cv::Mat> depthMapCache; // Буфер для хранения карт глубины
extern std::mutex cacheMutex; // Мьютекс для синхронизации доступа к буферу

const int MIN_DEPTH = 55;
const int MAX_DEPTH = 110;
//const int MIN_DEPTH = 85;
//const int MAX_DEPTH = 180;

void CaptureFrames(double captureInterval,
	const std::string& videoSource,
	cv::Mat& leftFrame,
	cv::Mat& rightFrame,
	std::atomic<bool>& stopFlag,
	bool isMilliseconds);
cv::Mat ProcessDepth(const cv::Mat& leftImage, const cv::Mat& rightImage);
void ProcessContours(cv::Mat& depthMap, cv::Mat& leftImage);