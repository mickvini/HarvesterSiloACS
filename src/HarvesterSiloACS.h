#pragma once
#include "ProcessImage/ProcessImage.h"
#include "Timer/Timer.h"
#include "Utils/Logger/Logger.h"
#include "Utils/ParseParams/ParameterParser.h"

//GENERAL COMMENT: Use Clang-format(I don't know wich one style), Use Doxygen
//GENERAL COMMENT: Using only Russian or English language in comments
//GENERAL COMMENT: Cover code with try-cath, and add logger
//TODO:: Make Unit-Test, maybe integration and regression tests
// Константы
const double CAPTURE_INTERVAL = 1000 / 25; // Интервал в миллисекундах

// Пути к файлам, по итогу тут будет VideoSource стереопары
constexpr std::string_view VIDEO_PATH = "C:/Users/romanyuk_ei/Videos/Harvester/HD2K_SN31284250_13-57-07_lr.avi";
constexpr std::string_view TEMP_IMAGE_PATH = "C:\\Users\\romanyuk_ei\\Pictures\\FromUnity\\TempImage.png";
constexpr std::string_view LEFT_IMAGE_PATH = "C:\\Users\\romanyuk_ei\\Pictures\\FromUnity\\LeftImage.png";
constexpr std::string_view RIGHT_IMAGE_PATH = "C:\\Users\\romanyuk_ei\\Pictures\\FromUnity\\RightImage.png";
constexpr std::string_view LOG_FILE_PATH = "Log.txt";

// Инициализация статических переменных
std::ofstream Logger::logFile;
std::string_view Logger::fileName;
std::mutex Logger::logMutex;

// Добавить шаблоны для разных типов самосвал, трактор + (три типа трейлеров малый средный большой)
// При это теряется динамичность под любой транспорт
// Или попробовать калибровать в процессе, условно брать слепок площади, когда начинаем загрузку...
// То бишь отдаем это на решение самого комбайнера, что не гуд?






