#pragma once
#include <chrono>
#include <iostream>
#include <thread>

using namespace std::chrono;

//Служебный класс для тестов
//TODO: Delete
class Timer {
public:
	Timer() : start_time(high_resolution_clock::now()), frame_count(0), fps(0) {}

	void start()
	{
		start_time = high_resolution_clock::now();
	}

	double elapsed()
	{
		auto end_time = high_resolution_clock::now();
		duration<double> diff = end_time - start_time;
		return diff.count();
	}

	void updateFPS()
	{
		frame_count++;
		auto now = high_resolution_clock::now();
		duration<double> diff = now - last_fps_time;

		if (diff.count() >= 1.0)
		{
			fps = frame_count;
			frame_count = 0;
			last_fps_time = now;
		}
	}

	int getFPS() const
	{
		return fps;
	}

private:
	high_resolution_clock::time_point start_time;
	high_resolution_clock::time_point last_fps_time;
	int frame_count;
	int fps;
};