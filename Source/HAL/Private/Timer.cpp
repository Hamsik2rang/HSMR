#include "HAL/Timer.h"

#include <chrono>

HS_NS_BEGIN

std::stack<double> Timer::_laps;
double Timer::_initTime = 0.0;

bool Timer::Initialize()
{
	auto current = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> microseconds = current.time_since_epoch();
	_initTime = microseconds.count();

	return true;
}

void Timer::Start()
{
	double delta = getTimeSinceInit();
	_laps.push(delta);
}

double Timer::Stop()
{
	if (_laps.empty())
	{
		return 0.0;
	}

	double delta = getTimeSinceInit();
	double start = _laps.top();

	_laps.pop();
	return (delta - start);
}

void Timer::Reset()
{
	while(_laps.empty() == false)
	{
		_laps.pop();
	}
}

double Timer::GetElapsedSeconds()
{
	return GetElapsedMilliseconds() * 0.001;
}

double Timer::GetElapsedMilliseconds()
{
	double current = getTimeSinceInit();
	current = current - _laps.top();
	return current;

}

double Timer::GetElapsedMicroseconds()
{
	return GetElapsedMilliseconds() * 1000.0;
}

double Timer::getTimeSinceInit()
{
	auto current = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> microseconds = current.time_since_epoch();

	return microseconds.count() - _initTime;
}

HS_NS_END