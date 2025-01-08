#pragma once
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

class ParameterParser {
public:
	// Конструктор
	ParameterParser(const std::string& filePath);

	// Парсинг параметров из файла
	void ParseParameters();

	// Получение значения параметра по имени
	template<typename T>
	T GetParameter(const std::string& key) const;

private:
	std::string filePath; // Путь к файлу с параметрами
	std::map<std::string, std::string> parameters; // Хранение параметров в виде пар ключ-значение
};