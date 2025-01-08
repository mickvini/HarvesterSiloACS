#pragma once
#include "Utils/Logger/Logger.h"
#include <fstream>
#include <opencv2/core/hal/interface.h>
#include <vector>

void SaveAngleToFile(double angle);

void SaveCompressedFrameToFile(const std::vector<uchar>& compressedFrame, const std::string& filePath);
