#pragma once
#include <ctime>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

class Logger {
private:
	static std::ofstream logFile;   // Статический файл для записи
	static std::string_view fileName;    // Имя файла
	static std::mutex logMutex;     // Мьютекс для потокобезопасности

	// Получение текущей даты и времени в строковом формате
	static std::string getCurrentTime() {
		std::time_t now = std::time(nullptr);
		char buf[80];
		std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
		return std::string(buf);
	}

public:
	// Инициализация логгера с указанием имени файла
	static void init(const std::string_view& file) {
		fileName = file;
		logFile.open(fileName.data(), std::ios::app);
		if (!logFile.is_open()) {
			throw std::runtime_error("Не удалось открыть файл для записи: " + std::string(fileName.data()));
		}
	}

	// Закрытие файла (вызывать вручную, если нужно явно завершить работу логгера)
	static void close() {
		if (logFile.is_open()) {
			logFile.close();
		}
	}

	// Метод записи ошибки в лог
	static void LogError(const std::string& errorMessage) {
		std::lock_guard<std::mutex> guard(logMutex); // Гарантирует потокобезопасность
		if (logFile.is_open()) {
			logFile << "[" << getCurrentTime() << "] ERROR: " << errorMessage << std::endl;
		}
		else {
			throw std::runtime_error("Файл лога закрыт");
		}
	}
	static void LogParams(const std::string& param)
	{
		std::lock_guard<std::mutex> guard(logMutex); // Гарантирует потокобезопасность
		if (logFile.is_open()) {
			logFile << "[" << getCurrentTime() << "] PARAMETR: " << param << std::endl;
		}
		else {
			throw std::runtime_error("Файл лога закрыт");
		}
	};
};
