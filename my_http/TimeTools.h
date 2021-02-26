#ifndef TIMETOOLS__H__
#define TIMETOOLS__H__

#include<chrono>

using Clock = std::chrono::high_resolution_clock;
using MS = std::chrono::milliseconds;
using MCS = std::chrono::microseconds;
//using DurationCast = std::chrono::duration_cast;
using Timestamp = Clock::time_point;
using Duration = Clock::duration;







#endif
