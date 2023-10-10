#include "telemetry.h"

using namespace std::literals; // enables literal suffixes, e.g. 24h, 1ms, 1s.

Telemetry::Timer::Timer(const char *timerMsg) : start(std::chrono::steady_clock::now()), timerMsg(timerMsg) {}
    
void Telemetry::Timer::check() const
{
    std::cout << "\e[1;90m" << timerMsg << " took " << (std::chrono::steady_clock::now() - start) / 1ms << "ms\e[0m" << std::endl;
}