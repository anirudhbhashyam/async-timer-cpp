#include <chrono>
#include <cstdint>
#include <future>
#include <functional>
#include <print>
#include <thread>

using i32 = int32_t;

using namespace std::chrono_literals;

template <typename Rep, typename Period, std::invocable F>
std::future<void> async_timer(std::chrono::duration<Rep, Period> duration, F&& callback) {
    return std::async(
        std::launch::async,
        // Move the callback when we can with forward
        [duration, cb{ std::forward<F>(callback) }](){
            std::this_thread::sleep_for(duration);
            std::invoke(cb);
        }
    );
}

i32 main() {
    auto future{ async_timer(5s, [](){ std::println("Five seconds ended."); }) };
    std::println("We are doing work...");
    future.wait();
    return 0;
}
