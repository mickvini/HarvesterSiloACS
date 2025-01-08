cmake_minimum_required(VERSION 3.8)

# Включение политики для MSVC, только если используете MSVC
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
endif()

# Название проекта
project("HarvesterSiloACS")

# Путь к OpenCV
set(OpenCV_DIR "C:/Users/romanyuk_ei/Documents/SDK/opencv-4.10.0/build/")

# Найдите OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Добавление include-директории
include_directories("src")

# Источники проекта
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} "src/ProcessImage/AvgSection.h")

# Линковка OpenCV
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

# Настройка стандарта C++
if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET HarvesterSiloACS PROPERTY CXX_STANDARD 20)
endif()
