#include "IOFile.h"
// 
void SaveAngleToFile(double angle)
{
	try
	{
		std::ofstream angleFile("angles.bin", std::ios::binary | std::ios::out);
		if (angleFile.is_open())
		{
			angleFile.write(reinterpret_cast<const char*>(&angle), sizeof(double));
			angleFile.close();
		}
	}
	catch (const std::exception& e)
	{
		Logger::LogError(e.what());
	}
}


// Функция для записи сжатого кадра в файл
void SaveCompressedFrameToFile(const std::vector<uchar>& compressedFrame, const std::string& filePath) {
	std::ofstream outFile(filePath, std::ios::binary);
	if (!outFile.is_open()) {
		throw std::runtime_error("Не удалось открыть файл для записи: " + filePath);
	}
	outFile.write(reinterpret_cast<const char*>(compressedFrame.data()), compressedFrame.size());
	outFile.close();
}