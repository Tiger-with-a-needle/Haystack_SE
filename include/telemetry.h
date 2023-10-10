#include <chrono>
#include <iostream>

namespace Telemetry
{
    class Timer
    {
        std::chrono::time_point<std::chrono::steady_clock> start;
        const char *timerMsg;

    public:
        Timer(const char*);
        void check() const;
    };
}