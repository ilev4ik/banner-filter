#pragma once
#ifndef RAMBLER_BANNERS_SIMPLE_THREAD_POOL_H
#define RAMBLER_BANNERS_SIMPLE_THREAD_POOL_H

#include <vector>
#include <thread>
#include <queue>
#include <future>
#include <mutex>
#include <functional>
#include <condition_variable>

class simple_thread_pool
{
public:
    explicit simple_thread_pool(std::size_t);

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type>;

    void wait_all();
    ~simple_thread_pool();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
    bool wait_all_flag = false;
};

inline simple_thread_pool::simple_thread_pool(std::size_t thread_count)
{
    for (std::size_t i = 0; i < thread_count; ++i)
    {
        workers.emplace_back(
                [this]
                {
                    while (true)
                    {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(queue_mutex);
                            condition.wait(lock, [this]{ return stop || !tasks.empty();});
                            if (stop && tasks.empty()) return;
                            task = std::move(tasks.front());
                            tasks.pop();
                        }

                        task();
                    }
                });
    }
}

inline void simple_thread_pool::wait_all()
{
    wait_all_flag = true;
}

template <class F, class... Args>
auto simple_thread_pool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop) throw std::runtime_error("thread pool is stopped already");
        tasks.emplace([task]{(*task)();});
    }
    condition.notify_one();
    return res;
}

inline simple_thread_pool::~simple_thread_pool()
{
    while (!tasks.empty() && wait_all_flag) {};

    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread& worker : workers)
        worker.join();
}

#endif //RAMBLER_BANNERS_SIMPLE_THREAD_POOL_H
