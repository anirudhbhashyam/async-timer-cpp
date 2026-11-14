#pragma once

#include <chrono>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <stop_token>
#include <utility>
#include <type_traits>

template <typename F, typename... Args>
concept TimerCallback = std::invocable<std::decay_t<F>, std::decay_t<Args>...>;

struct Timer {
    public:
        Timer() = default;

        template <typename Rep, typename Period, typename F, typename... Args>
            requires TimerCallback<F, Args...>
        void operator()(std::chrono::duration<Rep, Period> duration, F&& callback, Args&&... args) {
            m_thread = std::jthread(
                [
                    duration,
                    cb{ std::forward<F>(callback) },
                    ...args{ std::forward<Args>(args) }
                ] (std::stop_token st) mutable {
                    bool stopped{ };
                    {
                        std::mutex m{ };
                        std::condition_variable cv{ };
                        std::stop_callback on_stop{
                            st,
                            [&m, &cv] () {
                                std::lock_guard g{ m };
                                cv.notify_one();
                            }
                        };
                        std::unique_lock lock{ m };
                        stopped = cv.wait_for(lock, duration, [&st](){ return st.stop_requested(); });
                    }
                    if (!stopped) std::invoke(std::move(cb), std::move(args)...);
                }
            );
        }

        void wait() {
            if (m_thread.joinable()) m_thread.join();
        }

        // Use cancel to kill the timer
        // ignoring the callback, otherwise
        // the destructor will wait.
        void cancel() noexcept {
            m_thread.request_stop();
        }

        ~Timer() {
            wait();
        }
        Timer(const Timer& timer) = delete;
        Timer& operator=(const Timer& timer) = delete;
        Timer(Timer&&) noexcept = default;
        Timer& operator=(Timer&&) noexcept = default;

    private:
        std::jthread m_thread{ };
};
