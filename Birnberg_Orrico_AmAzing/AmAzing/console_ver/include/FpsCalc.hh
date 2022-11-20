#ifndef FPSCALC_HH
#define FPSCALC_HH

#include <ctime>     // clock_t

#include <chrono>

// Note: while seeking comparison of std::clock and <chrono>, learned of the
//   difference between process time and real time, see:
//   - https://cplusplus.com/forum/beginner/146620/
//   Some relevant excerpts from the above link appear after class declarations.

class ProcessTimeFpsCalc {
private:
    // timepoints to compare
    clock_t prev_tp;
    clock_t curr_tp;
    // divisor for moving average, smaller values make average adjust more quickly
    double alpha { 20.0 };
public:
    // arbitrary start value of 100 RT FPS
    double frame_duration_mvg_avg { 0.01 };
    void initialize();
    void calculate();
};

class RealTimeFpsCalc {
private:
    // timepoints to compare
    std::chrono::time_point<std::chrono::steady_clock> prev_tp;
    std::chrono::time_point<std::chrono::steady_clock> curr_tp;
    // divisor for moving average, smaller values make average adjust more quickly
    double alpha { 20.0 };
public:
    // arbitrary start value of 100 RT FPS
    std::chrono::duration<double> frame_duration_mvg_avg { 0.01 };
    void initialize();
    void calculate();
};

// """
// std::chrono::high_resolution_clock does not measure processor time; it measures
// wall clock time.
// std::chrono::high_resolution_clock is not required to be a monotonic clock;
// std::chrono::high_resolution_clock::is_steady may not be true.
//
// std::chrono::steady_clock is a monotonic clock.
// Class std::chrono::steady_clock represents a monotonic clock. The time points of
// this clock cannot decrease as physical time moves forward. This clock is not
// related to wall clock time, and is best suitable for measuring intervals.
// - http://en.cppreference.com/w/cpp/chrono/steady_clock
//
// std::clock_t clock() measures approximate processor time used by the process.
// ```
// std::clock_t clock();
// ```
// Returns the approximate processor time used by the process since the beginning
// of an implementation-defined era related to the program's execution. To convert
// result value to seconds divide it by CLOCKS_PER_SEC. ...
//
// std::clock time may advance faster or slower than the wall clock, depending on
// the execution resources given to the program by the operating system. For
// example, if the CPU is shared by other processes, std::clock time may advance
// slower than wall clock. On the other hand, if the current process is
// multithreaded and more than one execution core is available, std::clock time may
// advance faster than wall clock.
// - http://en.cppreference.com/w/cpp/chrono/c/clock
//
// Boost chrono has a set of clocks which measure actual processor time, in
// addition to clocks similar to the ones found in std::chrono. On most platforms,
// boost::chrono::process_cpu_clock has a higher resolution than std::clock()
// http://www.boost.org/doc/libs/1_56_0/doc/html/chrono/reference.html#chrono.reference.other_clocks.process_cpu_clocks_hpp 
// """
// """
// Intervals from wall clocks give elapsed wall clock time; intervals from
// std::clock() give processor time used for execution. Both are useful measures.
//
// Knowing how long a program takes to execute is useful in both test and
// production environments. It is also helpful if such timing information is broken
// down into real (wall clock) time, CPU time spent by the user, and CPU time spent
// by the operating system servicing user requests.
// http://www.boost.org/doc/libs/1_56_0/doc/html/chrono/reference.html#chrono.reference.other_clocks.process_cpu_clocks_hpp
//
// The time utility executes and times the specified utility. After the utility
// finishes, time writes to the standard error stream, (in seconds): the total time
// elapsed, the time used to execute the utility process and the time consumed by
// system overhead.
// http://www.freebsd.org/cgi/man.cgi?query=time&apropos=0&sektion=0&manpath=FreeBSD+10.1-RELEASE&arch=default&format=html
// """


#endif  // FPSCALC_HH
