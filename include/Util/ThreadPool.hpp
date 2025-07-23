#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <stop_token>

class ThreadPool {
    public:
        ThreadPool(size_t n = std::max(1u, std::jthread::hardware_concurrency() - 1)) {
            for (size_t i = 0; i < n; ++i) {
                workers.emplace_back([this](std::stop_token st) {
                    while (!st.stop_requested()) {
                        std::function<void()> task;
                        {
                            std::unique_lock lk{mtx};
                            cv.wait(lk, [&] { return st.stop_requested() || !tasks.empty(); });
                            if (st.stop_requested() && tasks.empty())
                                return;
                            task = std::move(tasks.front());
                            tasks.pop();
                        }
                        task();
                    }
                });
            }
        }

        ~ThreadPool() = default;

        template <typename F, typename... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
            using R = std::invoke_result_t<F, Args...>;

            auto task = std::make_shared<std::packaged_task<R()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<R> result = task->get_future();

            {
                std::scoped_lock lk{mtx};
                tasks.emplace([task]() { (*task)(); });
            }
            cv.notify_one();

            return result;
        }

    private:
        std::vector<std::jthread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex mtx;
        std::condition_variable cv;
};
