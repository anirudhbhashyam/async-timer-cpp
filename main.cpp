#include "timer.hh"
#include <print>

using i32 = int32_t;
using namespace std::chrono_literals;

i32 main() {
    Timer timer{ };
    timer(
        2s,
        [](){ std::println("The callback after the timer ends."); }
    );
    std::println("We are doing work...");
    return 0;
}
