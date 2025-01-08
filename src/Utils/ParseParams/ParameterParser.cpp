#include "ParameterParser.h"

ParameterParser::ParameterParser(const std::string& filePath)
	: filePath(filePath) {
}

void ParameterParser::ParseParameters() {
	std::ifstream file(filePath);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open parameter file: " + filePath);
	}

	std::string line;
	while (std::getline(file, line)) {
		// Пропуск комментариев и пустых строк
		if (line.empty() || line[0] == '#') {
			continue;
		}

		std::istringstream lineStream(line);
		std::string key, value;
		if (std::getline(lineStream, key, '=') && std::getline(lineStream, value)) {
			parameters[key] = value;
		}
	}
	file.close();
}

template<typename T>
T ParameterParser::GetParameter(const std::string& key) const {
	auto it = parameters.find(key);
	if (it == parameters.end()) {
		throw std::runtime_error("Parameter not found: " + key);
	}

	std::istringstream valueStream(it->second);
	T value;
	valueStream >> value;
	if (valueStream.fail()) {
		throw std::runtime_error("Failed to convert parameter: " + key);
	}

	return value;
}
