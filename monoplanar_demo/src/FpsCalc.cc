#include "FpsCalc.hh"
#include "safeLibcCall.hh"  // Libc*

#include <ctime>            // clock clock_t CLOCKS_PER_SEC

#include <chrono>


void ProcessTimeFpsCalc::initialize() {
    prev_tp = safeLibcCall(
        std::clock, "std::clock",
        LibcRetTest<clock_t>{ [](const clock_t ret){ return ret == -1; } });
}

void ProcessTimeFpsCalc::calculate() {
    curr_tp = safeLibcCall(
        std::clock, "std::clock",
        LibcRetTest<clock_t>{ [](const clock_t ret){ return ret == -1; } });
    double frame_duration { double(curr_tp - prev_tp) / CLOCKS_PER_SEC };
    prev_tp = curr_tp;
    frame_duration_mvg_avg =
        ((frame_duration_mvg_avg * (alpha - 1)) + frame_duration) / alpha;
}

void RealTimeFpsCalc::initialize() {
    prev_tp = std::chrono::steady_clock::now();
}

void RealTimeFpsCalc::calculate() {
    curr_tp = std::chrono::steady_clock::now();
    std::chrono::duration<double> frame_duration { curr_tp - prev_tp };
    prev_tp = curr_tp;
    frame_duration_mvg_avg =
        ((frame_duration_mvg_avg * (alpha - 1)) + frame_duration) / alpha;
}
