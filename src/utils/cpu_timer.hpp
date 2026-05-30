// utils/cpu_timer.hpp

#pragma once

#include <chrono>


namespace blr::utils
{


class CpuTimer
{
public:
    CpuTimer(float& outAccumulatorMs) 
        : m_startTime(std::chrono::steady_clock::now()), m_accumulator(outAccumulatorMs) 
    {}

    ~CpuTimer()
    {
        auto endTime = std::chrono::steady_clock::now();

        std::chrono::duration<float, std::milli> duration = endTime - m_startTime;
        
        m_accumulator += duration.count();
    }

private:
    std::chrono::steady_clock::time_point m_startTime;
    float& m_accumulator;
};


} /* namespace blr::utils */